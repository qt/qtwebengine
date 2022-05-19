// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "display_skia_output_device.h"

#include "type_conversion.h"

#include "gpu/command_buffer/service/skia_utils.h"
#include "third_party/skia/include/core/SkSurfaceProps.h"

namespace QtWebEngineCore {

class DisplaySkiaOutputDevice::Buffer
{
public:
    Buffer(DisplaySkiaOutputDevice *parent)
        : m_parent(parent)
        , m_shape(m_parent->m_shape)
    {
        const auto &colorType = m_shape.characterization.colorType();
        DCHECK(colorType != kUnknown_SkColorType);

        m_texture = m_parent->m_contextState->gr_context()->createBackendTexture(
                m_shape.characterization.width(), m_shape.characterization.height(), colorType,
                GrMipMapped::kNo, GrRenderable::kYes);
        DCHECK(m_texture.isValid());

        if (m_texture.backend() == GrBackendApi::kVulkan) {
#if BUILDFLAG(ENABLE_VULKAN)
            GrVkImageInfo info;
            bool result = m_texture.getVkImageInfo(&info);
            DCHECK(result);
            m_estimatedSize = info.fAlloc.fSize;
#else
            NOTREACHED();
#endif
        } else {
            auto info = SkImageInfo::Make(m_shape.characterization.width(), m_shape.characterization.height(),
                                          colorType, kUnpremul_SkAlphaType);
            m_estimatedSize = info.computeMinByteSize();
        }
        m_parent->memory_type_tracker_->TrackMemAlloc(m_estimatedSize);

        SkSurfaceProps surfaceProps = SkSurfaceProps{0, kUnknown_SkPixelGeometry};
        m_surface = SkSurface::MakeFromBackendTexture(
                m_parent->m_contextState->gr_context(), m_texture,
                m_parent->capabilities_.output_surface_origin == gfx::SurfaceOrigin::kTopLeft
                ? kTopLeft_GrSurfaceOrigin
                : kBottomLeft_GrSurfaceOrigin,
                0 /* sampleCount */, colorType, m_shape.colorSpace.ToSkColorSpace(),
                &surfaceProps);
    }

    ~Buffer()
    {
        DeleteGrBackendTexture(m_parent->m_contextState.get(), &m_texture);
        m_parent->memory_type_tracker_->TrackMemFree(m_estimatedSize);
    }

    void createFence()
    {
        m_fence = CompositorResourceFence::create();
    }

    void consumeFence()
    {
        if (m_fence) {
            m_fence->wait();
            m_fence.reset();
        }
    }

    const Shape &shape() const { return m_shape; }
    const GrBackendTexture &texture() const { return m_texture; }
    SkSurface *surface() const { return m_surface.get(); }

private:
    DisplaySkiaOutputDevice *m_parent;
    Shape m_shape;
    GrBackendTexture m_texture;
    sk_sp<SkSurface> m_surface;
    uint64_t m_estimatedSize = 0;
    scoped_refptr<CompositorResourceFence> m_fence;
};

DisplaySkiaOutputDevice::DisplaySkiaOutputDevice(
        scoped_refptr<gpu::SharedContextState> contextState,
        gpu::MemoryTracker *memoryTracker,
        DidSwapBufferCompleteCallback didSwapBufferCompleteCallback)
    : SkiaOutputDevice(
            contextState->gr_context(),
            memoryTracker,
            didSwapBufferCompleteCallback)
    , Compositor(Compositor::Type::OpenGL)
    , m_contextState(contextState)
{
    capabilities_.uses_default_gl_framebuffer = false;
    capabilities_.supports_surfaceless = true;

    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBA_8888)] =
            kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBX_8888)] =
            kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRA_8888)] =
            kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRX_8888)] =
            kRGBA_8888_SkColorType;
}

DisplaySkiaOutputDevice::~DisplaySkiaOutputDevice()
{
}

void DisplaySkiaOutputDevice::SetFrameSinkId(const viz::FrameSinkId &id)
{
    bind(id);
}

bool DisplaySkiaOutputDevice::Reshape(const SkSurfaceCharacterization &characterization,
                                      const gfx::ColorSpace &colorSpace,
                                      float device_scale_factor,
                                      gfx::OverlayTransform transform)
{
    m_shape = Shape{characterization, device_scale_factor, colorSpace};
    DCHECK_EQ(transform, gfx::OVERLAY_TRANSFORM_NONE);
    return true;
}

void DisplaySkiaOutputDevice::SwapBuffers(BufferPresentedCallback feedback,
                                          viz::OutputSurfaceFrame frame)
{
    DCHECK(m_backBuffer);

    StartSwapBuffers(std::move(feedback));
    m_frame = std::move(frame);
    m_backBuffer->createFence();

    {
        QMutexLocker locker(&m_mutex);
        m_taskRunner = base::ThreadTaskRunnerHandle::Get();
        m_middleBuffer = std::move(m_backBuffer);
        m_readyToUpdate = true;
    }

    if (auto obs = observer())
        obs->readyToSwap();
}

void DisplaySkiaOutputDevice::EnsureBackbuffer()
{
}

void DisplaySkiaOutputDevice::DiscardBackbuffer()
{
}

SkSurface *DisplaySkiaOutputDevice::BeginPaint(std::vector<GrBackendSemaphore> *)
{
    if (!m_backBuffer || m_backBuffer->shape() != m_shape)
        m_backBuffer = std::make_unique<Buffer>(this);
    return m_backBuffer->surface();
}

void DisplaySkiaOutputDevice::EndPaint()
{
}

void DisplaySkiaOutputDevice::swapFrame()
{
    QMutexLocker locker(&m_mutex);
    if (m_readyToUpdate) {
        std::swap(m_middleBuffer, m_frontBuffer);
        m_taskRunner->PostTask(FROM_HERE,
                               base::BindOnce(&DisplaySkiaOutputDevice::SwapBuffersFinished,
                                              base::Unretained(this)));
        m_taskRunner.reset();
        m_readyToUpdate = false;
    }
}

void DisplaySkiaOutputDevice::waitForTexture()
{
    if (m_frontBuffer)
        m_frontBuffer->consumeFence();
}

int DisplaySkiaOutputDevice::textureId()
{
    if (!m_frontBuffer)
        return 0;

    GrGLTextureInfo info;
    if (!m_frontBuffer->texture().getGLTextureInfo(&info))
        return 0;

    return info.fID;
}

QSize DisplaySkiaOutputDevice::size()
{
    return m_frontBuffer ? toQt(m_frontBuffer->shape().characterization.dimensions()) : QSize();
}

bool DisplaySkiaOutputDevice::hasAlphaChannel()
{
    return true;
}

float DisplaySkiaOutputDevice::devicePixelRatio()
{
    return m_frontBuffer ? m_frontBuffer->shape().devicePixelRatio : 1;
}

void DisplaySkiaOutputDevice::SwapBuffersFinished()
{
    {
        QMutexLocker locker(&m_mutex);
        m_backBuffer = std::move(m_middleBuffer);
    }

    FinishSwapBuffers(gfx::SwapCompletionResult(gfx::SwapResult::SWAP_ACK),
                      gfx::Size(m_shape.characterization.width(), m_shape.characterization.height()),
                      std::move(m_frame));
}

} // namespace QtWebEngineCore
