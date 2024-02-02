// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "native_skia_output_device.h"

#include "type_conversion.h"

#include "components/viz/common/resources/shared_image_format.h"
#include "components/viz/service/display_embedder/skia_output_surface_dependency.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/command_buffer/common/shared_image_usage.h"
#include "gpu/command_buffer/service/shared_image/shared_image_factory.h"
#include "gpu/command_buffer/service/shared_image/shared_image_representation.h"
#include "gpu/command_buffer/service/skia_utils.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "third_party/skia/include/core/SkSurfaceProps.h"
#include "ui/gl/gl_fence.h"

#if defined(Q_OS_WIN)
#include <QtCore/private/qsystemerror_p.h>
#include <QtQuick/qsgtexture.h>
#include <d3d11_1.h>
#endif

#include <QQuickWindow>

namespace QtWebEngineCore {

class NativeSkiaOutputDevice::Buffer
{
public:
    Buffer(NativeSkiaOutputDevice *parent)
        : m_parent(parent)
        , m_shape(m_parent->m_shape)
    {
    }
    ~Buffer()
    {
        if (m_scopedSkiaWriteAccess)
            endWriteSkia(false);

        if (!m_mailbox.IsZero())
            m_parent->m_factory->DestroySharedImage(m_mailbox);
    }

    // The following Buffer methods are based on components/viz/service/display_embedder/output_presenter.cc:
    // Copyright 2020 The Chromium Authors
    // Use of this source code is governed by a BSD-style license that can be
    // found in the LICENSE file.
    bool initialize()
    {
        static const uint32_t kDefaultSharedImageUsage =
                gpu::SHARED_IMAGE_USAGE_SCANOUT | gpu::SHARED_IMAGE_USAGE_DISPLAY_READ
              | gpu::SHARED_IMAGE_USAGE_DISPLAY_WRITE | gpu::SHARED_IMAGE_USAGE_GLES2_FRAMEBUFFER_HINT;
        auto mailbox = gpu::Mailbox::GenerateForSharedImage();

        if (!m_parent->m_factory->CreateSharedImage(mailbox,
                                                    viz::SharedImageFormat::SinglePlane(viz::RGBA_8888),
                                                    {m_parent->m_shape.characterization.width(), m_parent->m_shape.characterization.height()},
                                                    m_parent->m_shape.colorSpace,
                                                    m_parent->capabilities_.output_surface_origin == gfx::SurfaceOrigin::kTopLeft
                                                        ? kTopLeft_GrSurfaceOrigin
                                                        : kBottomLeft_GrSurfaceOrigin,
                                                    kPremul_SkAlphaType,
                                                    m_parent->m_deps->GetSurfaceHandle(), kDefaultSharedImageUsage)) {
            LOG(ERROR) << "CreateSharedImage failed.";
            return false;
        }
        m_mailbox = mailbox;

        m_skiaRepresentation = m_parent->m_representationFactory->ProduceSkia(m_mailbox, m_parent->m_deps->GetSharedContextState());
        if (!m_skiaRepresentation) {
            LOG(ERROR) << "ProduceSkia() failed.";
            return false;
        }

        m_overlayRepresentation = m_parent->m_representationFactory->ProduceOverlay(m_mailbox);
        if (!m_overlayRepresentation) {
            LOG(ERROR) << "ProduceOverlay() failed";
            return false;
        }

        return true;
    }
    SkSurface *beginWriteSkia()
    {
        DCHECK(!m_scopedSkiaWriteAccess);
        DCHECK(!m_presentCount);
        DCHECK(m_endSemaphores.empty());

        std::vector<GrBackendSemaphore> beginSemaphores;
        SkSurfaceProps surface_props{0, kUnknown_SkPixelGeometry};

        // Buffer queue is internal to GPU proc and handles texture initialization,
        // so allow uncleared access.
        m_scopedSkiaWriteAccess = m_skiaRepresentation->BeginScopedWriteAccess(
            0 /* sampleCount */, surface_props, &beginSemaphores, &m_endSemaphores,
            gpu::SharedImageRepresentation::AllowUnclearedAccess::kYes);
        DCHECK(m_scopedSkiaWriteAccess);
        if (!beginSemaphores.empty()) {
            m_scopedSkiaWriteAccess->surface()->wait(
                beginSemaphores.size(),
                beginSemaphores.data(),
                /*deleteSemaphoresAfterWait=*/false);
        }
        return m_scopedSkiaWriteAccess->surface();
    }

    void endWriteSkia(bool force_flush)
    {
        // The Flush now takes place in finishPaintCurrentBuffer on the CPU side.
        // check if end_semaphores is not empty then flush here
        DCHECK(m_scopedSkiaWriteAccess);
        auto end_state = m_scopedSkiaWriteAccess->TakeEndState();
        if (!m_endSemaphores.empty() || end_state || force_flush) {
            GrFlushInfo flush_info = {};
                flush_info.fNumSemaphores = m_endSemaphores.size();
                flush_info.fSignalSemaphores = m_endSemaphores.data();
            m_scopedSkiaWriteAccess->surface()->flush(flush_info, end_state.get());
            auto *direct_context = m_scopedSkiaWriteAccess->surface()->recordingContext()->asDirectContext();
            DCHECK(direct_context);
            direct_context->submit();
        }
        m_scopedSkiaWriteAccess.reset();
        m_endSemaphores.clear();

        // SkiaRenderer always draws the full frame.
        m_skiaRepresentation->SetCleared();
    }

    std::vector<GrBackendSemaphore> takeEndWriteSkiaSemaphores()
    {
        return std::exchange(m_endSemaphores, {});
    }

    void beginPresent()
    {
        if (++m_presentCount != 1) {
            DCHECK(m_scopedOverlayReadAccess);
            return;
        }

        DCHECK(!m_scopedSkiaWriteAccess);
        DCHECK(!m_scopedOverlayReadAccess);

        m_scopedOverlayReadAccess = m_overlayRepresentation->BeginScopedReadAccess();
        DCHECK(m_scopedOverlayReadAccess);
        m_acquireFence = TakeGpuFence(m_scopedOverlayReadAccess->TakeAcquireFence());
    }

    void endPresent()
    {
        if (!m_presentCount)
            return;
        DCHECK(m_scopedOverlayReadAccess);
        if (--m_presentCount)
            return;

        m_scopedOverlayReadAccess.reset();
    }

    void freeTexture()
    {
        if (m_textureCleanup) {
            m_textureCleanup();
            m_textureCleanup = nullptr;
        }
    }
#if defined(Q_OS_WIN)
    absl::optional<gl::DCLayerOverlayImage> overlayImage() const
    {
        DCHECK(m_presentCount);
        return m_scopedOverlayReadAccess->GetDCLayerOverlayImage();
    }
#elif defined(Q_OS_MACOS)
    gfx::ScopedIOSurface ioSurface() const
    {
        DCHECK(m_presentCount);
        return m_scopedOverlayReadAccess->GetIOSurface();
    }
#endif

    void createFence()
    {
        // For some reason we still need to create this, but we do not need to wait on it.
        m_fence = gl::GLFence::Create();
    }

    void consumeFence()
    {
        if (m_acquireFence) {
            m_acquireFence->Wait();
            m_acquireFence.reset();
        }
    }

    const Shape &shape() const { return m_shape; }

    std::function<void()> m_textureCleanup;
private:
    NativeSkiaOutputDevice *m_parent;
    Shape m_shape;
    uint64_t m_estimatedSize = 0; // FIXME: estimate size
    static std::unique_ptr<gfx::GpuFence> TakeGpuFence(gfx::GpuFenceHandle fence)
    {
      return fence.is_null() ? nullptr
                             : std::make_unique<gfx::GpuFence>(std::move(fence));
    }
    std::unique_ptr<gfx::GpuFence> m_acquireFence;
    std::unique_ptr<gl::GLFence> m_fence;
    gpu::Mailbox m_mailbox;
    std::unique_ptr<gpu::SkiaImageRepresentation> m_skiaRepresentation;
    std::unique_ptr<gpu::SkiaImageRepresentation::ScopedWriteAccess> m_scopedSkiaWriteAccess;
    std::unique_ptr<gpu::OverlayImageRepresentation> m_overlayRepresentation;
    std::unique_ptr<gpu::OverlayImageRepresentation::ScopedReadAccess> m_scopedOverlayReadAccess;

    std::vector<GrBackendSemaphore> m_endSemaphores;
    int m_presentCount = 0;
};

NativeSkiaOutputDevice::NativeSkiaOutputDevice(
        scoped_refptr<gpu::SharedContextState> contextState,
        bool requiresAlpha,
        gpu::MemoryTracker *memoryTracker,
        viz::SkiaOutputSurfaceDependency *dependency,
        gpu::SharedImageFactory *shared_image_factory,
        gpu::SharedImageRepresentationFactory *shared_image_representation_factory,
        DidSwapBufferCompleteCallback didSwapBufferCompleteCallback)
    : SkiaOutputDevice(contextState->gr_context(), memoryTracker, didSwapBufferCompleteCallback)
    , Compositor(Compositor::Type::NativeBuffer)
    , m_requiresAlpha(requiresAlpha)
    , m_factory(shared_image_factory)
    , m_representationFactory(shared_image_representation_factory)
    , m_deps(dependency)
{
    capabilities_.uses_default_gl_framebuffer = false;
    capabilities_.supports_surfaceless = true;
    capabilities_.preserve_buffer_content = true;
    capabilities_.only_invalidates_damage_rect = false;
    capabilities_.number_of_buffers = 3;

    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBA_8888)] =
            kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBX_8888)] =
            kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRA_8888)] =
            kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRX_8888)] =
            kRGBA_8888_SkColorType;
}

NativeSkiaOutputDevice::~NativeSkiaOutputDevice()
{
}

void NativeSkiaOutputDevice::SetFrameSinkId(const viz::FrameSinkId &id)
{
    bind(id);
}

bool NativeSkiaOutputDevice::Reshape(const SkSurfaceCharacterization &characterization,
                                      const gfx::ColorSpace &colorSpace,
                                      float device_scale_factor,
                                      gfx::OverlayTransform transform)
{
    m_shape = Shape{characterization, device_scale_factor, colorSpace};
    DCHECK_EQ(transform, gfx::OVERLAY_TRANSFORM_NONE);
    return true;
}

void NativeSkiaOutputDevice::SwapBuffers(BufferPresentedCallback feedback,
                                          viz::OutputSurfaceFrame frame)
{
    DCHECK(m_backBuffer);

    StartSwapBuffers(std::move(feedback));
    m_frame = std::move(frame);
    {
        QMutexLocker locker(&m_mutex);
        m_backBuffer->createFence();
        m_taskRunner = base::SingleThreadTaskRunner::GetCurrentDefault();
        std::swap(m_middleBuffer, m_backBuffer);
        m_readyToUpdate = true;
    }

    if (auto obs = observer())
        obs->readyToSwap();
}

void NativeSkiaOutputDevice::EnsureBackbuffer()
{
}

void NativeSkiaOutputDevice::DiscardBackbuffer()
{
}


SkSurface *NativeSkiaOutputDevice::BeginPaint(std::vector<GrBackendSemaphore> *end_semaphores)
{
    {
        QMutexLocker locker(&m_mutex);
        if (!m_backBuffer || m_backBuffer->shape() != m_shape) {
            m_backBuffer = std::make_unique<Buffer>(this);
            if (!m_backBuffer->initialize())
                return nullptr;
        }
    }
    auto surface = m_backBuffer->beginWriteSkia();
    *end_semaphores = m_backBuffer->takeEndWriteSkiaSemaphores();
    return surface;
}

void NativeSkiaOutputDevice::EndPaint()
{
    m_backBuffer->endWriteSkia(true);
}

void NativeSkiaOutputDevice::swapFrame()
{
    QMutexLocker locker(&m_mutex);
    if (m_readyToUpdate) {
        std::swap(m_frontBuffer, m_middleBuffer);
        m_taskRunner->PostTask(FROM_HERE,
                               base::BindOnce(&NativeSkiaOutputDevice::SwapBuffersFinished,
                                              base::Unretained(this)));
        m_taskRunner.reset();
        m_readyToUpdate = false;
        if (m_frontBuffer) {
            m_readyWithTexture = true;
            m_frontBuffer->beginPresent();
        }
        if (m_middleBuffer)
            m_middleBuffer->freeTexture();
    }
}

void NativeSkiaOutputDevice::waitForTexture()
{
    if (m_readyWithTexture)
        m_frontBuffer->consumeFence();
}

void NativeSkiaOutputDevice::releaseTexture()
{
    if (m_readyWithTexture) {
        m_frontBuffer->endPresent();
        m_readyWithTexture = false;
    }
}

void NativeSkiaOutputDevice::releaseResources(QQuickWindow *)
{
    if (m_frontBuffer)
        m_frontBuffer->freeTexture();
}

#if defined(Q_OS_MACOS)
QSGTexture *makeMetalTexture(QQuickWindow *win, IOSurfaceRef io_surface, uint io_surface_plane, int width, int height, uint32_t textureOptions);
QSGTexture *makeCGLTexture(QQuickWindow *win, IOSurfaceRef io_surface, int width, int height, uint32_t textureOptions, uint32_t *heldTexture);
void releaseGlTexture(uint32_t);

QSGTexture *NativeSkiaOutputDevice::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer || !m_readyWithTexture)
        return nullptr;
    static const auto graphicsApi = QQuickWindow::graphicsApi();

    QSGTexture *texture = nullptr;
    gfx::ScopedIOSurface ioSurface = m_frontBuffer->ioSurface();
    if (graphicsApi == QSGRendererInterface::Metal) {
        texture = makeMetalTexture(win, ioSurface.release(), /* plane */ 0,
                                   m_shape.characterization.width(), m_shape.characterization.height(),
                                   textureOptions);
#if QT_CONFIG(opengl)
    } else if (graphicsApi == QSGRendererInterface::OpenGL) {
        uint heldTexture;
        texture = makeCGLTexture(win, ioSurface.release(),
                                 m_shape.characterization.width(), m_shape.characterization.height(),
                                 textureOptions, &heldTexture);
        m_frontBuffer->m_textureCleanup = [heldTexture]() { releaseGlTexture(heldTexture); };
#endif
    }
    if (!texture)
        qFatal("Unknown QSG graphics backend");
    return texture;
}
#elif defined(Q_OS_WIN)
QSGTexture *NativeSkiaOutputDevice::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer || !m_readyWithTexture)
        return nullptr;
    Q_ASSERT(QQuickWindow::graphicsApi() == QSGRendererInterface::Direct3D11);

    QSGTexture *texture = nullptr;
    auto overlay_image = m_frontBuffer->overlayImage();
    if (overlay_image) {
        // Pass texture between two D3D devices:
        HRESULT status = S_OK;
        HANDLE sharedHandle = nullptr;
        IDXGIResource1 *resource = nullptr;
        if (!overlay_image->nv12_texture()) {
            qWarning() << "No D3D texture";
            return nullptr;
        }
        status = overlay_image->nv12_texture()->QueryInterface(__uuidof(IDXGIResource1), (void**)&resource);
        Q_ASSERT(status == S_OK);
        status = resource->CreateSharedHandle(NULL, DXGI_SHARED_RESOURCE_READ, NULL, &sharedHandle);
        Q_ASSERT(status == S_OK);
        Q_ASSERT(sharedHandle);

        QSGRendererInterface *ri = win->rendererInterface();
        ID3D11Device1 *device = static_cast<ID3D11Device1 *>(ri->getResource(win, QSGRendererInterface::DeviceResource));

        ID3D11Texture2D *qtTexture;
        status = device->OpenSharedResource1(sharedHandle, __uuidof(ID3D11Texture2D), (void**)&qtTexture);
        if (status != S_OK) {
            qWarning("Failed to share D3D11 texture (%s). This will result in failed rendering. Report the bug, and try restarting with QTWEBENGINE_CHROMIUM_FLAGS=--disble-gpu",
                     qPrintable(QSystemError::windowsComString(status)));
            ::CloseHandle(sharedHandle);
            return nullptr;
        }

        Q_ASSERT(qtTexture);

        QQuickWindow::CreateTextureOptions texOpts(textureOptions);
        texture = QNativeInterface::QSGD3D11Texture::fromNative(qtTexture, win, size(), texOpts);

        m_frontBuffer->m_textureCleanup = [qtTexture,sharedHandle]() {
            qtTexture->Release();
            ::CloseHandle(sharedHandle);
        };
    } else {
        qWarning() << "No overlay image";
    }

    return texture;
}
#else
QSGTexture *NativeSkiaOutputDevice::texture(QQuickWindow *, uint32_t)
{
    // Add Linux versions.
    NOTIMPLEMENTED();
    return nullptr;
}
#endif

bool NativeSkiaOutputDevice::textureIsFlipped()
{
    return true;
}

QSize NativeSkiaOutputDevice::size()
{
    return m_frontBuffer ? toQt(m_frontBuffer->shape().characterization.dimensions()) : QSize();
}

bool NativeSkiaOutputDevice::requiresAlphaChannel()
{
    return m_requiresAlpha;
}

float NativeSkiaOutputDevice::devicePixelRatio()
{
    return m_frontBuffer ? m_frontBuffer->shape().devicePixelRatio : 1;
}

void NativeSkiaOutputDevice::SwapBuffersFinished()
{
    {
        QMutexLocker locker(&m_mutex);
        std::swap(m_backBuffer, m_middleBuffer);
    }

    FinishSwapBuffers(gfx::SwapCompletionResult(gfx::SwapResult::SWAP_ACK),
                      gfx::Size(m_shape.characterization.width(), m_shape.characterization.height()),
                      std::move(m_frame));
}

} // namespace QtWebEngineCore
