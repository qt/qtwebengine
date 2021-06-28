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
    bool Reshape(const gfx::Size& size,
                 float devicePixelRatio,
                 const gfx::ColorSpace& colorSpace,
                 gfx::BufferFormat format,
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
        gfx::Size sizeInPixels;
        float devicePixelRatio;
        gfx::ColorSpace colorSpace;
        gfx::BufferFormat format;

        bool operator==(const Shape &that) const
        {
            return (sizeInPixels == that.sizeInPixels &&
                    devicePixelRatio == that.devicePixelRatio &&
                    colorSpace == that.colorSpace &&
                    format == that.format);
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
