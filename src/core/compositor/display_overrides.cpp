// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "display_gl_output_surface.h"
#include "display_skia_output_device.h"
#include "display_software_output_surface.h"

#include "components/viz/service/display_embedder/output_surface_provider_impl.h"
#include "components/viz/service/display_embedder/skia_output_surface_impl_on_gpu.h"
#include "gpu/ipc/in_process_command_buffer.h"
#include <qtgui-config.h>

std::unique_ptr<viz::OutputSurface>
viz::OutputSurfaceProviderImpl::CreateGLOutputSurface(
        scoped_refptr<VizProcessContextProvider> context_provider)
{
#if QT_CONFIG(opengl)
    return std::make_unique<QtWebEngineCore::DisplayGLOutputSurface>(std::move(context_provider));
#else
    return nullptr;
#endif // QT_CONFIG(opengl)
}

std::unique_ptr<viz::OutputSurface>
viz::OutputSurfaceProviderImpl::CreateSoftwareOutputSurface()
{
    return std::make_unique<QtWebEngineCore::DisplaySoftwareOutputSurface>();
}

std::unique_ptr<viz::SkiaOutputDevice>
viz::SkiaOutputSurfaceImplOnGpu::CreateOutputDevice()
{
#if QT_CONFIG(opengl)
    return std::make_unique<QtWebEngineCore::DisplaySkiaOutputDevice>(
            context_state_,
            shared_gpu_deps_->memory_tracker(),
            GetDidSwapBuffersCompleteCallback());
#else
    return nullptr;
#endif // QT_CONFIG(opengl)
}

void gpu::InProcessCommandBuffer::GetTextureQt(
        unsigned int client_id,
        GetTextureCallback callback,
        const std::vector<SyncToken>& sync_token_fences)
{
    ScheduleGpuTask(base::BindOnce(&InProcessCommandBuffer::GetTextureQtOnGpuThread,
                                   gpu_thread_weak_ptr_factory_.GetWeakPtr(),
                                   client_id,
                                   std::move(callback)),
                    sync_token_fences);
}

void gpu::InProcessCommandBuffer::GetTextureQtOnGpuThread(
        unsigned int client_id, GetTextureCallback callback)
{
    if (!MakeCurrent()) {
        LOG(ERROR) << "MakeCurrent failed for GetTextureQt";
        return;
    }
    gpu::TextureBase *texture = decoder_->GetTextureBase(client_id);
    std::move(callback).Run(texture ? texture->service_id() : 0, gl::GLFence::Create());
}
