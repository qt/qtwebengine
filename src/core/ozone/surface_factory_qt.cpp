// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#if defined(USE_OZONE)
#include "surface_factory_qt.h"

#include "ozone/gl_context_qt.h"
#include "ozone/gl_ozone_egl_qt.h"

#include "media/gpu/buildflags.h"
#include "ui/gfx/linux/drm_util_linux.h"
#include "ui/gfx/linux/gbm_buffer.h"
#include "ui/gfx/linux/gbm_device.h"
#include "ui/gfx/linux/gbm_util.h"
#include "ui/gfx/linux/native_pixmap_dmabuf.h"
#include "ui/ozone/buildflags.h"

#if BUILDFLAG(OZONE_PLATFORM_X11)
#include "ozone/gl_ozone_glx_qt.h"

#include "ui/gfx/linux/gpu_memory_buffer_support_x11.h"
#endif

#include "qtwebenginecoreglobal_p.h"

#if QT_CONFIG(webengine_vulkan)
#include "compositor/vulkan_implementation_qt.h"
#endif

namespace QtWebEngineCore {

SurfaceFactoryQt::SurfaceFactoryQt()
{
#if BUILDFLAG(OZONE_PLATFORM_X11)
    if (GLContextHelper::getGlxPlatformInterface()) {
        m_impl = { gl::GLImplementationParts(gl::kGLImplementationDesktopGL),
                   gl::GLImplementationParts(gl::kGLImplementationDisabled) };
        m_ozone.reset(new ui::GLOzoneGLXQt());
    } else
#endif
    if (GLContextHelper::getEglPlatformInterface()) {
        m_impl = { gl::GLImplementationParts(gl::kGLImplementationEGLGLES2),
                   gl::GLImplementationParts(gl::kGLImplementationDesktopGL),
                   gl::GLImplementationParts(gl::kGLImplementationDisabled) };
        m_ozone.reset(new ui::GLOzoneEGLQt());
    } else {
        m_impl = { gl::GLImplementationParts(gl::kGLImplementationDisabled) };
    }
}

std::vector<gl::GLImplementationParts> SurfaceFactoryQt::GetAllowedGLImplementations()
{
    return m_impl;
}

ui::GLOzone *SurfaceFactoryQt::GetGLOzone(const gl::GLImplementationParts &implementation)
{
    return m_ozone.get();
}
#if BUILDFLAG(ENABLE_VULKAN)
std::unique_ptr<gpu::VulkanImplementation>
SurfaceFactoryQt::CreateVulkanImplementation(bool /*allow_protected_memory*/,
                                             bool /*enforce_protected_memory*/)
{
#if QT_CONFIG(webengine_vulkan)
    return std::make_unique<gpu::VulkanImplementationQt>();
#else
    return nullptr;
#endif
}
#endif

bool SurfaceFactoryQt::CanCreateNativePixmapForFormat(gfx::BufferFormat format)
{
#if BUILDFLAG(OZONE_PLATFORM_X11)
    if (GLContextHelper::getGlxPlatformInterface())
        return ui::GpuMemoryBufferSupportX11::GetInstance()->CanCreateNativePixmapForFormat(format);
#endif

    if (GLContextHelper::getEglPlatformInterface())
        return ui::SurfaceFactoryOzone::CanCreateNativePixmapForFormat(format);

    return false;
}

scoped_refptr<gfx::NativePixmap> SurfaceFactoryQt::CreateNativePixmap(
        gfx::AcceleratedWidget widget,
        gpu::VulkanDeviceQueue *device_queue,
        gfx::Size size,
        gfx::BufferFormat format,
        gfx::BufferUsage usage,
        absl::optional<gfx::Size> framebuffer_size)
{
    Q_ASSERT(SupportsNativePixmaps());
    if (framebuffer_size && !gfx::Rect(size).Contains(gfx::Rect(*framebuffer_size)))
        return nullptr;

    std::unique_ptr<ui::GbmBuffer> gbmBuffer;

#if BUILDFLAG(OZONE_PLATFORM_X11)
    if (GLContextHelper::getGlxPlatformInterface()) {
        gbmBuffer = ui::GpuMemoryBufferSupportX11::GetInstance()->CreateBuffer(format, size, usage);
        if (!gbmBuffer)
            qFatal("Failed to create GBM buffer for GLX.");
    }
#endif

    if (GLContextHelper::getEglPlatformInterface()) {
        const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
        const uint32_t gbmFlags = ui::BufferUsageToGbmFlags(usage);

        ui::GbmDevice *gbmDevice = EGLHelper::instance()->getGbmDevice();
        // FIXME: CreateBufferWithModifiers for wayland?
        gbmBuffer = gbmDevice->CreateBuffer(fourccFormat, size, gbmFlags);
        if (!gbmBuffer)
            qFatal("Failed to create GBM buffer for EGL.");
    }

    Q_ASSERT(gbmBuffer);
    gfx::NativePixmapHandle handle = gbmBuffer->ExportHandle();
    return base::MakeRefCounted<gfx::NativePixmapDmaBuf>(size, format, std::move(handle));
}

void SurfaceFactoryQt::CreateNativePixmapAsync(
        gfx::AcceleratedWidget widget,
        gpu::VulkanDeviceQueue *device_queue,
        gfx::Size size,
        gfx::BufferFormat format,
        gfx::BufferUsage usage,
        NativePixmapCallback callback)
{
    Q_ASSERT(SupportsNativePixmaps());
    // CreateNativePixmap is non-blocking operation. Thus, it is safe to call it
    // and return the result with the provided callback.
    std::move(callback).Run(CreateNativePixmap(widget, device_queue, size, format, usage));
}

scoped_refptr<gfx::NativePixmap>
SurfaceFactoryQt::CreateNativePixmapFromHandle(
        gfx::AcceleratedWidget /*widget*/,
        gfx::Size size,
        gfx::BufferFormat format,
        gfx::NativePixmapHandle handle)
{
    Q_ASSERT(SupportsNativePixmaps());
    std::unique_ptr<ui::GbmBuffer> gbmBuffer;

#if BUILDFLAG(OZONE_PLATFORM_X11)
    if (GLContextHelper::getGlxPlatformInterface()) {
        gbmBuffer = ui::GpuMemoryBufferSupportX11::GetInstance()->CreateBufferFromHandle(
                size, format, std::move(handle));
        if (!gbmBuffer)
            qFatal("Failed to create GBM buffer for GLX.");
    }
#endif

    if (GLContextHelper::getEglPlatformInterface()) {
        const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
        ui::GbmDevice *gbmDevice = EGLHelper::instance()->getGbmDevice();
        gbmBuffer = gbmDevice->CreateBufferFromHandle(fourccFormat, size, std::move(handle));

        if (!gbmBuffer)
            qFatal("Failed to create GBM buffer for EGL.");
    }

    Q_ASSERT(gbmBuffer);
    gfx::NativePixmapHandle bufferHandle = gbmBuffer->ExportHandle();
    return base::MakeRefCounted<gfx::NativePixmapDmaBuf>(size, format, std::move(bufferHandle));
}

bool SurfaceFactoryQt::SupportsNativePixmaps() const
{
#if BUILDFLAG(OZONE_PLATFORM_X11)
    if (GLContextHelper::getGlxPlatformInterface())
        return ui::GpuMemoryBufferSupportX11::GetInstance()->has_gbm_device();
#endif

    if (GLContextHelper::getEglPlatformInterface())
        return (EGLHelper::instance()->getGbmDevice() != nullptr);

    return false;
}

} // namespace QtWebEngineCore
#endif // defined(USE_OZONE)

