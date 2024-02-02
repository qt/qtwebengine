// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "display_skia_output_device.h"
#include "display_software_output_surface.h"
#include "native_skia_output_device.h"

#include "components/viz/service/display_embedder/output_surface_provider_impl.h"
#include "components/viz/service/display_embedder/skia_output_surface_impl_on_gpu.h"
#include "gpu/ipc/in_process_command_buffer.h"

#include <qtgui-config.h>

std::unique_ptr<viz::OutputSurface>
viz::OutputSurfaceProviderImpl::CreateSoftwareOutputSurface(const RendererSettings &renderer_settings)
{
    return std::make_unique<QtWebEngineCore::DisplaySoftwareOutputSurface>(renderer_settings.requires_alpha_channel);
}

std::unique_ptr<viz::SkiaOutputDevice>
viz::SkiaOutputSurfaceImplOnGpu::CreateOutputDevice()
{
    if (gl::GetGLImplementation() == gl::kGLImplementationEGLANGLE) {
        return std::make_unique<QtWebEngineCore::NativeSkiaOutputDevice>(
                context_state_,
                renderer_settings_.requires_alpha_channel,
                shared_gpu_deps_->memory_tracker(),
                dependency_.get(),
                shared_image_factory_.get(),
                shared_image_representation_factory_.get(),
                GetDidSwapBuffersCompleteCallback());
    }
#ifdef Q_OS_MACOS
    qFatal("macOS only supports ANGLE");
#endif
#if QT_CONFIG(opengl)
    return std::make_unique<QtWebEngineCore::DisplaySkiaOutputDevice>(
            context_state_,
            renderer_settings_.requires_alpha_channel,
            shared_gpu_deps_->memory_tracker(),
            GetDidSwapBuffersCompleteCallback());
#else
    return nullptr;
#endif // QT_CONFIG(opengl)
}
