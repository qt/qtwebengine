// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DISPLAY_SKIA_OUTPUT_DEVICE_H
#define DISPLAY_SKIA_OUTPUT_DEVICE_H

#include "compositor_resource_fence.h"
#include "compositor.h"

#include "base/threading/thread_task_runner_handle.h"
#include "components/viz/service/display_embedder/skia_output_device.h"
#include "gpu/command_buffer/service/shared_context_state.h"

#include <QMutex>

namespace QtWebEngineCore {

class DisplaySkiaOutputDevice final : public viz::SkiaOutputDevice, public Compositor
{
public:
    DisplaySkiaOutputDevice(scoped_refptr<gpu::SharedContextState> contextState,
                            gpu::MemoryTracker *memoryTracker,
                            DidSwapBufferCompleteCallback didSwapBufferCompleteCallback);
    ~DisplaySkiaOutputDevice() override;

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
    int textureId() override;
    QSize size() override;
    bool hasAlphaChannel() override;
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

    void SwapBuffersFinished();

    mutable QMutex m_mutex;
    scoped_refptr<gpu::SharedContextState> m_contextState;
    Shape m_shape;
    std::unique_ptr<Buffer> m_frontBuffer;
    std::unique_ptr<Buffer> m_middleBuffer;
    std::unique_ptr<Buffer> m_backBuffer;
    viz::OutputSurfaceFrame m_frame;
    bool m_readyToUpdate = false;
    scoped_refptr<base::SingleThreadTaskRunner> m_taskRunner;
};

} // namespace QtWebEngineCore

#endif // !DISPLAY_SKIA_OUTPUT_DEVICE_H
