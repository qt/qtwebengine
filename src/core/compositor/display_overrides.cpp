// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "display_skia_output_device.h"
#include "display_software_output_surface.h"
#include "native_skia_output_device.h"

#include "components/viz/service/display_embedder/output_surface_provider_impl.h"
#include "components/viz/service/display_embedder/skia_output_surface_impl_on_gpu.h"
#include "gpu/ipc/in_process_command_buffer.h"

#include <qtgui-config.h>
#include <QtQuick/qquickwindow.h>

#if QT_CONFIG(opengl)
#include "native_skia_output_device_opengl.h"
#endif

#if BUILDFLAG(ENABLE_VULKAN)
#include "native_skia_output_device_vulkan.h"
#endif

#if defined(Q_OS_WIN)
#include "native_skia_output_device_direct3d11.h"
#endif

#if defined(Q_OS_MACOS)
#include "native_skia_output_device_metal.h"
#endif

std::unique_ptr<viz::OutputSurface>
viz::OutputSurfaceProviderImpl::CreateSoftwareOutputSurface(const RendererSettings &renderer_settings)
{
    return std::make_unique<QtWebEngineCore::DisplaySoftwareOutputSurface>(renderer_settings.requires_alpha_channel);
}

std::unique_ptr<viz::SkiaOutputDevice>
viz::SkiaOutputSurfaceImplOnGpu::CreateOutputDevice()
{
    static const auto graphicsApi = QQuickWindow::graphicsApi();

#if QT_CONFIG(opengl)
    if (graphicsApi == QSGRendererInterface::OpenGL) {
        if (gl::GetGLImplementation() != gl::kGLImplementationEGLANGLE) {
#if !defined(Q_OS_MACOS)
            if (context_state_->gr_context_type() == gpu::GrContextType::kGL) {
                return std::make_unique<QtWebEngineCore::DisplaySkiaOutputDevice>(
                        context_state_, renderer_settings_.requires_alpha_channel,
                        shared_gpu_deps_->memory_tracker(), GetDidSwapBuffersCompleteCallback());
            }
#else
            qFatal("macOS only supports ANGLE.");
#endif // !defined(Q_OS_MACOS)
        }

        return std::make_unique<QtWebEngineCore::NativeSkiaOutputDeviceOpenGL>(
                context_state_, renderer_settings_.requires_alpha_channel,
                shared_gpu_deps_->memory_tracker(), dependency_.get(), shared_image_factory_.get(),
                shared_image_representation_factory_.get(), GetDidSwapBuffersCompleteCallback());
    }
#endif // QT_CONFIG(opengl)

#if BUILDFLAG(ENABLE_VULKAN)
    if (graphicsApi == QSGRendererInterface::Vulkan) {
#if !defined(Q_OS_MACOS)
        return std::make_unique<QtWebEngineCore::NativeSkiaOutputDeviceVulkan>(
                context_state_, renderer_settings_.requires_alpha_channel,
                shared_gpu_deps_->memory_tracker(), dependency_.get(), shared_image_factory_.get(),
                shared_image_representation_factory_.get(), GetDidSwapBuffersCompleteCallback());
#else
        qFatal("Vulkan is not supported on macOS.");
#endif // !defined(Q_OS_MACOS)
    }
#endif // BUILDFLAG(ENABLE_VULKAN)

#if defined(Q_OS_WIN)
    if (graphicsApi == QSGRendererInterface::Direct3D11) {
        if (gl::GetGLImplementation() != gl::kGLImplementationEGLANGLE)
            qFatal("Direct3D11 is only supported over ANGLE.");

        return std::make_unique<QtWebEngineCore::NativeSkiaOutputDeviceDirect3D11>(
                context_state_, renderer_settings_.requires_alpha_channel,
                shared_gpu_deps_->memory_tracker(), dependency_.get(), shared_image_factory_.get(),
                shared_image_representation_factory_.get(), GetDidSwapBuffersCompleteCallback());
    }
#endif

#if defined(Q_OS_MACOS)
    if (graphicsApi == QSGRendererInterface::Metal) {
        if (gl::GetGLImplementation() != gl::kGLImplementationEGLANGLE)
            qFatal("Metal is only supported over ANGLE.");

        return std::make_unique<QtWebEngineCore::NativeSkiaOutputDeviceMetal>(
                context_state_, renderer_settings_.requires_alpha_channel,
                shared_gpu_deps_->memory_tracker(), dependency_.get(), shared_image_factory_.get(),
                shared_image_representation_factory_.get(), GetDidSwapBuffersCompleteCallback());
    }
#endif

    qFatal() << "Unsupported Graphics API:" << graphicsApi;
    return nullptr;
}
