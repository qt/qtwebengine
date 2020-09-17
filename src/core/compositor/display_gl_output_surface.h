/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef DISPLAY_GL_OUTPUT_SURFACE_H
#define DISPLAY_GL_OUTPUT_SURFACE_H

#include "compositor_resource_fence.h"
#include "display_frame_sink.h"

#include "components/viz/common/display/update_vsync_parameters_callback.h"
#include "components/viz/service/display/output_surface.h"
#include "components/viz/service/display_embedder/viz_process_context_provider.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/command_buffer/common/sync_token.h"

namespace viz {
class Display;
class SyntheticBeginFrameSource;
} // namespace viz

namespace QtWebEngineCore {

// NOTE: Some methods are defined in display_gl_output_surface_qsg.cpp due
// to conflicts between Qt & Chromium OpenGL APIs.
class DisplayGLOutputSurface final : public viz::OutputSurface, public DisplayProducer
{
public:
    DisplayGLOutputSurface(scoped_refptr<viz::VizProcessContextProvider> contextProvider);
    ~DisplayGLOutputSurface() override;

    // Overridden from viz::OutputSurface.
    void BindToClient(viz::OutputSurfaceClient *client) override;
    void EnsureBackbuffer() override;
    void DiscardBackbuffer() override;
    void BindFramebuffer() override;
    void SetDrawRectangle(const gfx::Rect &drawRect) override;
    bool IsDisplayedAsOverlayPlane() const override;
    unsigned GetOverlayTextureId() const override;
    void Reshape(const gfx::Size &size,
                 float devicePixelRatio,
                 const gfx::ColorSpace &colorSpace,
                 gfx::BufferFormat format,
                 bool useStencil) override;
    bool HasExternalStencilTest() const override;
    void ApplyExternalStencil() override;
    uint32_t GetFramebufferCopyTextureFormat() override;
    void SwapBuffers(viz::OutputSurfaceFrame frame) override;
    unsigned UpdateGpuFence() override;
    void SetUpdateVSyncParametersCallback(viz::UpdateVSyncParametersCallback callback) override;
    void SetDisplayTransformHint(gfx::OverlayTransform transform) override;
    gfx::OverlayTransform GetDisplayTransform() override;
    scoped_refptr<gpu::GpuTaskSchedulerHelper> GetGpuTaskSchedulerHelper() override;
    gpu::MemoryTracker *GetMemoryTracker() override;

    // Overridden from DisplayProducer.
    QSGNode *updatePaintNode(QSGNode *oldNode, RenderWidgetHostViewQtDelegate *delegate) override;

private:
    struct Shape
    {
        gfx::Size sizeInPixels;
        float devicePixelRatio;
        gfx::ColorSpace colorSpace;
        bool hasAlpha;

        bool operator==(const Shape &that) const
        {
            return (sizeInPixels == that.sizeInPixels &&
                    devicePixelRatio == that.devicePixelRatio &&
                    colorSpace == that.colorSpace &&
                    hasAlpha == that.hasAlpha);
        }
        bool operator!=(const Shape &that) const { return !(*this == that); }
    };

    struct Buffer
    {
        DisplayGLOutputSurface *parent;
        Shape shape;
        uint32_t clientId = 0;
        uint32_t serviceId = 0;
        scoped_refptr<CompositorResourceFence> fence;

        Buffer(DisplayGLOutputSurface *parent) : parent(parent) {}
        ~Buffer() { parent->deleteBufferResources(this); }
    };

    class Texture;

    void swapBuffersOnGpuThread(unsigned int id, std::unique_ptr<gl::GLFence> fence);
    void swapBuffersOnVizThread();

    std::unique_ptr<Buffer> makeBuffer(const Shape &shape);
    void deleteBufferResources(Buffer *buffer);
    void attachBuffer();
    void detachBuffer();

    gpu::InProcessCommandBuffer *const m_commandBuffer;
    gpu::gles2::GLES2Interface *const m_gl;
    mutable QMutex m_mutex;
    uint32_t m_fboId = 0;
    viz::Display *m_display = nullptr;
    scoped_refptr<DisplayFrameSink> m_sink;
    Shape m_currentShape;
    std::unique_ptr<Buffer> m_backBuffer;
    std::unique_ptr<Buffer> m_middleBuffer;
    std::unique_ptr<Buffer> m_frontBuffer;
    bool m_readyToUpdate = false;
    scoped_refptr<base::SingleThreadTaskRunner> m_taskRunner;
    scoped_refptr<viz::VizProcessContextProvider> m_vizContextProvider;
};

} // namespace QtWebEngineCore

#endif // !DISPLAY_GL_OUTPUT_SURFACE_H
