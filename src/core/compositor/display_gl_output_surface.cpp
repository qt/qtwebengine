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

#include "display_gl_output_surface.h"

#include "base/threading/thread_task_runner_handle.h"
#include "components/viz/service/display/display.h"
#include "components/viz/service/display/output_surface_frame.h"
#include "gpu/command_buffer/client/gles2_implementation.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/texture_base.h"
#include "gpu/ipc/in_process_command_buffer.h"
#include "ui/gfx/buffer_format_util.h"

namespace QtWebEngineCore {

DisplayGLOutputSurface::DisplayGLOutputSurface(scoped_refptr<viz::VizProcessContextProvider> contextProvider)
    : OutputSurface(contextProvider)
    , m_commandBuffer(contextProvider->command_buffer())
    , m_gl(contextProvider->ContextGL())
    , m_vizContextProvider(contextProvider)
{
    capabilities_.uses_default_gl_framebuffer = false;
    m_gl->GenFramebuffers(1, &m_fboId);
}

DisplayGLOutputSurface::~DisplayGLOutputSurface()
{
    m_vizContextProvider->SetUpdateVSyncParametersCallback(viz::UpdateVSyncParametersCallback());
    m_gl->DeleteFramebuffers(1, &m_fboId);
    if (m_sink)
        m_sink->disconnect(this);
}

// Called from viz::Display::Initialize.
void DisplayGLOutputSurface::BindToClient(viz::OutputSurfaceClient *client)
{
    m_display = static_cast<viz::Display *>(client);
    m_sink = DisplayFrameSink::findOrCreate(m_display->frame_sink_id());
    m_sink->connect(this);
}

// Triggered by ui::Compositor::SetVisible(true).
void DisplayGLOutputSurface::EnsureBackbuffer()
{
}

// Triggered by ui::Compositor::SetVisible(false). Framebuffer must be cleared.
void DisplayGLOutputSurface::DiscardBackbuffer()
{
    NOTIMPLEMENTED();
    // m_gl->DiscardBackbufferCHROMIUM();
}

// Called from viz::DirectRenderer::DrawFrame before rendering starts, but only
// if the parameters differ from the previous Reshape call.
//
// Parameters:
//
//   - sizeInPixels comes from ui::Compositor::SetScaleAndSize via
//     viz::HostContextFactoryPrivate::ResizeDisplay.
//
//   - devicePixelRatio comes from viz::CompositorFrame::device_scale_factor()
//     via viz::RootCompositorFrameSinkImpl::SubmitCompositorFrame and
//     viz::Display::SetLocalSurfaceId.
//
//   - colorSpace and hasAlpha correspond to the color_space and
//     has_transparent_background properties of the root viz::RenderPass.
//
//   - useStencil should create a stencil buffer, but this is only needed for
//     overdraw feedback (--show-overdraw-feedback), so it's safe to ignore.
//     Accordingly, capabilities_.supports_stencil should be set to false.
//
void DisplayGLOutputSurface::Reshape(const gfx::Size &sizeInPixels,
                                     float devicePixelRatio,
                                     const gfx::ColorSpace &colorSpace,
                                     gfx::BufferFormat format,
                                     bool /*useStencil*/)
{
    bool hasAlpha = gfx::AlphaBitsForBufferFormat(format) > 0;
    m_currentShape = Shape{sizeInPixels, devicePixelRatio, colorSpace, hasAlpha};
    m_gl->ResizeCHROMIUM(sizeInPixels.width(), sizeInPixels.height(), devicePixelRatio,
                         colorSpace.AsGLColorSpace(), hasAlpha);
}

std::unique_ptr<DisplayGLOutputSurface::Buffer> DisplayGLOutputSurface::makeBuffer(const Shape &shape)
{
    std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>(this);
    buffer->shape = shape;
    m_gl->GenTextures(1, &buffer->clientId);
    m_gl->BindTexture(GL_TEXTURE_2D, buffer->clientId);
    m_gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    uint32_t width = shape.sizeInPixels.width();
    uint32_t height = shape.sizeInPixels.height();
    uint32_t format = shape.hasAlpha ? GL_RGBA : GL_RGB;
    m_gl->TexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    return buffer;
}

void DisplayGLOutputSurface::deleteBufferResources(Buffer *buffer)
{
    m_gl->DeleteTextures(1, &buffer->clientId);
}

// Called by viz::GLRenderer during rendering whenever it switches to the root
// render pass.
void DisplayGLOutputSurface::BindFramebuffer()
{
    if (!m_backBuffer || m_backBuffer->shape != m_currentShape)
        m_backBuffer = makeBuffer(m_currentShape);

    m_gl->BindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    m_gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_backBuffer->clientId, 0);
}

// Called from viz::Display::DrawAndSwap after rendering.
//
// Parameters:
//
//   - frame.size is the same as the size given to Reshape.
//
//   - frame.sub_buffer_rect and frame.content_bounds are never used since these
//     are only enabled if gl::GLSurface::SupportsPostSubBuffer() or
//     gl::GLSurface::SupportsSwapBuffersWithBounds() are true, respectively,
//     but this not the case for any offscreen gl::GLSurface.
//
//   - frame.latency_info is viz::CompositorFrame::metadata.latency_info.
void DisplayGLOutputSurface::SwapBuffers(viz::OutputSurfaceFrame frame)
{
    DCHECK(frame.size == m_currentShape.sizeInPixels);
    DCHECK(!frame.sub_buffer_rect.has_value());
    DCHECK(frame.content_bounds.empty());
    DCHECK(m_backBuffer);

    m_gl->BindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    m_gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    gpu::SyncToken syncToken;
    m_gl->GenSyncTokenCHROMIUM(syncToken.GetData());

    unsigned int clientId = m_backBuffer->clientId;

    // Now some thread-hopping:
    //
    //   - We start here on the viz thread (client side of command buffer).
    //
    //   - Then we'll jump to the gpu thread (service side of command buffer) to
    //     get the real OpenGL texture id.
    //
    //   - Then we'll get a call from the Qt Quick Scene Graph thread (could be
    //     a separate thread or the main thread).
    //
    //   - Finally we'll return to the viz thread to acknowledge the swap.

    {
        QMutexLocker locker(&m_mutex);
        m_taskRunner = base::ThreadTaskRunnerHandle::Get();
        m_middleBuffer = std::move(m_backBuffer);
        m_middleBuffer->serviceId = 0;
    }

    m_commandBuffer->GetTextureQt(
            clientId,
            base::BindOnce(&DisplayGLOutputSurface::swapBuffersOnGpuThread, base::Unretained(this)),
            std::vector<gpu::SyncToken>{syncToken});
}

void DisplayGLOutputSurface::swapBuffersOnGpuThread(unsigned int id, std::unique_ptr<gl::GLFence> fence)
{
    {
        QMutexLocker locker(&m_mutex);
        m_middleBuffer->serviceId = id;
        m_middleBuffer->fence = CompositorResourceFence::create(std::move(fence));
        m_readyToUpdate = true;
    }

    m_sink->scheduleUpdate();
}

void DisplayGLOutputSurface::swapBuffersOnVizThread()
{
    {
        QMutexLocker locker(&m_mutex);
        m_backBuffer = std::move(m_middleBuffer);
    }

    const auto now = base::TimeTicks::Now();
    m_display->DidReceiveSwapBuffersAck(gfx::SwapTimings{now, now});
    m_display->DidReceivePresentationFeedback(
            gfx::PresentationFeedback(now, base::TimeDelta(),
                                      gfx::PresentationFeedback::Flags::kVSync));
}

void DisplayGLOutputSurface::SetDrawRectangle(const gfx::Rect &)
{
}

// Returning true here will cause viz::GLRenderer to try to render the output
// surface as an overlay plane (see viz::DirectRenderer::DrawFrame and
// viz::GLRenderer::ScheduleOverlays).
bool DisplayGLOutputSurface::IsDisplayedAsOverlayPlane() const
{
    return false;
}

// Only used if IsDisplayedAsOverlayPlane was true (called from
// viz::GLRenderer::ScheduleOverlays).
unsigned DisplayGLOutputSurface::GetOverlayTextureId() const
{
    return 0;
}

// Called by viz::GLRenderer but always false in all implementations except for
// android_webview::ParentOutputSurface.
bool DisplayGLOutputSurface::HasExternalStencilTest() const
{
    return false;
}

// Only called if HasExternalStencilTest was true. Dead code?
void DisplayGLOutputSurface::ApplyExternalStencil()
{
    NOTREACHED();
}

// Called from GLRenderer::GetFramebufferCopyTextureFormat when using
// glCopyTexSubImage2D on our framebuffer.
uint32_t DisplayGLOutputSurface::GetFramebufferCopyTextureFormat()
{
    return m_currentShape.hasAlpha ? GL_RGBA : GL_RGB;
}

// Called from viz::DirectRenderer::DrawFrame, only used for overlays.
unsigned DisplayGLOutputSurface::UpdateGpuFence()
{
    NOTREACHED();
    return 0;
}

scoped_refptr<gpu::GpuTaskSchedulerHelper> DisplayGLOutputSurface::GetGpuTaskSchedulerHelper()
{
    return m_vizContextProvider->GetGpuTaskSchedulerHelper();
}

gpu::MemoryTracker *DisplayGLOutputSurface::GetMemoryTracker()
{
    return m_vizContextProvider->GetMemoryTracker();
}

void DisplayGLOutputSurface::SetUpdateVSyncParametersCallback(viz::UpdateVSyncParametersCallback callback)
{
    m_vizContextProvider->SetUpdateVSyncParametersCallback(std::move(callback));
}

void DisplayGLOutputSurface::SetDisplayTransformHint(gfx::OverlayTransform)
{
}

gfx::OverlayTransform DisplayGLOutputSurface::GetDisplayTransform()
{
    return gfx::OVERLAY_TRANSFORM_NONE;
}

} // namespace QtWebEngineCore
