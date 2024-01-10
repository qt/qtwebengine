// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#if defined(USE_OZONE)
#include "surface_factory_qt.h"

#include "ozone/gl_context_qt.h"
#include "ozone/gl_ozone_egl_qt.h"

#include "media/gpu/buildflags.h"
#include "ui/gfx/linux/native_pixmap_dmabuf.h"

#if defined(USE_GLX)
#include "ozone/gl_ozone_glx_qt.h"

#include "ui/gfx/linux/gbm_buffer.h"
#include "ui/gfx/linux/gpu_memory_buffer_support_x11.h"
#endif

#include "qtwebenginecoreglobal_p.h"

#if QT_CONFIG(webengine_vulkan)
#include "compositor/vulkan_implementation_qt.h"
#endif

namespace QtWebEngineCore {

SurfaceFactoryQt::SurfaceFactoryQt()
{
#if defined(USE_GLX)
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
#if BUILDFLAG(USE_VAAPI)
#if defined(USE_GLX)
    if (GLContextHelper::getGlxPlatformInterface())
        return ui::GpuMemoryBufferSupportX11::GetInstance()->CanCreateNativePixmapForFormat(format);
#endif
    return ui::SurfaceFactoryOzone::CanCreateNativePixmapForFormat(format);
#else  // !BUILDFLAG(USE_VAAPI)
    return false;
#endif // BUILDFLAG(USE_VAAPI)
}

scoped_refptr<gfx::NativePixmap> SurfaceFactoryQt::CreateNativePixmap(
        gfx::AcceleratedWidget widget,
        gpu::VulkanDeviceQueue *device_queue,
        gfx::Size size,
        gfx::BufferFormat format,
        gfx::BufferUsage usage,
        absl::optional<gfx::Size> framebuffer_size)
{
    if (framebuffer_size && !gfx::Rect(size).Contains(gfx::Rect(*framebuffer_size)))
        return nullptr;
#if defined(USE_GLX) && BUILDFLAG(USE_VAAPI)
    if (GLContextHelper::getGlxPlatformInterface()) {
        scoped_refptr<gfx::NativePixmapDmaBuf> pixmap;
        auto buffer = ui::GpuMemoryBufferSupportX11::GetInstance()->CreateBuffer(format, size, usage);
        if (buffer) {
            gfx::NativePixmapHandle handle = buffer->ExportHandle();
            pixmap = base::MakeRefCounted<gfx::NativePixmapDmaBuf>(size, format, std::move(handle));
        }
        // CreateNativePixmap is non-blocking operation. Thus, it is safe to call it
        // and return the result with the provided callback.
        return pixmap;
    }
#endif
    // FIXME: No EGL implementation.
    return nullptr;
}

void SurfaceFactoryQt::CreateNativePixmapAsync(
        gfx::AcceleratedWidget widget,
        gpu::VulkanDeviceQueue *device_queue,
        gfx::Size size,
        gfx::BufferFormat format,
        gfx::BufferUsage usage,
        NativePixmapCallback callback)
{
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
#if BUILDFLAG(USE_VAAPI)
#if defined(USE_GLX)
    if (GLContextHelper::getGlxPlatformInterface()) {
        scoped_refptr<gfx::NativePixmapDmaBuf> pixmap;
        auto buffer = ui::GpuMemoryBufferSupportX11::GetInstance()->CreateBufferFromHandle(size, format, std::move(handle));
        if (buffer) {
            gfx::NativePixmapHandle buffer_handle = buffer->ExportHandle();
            pixmap = base::MakeRefCounted<gfx::NativePixmapDmaBuf>(size, format, std::move(buffer_handle));
        }
        return pixmap;
    }
#endif
    if (GLContextHelper::getEglPlatformInterface())
        return base::MakeRefCounted<gfx::NativePixmapDmaBuf>(size, format, std::move(handle));
#endif
    return nullptr;
}

} // namespace QtWebEngineCore
#endif // defined(USE_OZONE)

