/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "display_skia_output_device.h"

#include "type_conversion.h"

#include "gpu/command_buffer/service/skia_utils.h"
#include "skia/ext/legacy_display_globals.h"

namespace QtWebEngineCore {

class DisplaySkiaOutputDevice::Buffer
{
public:
    Buffer(DisplaySkiaOutputDevice *parent)
        : m_parent(parent)
        , m_shape(m_parent->m_shape)
    {
        auto formatIndex = static_cast<int>(m_shape.format);
        const auto &colorType = m_parent->capabilities_.sk_color_types[formatIndex];
        DCHECK(colorType != kUnknown_SkColorType)
                << "SkColorType is invalid for format: " << formatIndex;

        m_texture = m_parent->m_contextState->gr_context()->createBackendTexture(
                m_shape.sizeInPixels.width(), m_shape.sizeInPixels.height(), colorType,
                GrMipMapped::kNo, GrRenderable::kYes);
        DCHECK(m_texture.isValid());

        if (m_texture.backend() == GrBackendApi::kVulkan) {
            GrVkImageInfo info;
            bool result = m_texture.getVkImageInfo(&info);
            DCHECK(result);
            m_estimatedSize = info.fAlloc.fSize;
        } else {
            auto info = SkImageInfo::Make(m_shape.sizeInPixels.width(), m_shape.sizeInPixels.height(),
                                          colorType, kUnpremul_SkAlphaType);
            m_estimatedSize = info.computeMinByteSize();
        }
        m_parent->memory_type_tracker_->TrackMemAlloc(m_estimatedSize);

        SkSurfaceProps surfaceProps = skia::LegacyDisplayGlobals::GetSkSurfaceProps();
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

bool DisplaySkiaOutputDevice::Reshape(const gfx::Size& sizeInPixels,
                                      float devicePixelRatio,
                                      const gfx::ColorSpace& colorSpace,
                                      gfx::BufferFormat format,
                                      gfx::OverlayTransform transform)
{
    m_shape = Shape{sizeInPixels, devicePixelRatio, colorSpace, format};
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
    return m_frontBuffer ? toQt(m_frontBuffer->shape().sizeInPixels) : QSize();
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
                      gfx::Size(m_shape.sizeInPixels.width(), m_shape.sizeInPixels.height()),
                      std::move(m_frame));
}

} // namespace QtWebEngineCore
