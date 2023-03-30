// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef NATIVE_SKIA_OUTPUT_DEVICE_H
#define NATIVE_SKIA_OUTPUT_DEVICE_H

#include "compositor.h"

#include "base/task/single_thread_task_runner.h"
#include "components/viz/service/display_embedder/skia_output_device.h"
#include "gpu/command_buffer/service/shared_context_state.h"

#include <QMutex>

QT_BEGIN_NAMESPACE
class QQuickWindow;
QT_END_NAMESPACE

namespace gpu {
class SharedImageFactory;
class SharedImageRepresentationFactory;
}

namespace viz {
class SkiaOutputSurfaceDependency;
}

namespace QtWebEngineCore {

class NativeSkiaOutputDevice final : public viz::SkiaOutputDevice, public Compositor
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
    bool Reshape(const SkSurfaceCharacterization &characterization,
                 const gfx::ColorSpace& colorSpace,
                 float device_scale_factor,
                 gfx::OverlayTransform transform) override;
    void SwapBuffers(BufferPresentedCallback feedback,
                     viz::OutputSurfaceFrame frame) override;
    void EnsureBackbuffer() override;
    void DiscardBackbuffer() override;
    SkSurface *BeginPaint(std::vector<GrBackendSemaphore> *semaphores) override;
    void EndPaint() override;

    // Overridden from Compositor.
    void swapFrame() override;
    void waitForTexture() override;
    void releaseTexture() override;
    void releaseResources(QQuickWindow *win) override;
    QSGTexture *texture(QQuickWindow *win, uint32_t textureOptions) override;
    bool textureIsFlipped() override;
    QSize size() override;
    bool requiresAlphaChannel() override;
    float devicePixelRatio() override;

private:
    struct Shape
    {
        SkSurfaceCharacterization characterization;
        float devicePixelRatio;
        gfx::ColorSpace colorSpace;

        bool operator==(const Shape &that) const
        {
            return (characterization == that.characterization &&
                    devicePixelRatio == that.devicePixelRatio &&
                    colorSpace == that.colorSpace);
        }
        bool operator!=(const Shape &that) const { return !(*this == that); }
    };

    class Buffer;
    friend class NativeSkiaOutputDevice::Buffer;
    void SwapBuffersFinished();

    mutable QMutex m_mutex;
    Shape m_shape;
    std::unique_ptr<Buffer> m_frontBuffer;
    std::unique_ptr<Buffer> m_middleBuffer;
    std::unique_ptr<Buffer> m_backBuffer;
    viz::OutputSurfaceFrame m_frame;
    bool m_readyToUpdate = false;
    bool m_readyWithTexture = false;
    bool m_requiresAlpha;
    scoped_refptr<base::SingleThreadTaskRunner> m_taskRunner;

    const raw_ptr<gpu::SharedImageFactory> m_factory;
    const raw_ptr<gpu::SharedImageRepresentationFactory> m_representationFactory;
    const raw_ptr<viz::SkiaOutputSurfaceDependency> m_deps;
};

} // namespace QtWebEngineCore

#endif // !NATIVE_SKIA_OUTPUT_DEVICE_H
