// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef NATIVE_SKIA_OUTPUT_DEVICE_H
#define NATIVE_SKIA_OUTPUT_DEVICE_H

#include "compositor.h"

#include "base/task/single_thread_task_runner.h"
#include "components/viz/service/display_embedder/skia_output_device.h"
#include "gpu/command_buffer/service/shared_context_state.h"
#include "gpu/command_buffer/service/shared_image/shared_image_representation.h"
#include "gpu/config/gpu_preferences.h"

#include <QMutex>

#if defined(Q_OS_WIN)
#include "ui/gl/dc_layer_overlay_image.h"
#endif

#if defined(Q_OS_MACOS)
#include "ui/gfx/mac/io_surface.h"
#endif

QT_BEGIN_NAMESPACE
class QQuickWindow;
QT_END_NAMESPACE

namespace gl {
class GLFence;
}

namespace gfx {
class GpuFence;
class NativePixmap;
}

namespace gpu {
class SharedImageFactory;
class SharedImageRepresentationFactory;
}

namespace viz {
class SkiaOutputSurfaceDependency;
}

namespace QtWebEngineCore {

class NativeSkiaOutputDevice : public viz::SkiaOutputDevice, public Compositor
{
public:
    NativeSkiaOutputDevice(scoped_refptr<gpu::SharedContextState> contextState,
                            bool requiresAlpha,
                            gpu::MemoryTracker *memoryTracker,
                            viz::SkiaOutputSurfaceDependency *dependency,
                            gpu::SharedImageFactory *shared_image_factory,
                            gpu::SharedImageRepresentationFactory *shared_image_representation_factory,
                            DidSwapBufferCompleteCallback didSwapBufferCompleteCallback);
    ~NativeSkiaOutputDevice() override;

    // Overridden from SkiaOutputDevice.
    void SetFrameSinkId(const viz::FrameSinkId &frame_sink_id) override;
    bool Reshape(const SkImageInfo &image_info,
                 const gfx::ColorSpace &color_space,
                 int sample_count,
                 float device_scale_factor,
                 gfx::OverlayTransform transform) override;
    void Present(const absl::optional<gfx::Rect>& update_rect,
                 BufferPresentedCallback feedback,
                 viz::OutputSurfaceFrame frame) override;
    void EnsureBackbuffer() override;
    void DiscardBackbuffer() override;
    SkSurface *BeginPaint(std::vector<GrBackendSemaphore> *semaphores) override;
    void EndPaint() override;

    // Overridden from Compositor.
    void swapFrame() override;
    void waitForTexture() override;
    void releaseTexture() override;
    void releaseResources() override;
    bool textureIsFlipped() override;
    QSize size() override;
    bool requiresAlphaChannel() override;
    float devicePixelRatio() override;

protected:
    struct Shape
    {
        SkImageInfo imageInfo;
        float devicePixelRatio;
        gfx::ColorSpace colorSpace;
        int sampleCount;

        bool operator==(const Shape &that) const
        {
            return (imageInfo == that.imageInfo &&
                    devicePixelRatio == that.devicePixelRatio &&
                    colorSpace == that.colorSpace &&
                    sampleCount == that.sampleCount);
        }
        bool operator!=(const Shape &that) const { return !(*this == that); }
    };

    class Buffer
    {
    public:
        Buffer(NativeSkiaOutputDevice *parent);
        ~Buffer();

        bool initialize();
        SkSurface *beginWriteSkia();
        void endWriteSkia(bool force_flush);
        std::vector<GrBackendSemaphore> takeEndWriteSkiaSemaphores();
        void beginPresent();
        void endPresent();
        void freeTexture();
        void createFence();
        void consumeFence();

        sk_sp<SkImage> skImage();
#if defined(USE_OZONE)
        scoped_refptr<gfx::NativePixmap> nativePixmap();
#elif defined(Q_OS_WIN)
        absl::optional<gl::DCLayerOverlayImage> overlayImage() const;
#elif defined(Q_OS_MACOS)
        gfx::ScopedIOSurface ioSurface() const;
#endif

        const Shape &shape() const { return m_shape; }
        viz::SharedImageFormat sharedImageFormat() const { return m_skiaRepresentation->format(); }

        std::function<void()> textureCleanupCallback;

    private:
        void createSkImageOnGPUThread();

        NativeSkiaOutputDevice *m_parent;
        Shape m_shape;
        uint64_t m_estimatedSize = 0; // FIXME: estimate size
        std::unique_ptr<gfx::GpuFence> m_acquireFence;
        std::unique_ptr<gl::GLFence> m_fence;
        gpu::Mailbox m_mailbox;
        std::unique_ptr<gpu::SkiaImageRepresentation> m_skiaRepresentation;
        std::unique_ptr<gpu::SkiaImageRepresentation::ScopedWriteAccess> m_scopedSkiaWriteAccess;
        std::unique_ptr<gpu::SkiaImageRepresentation::ScopedReadAccess> m_scopedSkiaReadAccess;
        std::unique_ptr<gpu::OverlayImageRepresentation> m_overlayRepresentation;
        std::unique_ptr<gpu::OverlayImageRepresentation::ScopedReadAccess>
                m_scopedOverlayReadAccess;
        std::vector<GrBackendSemaphore> m_endSemaphores;
        int m_presentCount = 0;

        mutable QMutex m_skImageMutex;
        sk_sp<SkImage> m_cachedSkImage;
    };

protected:
    scoped_refptr<gpu::SharedContextState> m_contextState;
    std::unique_ptr<Buffer> m_frontBuffer;
    bool m_readyWithTexture = false;
    bool m_isNativeBufferSupported = true;

private:
    friend class NativeSkiaOutputDevice::Buffer;

    void SwapBuffersFinished();

    bool m_requiresAlpha;
    gpu::SharedImageFactory *const m_factory;
    gpu::SharedImageRepresentationFactory *const m_representationFactory;
    viz::SkiaOutputSurfaceDependency *const m_deps;
    mutable QMutex m_mutex;
    Shape m_shape;
    std::unique_ptr<Buffer> m_middleBuffer;
    std::unique_ptr<Buffer> m_backBuffer;
    viz::OutputSurfaceFrame m_frame;
    bool m_readyToUpdate = false;
    scoped_refptr<base::SingleThreadTaskRunner> m_gpuTaskRunner;
};

} // namespace QtWebEngineCore

#endif // !NATIVE_SKIA_OUTPUT_DEVICE_H
