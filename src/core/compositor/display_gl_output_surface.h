// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DISPLAY_GL_OUTPUT_SURFACE_H
#define DISPLAY_GL_OUTPUT_SURFACE_H

#include "compositor_resource_fence.h"
#include "compositor.h"

#include "components/viz/common/display/update_vsync_parameters_callback.h"
#include "components/viz/service/display/output_surface.h"
#include "components/viz/service/display_embedder/viz_process_context_provider.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/command_buffer/common/sync_token.h"

#include <QMutex>

namespace QtWebEngineCore {

class DisplayGLOutputSurface final : public viz::OutputSurface, public Compositor
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
    void SetFrameSinkId(const viz::FrameSinkId &id) override;

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
    viz::OutputSurfaceClient *m_client = nullptr;
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
