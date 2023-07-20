// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DISPLAY_SKIA_OUTPUT_DEVICE_H
#define DISPLAY_SKIA_OUTPUT_DEVICE_H

#include <QtCore/QMutex>
#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

#include "compositor.h"

#include "base/task/single_thread_task_runner.h"
#include "components/viz/service/display_embedder/skia_output_device.h"
#include "gpu/command_buffer/service/shared_context_state.h"

#if QT_CONFIG(webengine_vulkan)
#include <vulkan/vulkan.h>
#endif

QT_BEGIN_NAMESPACE
class QQuickWindow;
QT_END_NAMESPACE

namespace QtWebEngineCore {

class DisplaySkiaOutputDevice final : public viz::SkiaOutputDevice, public Compositor
{
public:
    DisplaySkiaOutputDevice(scoped_refptr<gpu::SharedContextState> contextState,
                            bool requiresAlpha,
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
    QSGTexture *texture(QQuickWindow *win, uint32_t texOpts) override;
    bool textureIsFlipped() override;
    QSize size() override;
    bool requiresAlphaChannel() override;
    float devicePixelRatio() override;

#if QT_CONFIG(webengine_vulkan)
    VkImage vkImage(QQuickWindow *win);
    VkImageLayout vkImageLayout();
    void releaseResources(QQuickWindow *win) override;
#endif

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
    bool m_requiresAlpha;
    scoped_refptr<base::SingleThreadTaskRunner> m_taskRunner;

#if QT_CONFIG(webengine_vulkan)
    VkImage m_importedImage = VK_NULL_HANDLE;
    VkDeviceMemory m_importedImageMemory = VK_NULL_HANDLE;
#endif // QT_CONFIG(webengine_vulkan)
};

} // namespace QtWebEngineCore

#endif // !DISPLAY_SKIA_OUTPUT_DEVICE_H
