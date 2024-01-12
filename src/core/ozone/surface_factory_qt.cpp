// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#if defined(USE_OZONE)
#include "surface_factory_qt.h"

#include "qtwebenginecoreglobal_p.h"
#include "ozone/gl_context_qt.h"
#include "ozone/gl_ozone_angle_qt.h"
#include "ozone/gl_ozone_egl_qt.h"
#include "qtwebenginecoreglobal_p.h"

#include "media/gpu/buildflags.h"
#include "ui/base/ozone_buildflags.h"
#include "ui/gfx/linux/drm_util_linux.h"
#include "ui/gfx/linux/gbm_buffer.h"
#include "ui/gfx/linux/native_pixmap_dmabuf.h"
#include "ui/gl/egl_util.h"

#include <QDebug>
#include <QtGui/qtgui-config.h>

#if BUILDFLAG(IS_OZONE_X11)
#include "ozone/gl_ozone_glx_qt.h"

#include "ui/gfx/linux/gpu_memory_buffer_support_x11.h"
#endif

#if QT_CONFIG(webengine_vulkan)
#include "compositor/vulkan_implementation_qt.h"
#endif

namespace QtWebEngineCore {

SurfaceFactoryQt::SurfaceFactoryQt()
{
#if BUILDFLAG(IS_OZONE_X11)
    if (GLContextHelper::getGlxPlatformInterface()) {
        m_impls.push_back({ gl::GLImplementationParts(gl::kGLImplementationDesktopGL),
                            std::make_unique<ui::GLOzoneGLXQt>() });
    } else
#endif
    if (GLContextHelper::getEglPlatformInterface()) {
        m_impls.push_back({ gl::GLImplementationParts(gl::kGLImplementationEGLGLES2),
                            std::make_unique<ui::GLOzoneEGLQt>() });
        m_impls.push_back({ gl::GLImplementationParts(gl::kGLImplementationDesktopGL),
                            std::make_unique<ui::GLOzoneEGLQt>() });
    }

    m_impls.push_back({ gl::GLImplementationParts(gl::kGLImplementationEGLANGLE),
                        std::make_unique<ui::GLOzoneANGLEQt>() });
    m_impls.push_back({ gl::GLImplementationParts(gl::kGLImplementationDisabled), nullptr });
}

std::vector<gl::GLImplementationParts> SurfaceFactoryQt::GetAllowedGLImplementations()
{
    std::vector<gl::GLImplementationParts> allowed;
    for (const auto &impl : m_impls)
        allowed.push_back(impl.first);

    return allowed;
}

ui::GLOzone *SurfaceFactoryQt::GetGLOzone(const gl::GLImplementationParts &implementation)
{
    for (const auto &impl : m_impls) {
        if (impl.first.gl == implementation.gl)
            return impl.second.get();
    }

    qFatal() << "GLOzone not found for" << gl::GetGLImplementationGLName(implementation);
    return nullptr;
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
#if BUILDFLAG(IS_OZONE_X11)
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
    if (!SupportsNativePixmaps())
        return nullptr;

#if QT_CONFIG(opengl)
    if (framebuffer_size && !gfx::Rect(size).Contains(gfx::Rect(*framebuffer_size)))
        return nullptr;

    gfx::NativePixmapHandle handle;

#if BUILDFLAG(IS_OZONE_X11)
    if (GLContextHelper::getGlxPlatformInterface()) {
        auto gbmBuffer =
                ui::GpuMemoryBufferSupportX11::GetInstance()->CreateBuffer(format, size, usage);
        if (!gbmBuffer)
            qFatal("Failed to create GBM buffer for GLX.");
        handle = gbmBuffer->ExportHandle();
    }
#endif

    if (GLContextHelper::getEglPlatformInterface()) {
        int fd = -1;
        int stride;
        int offset;
        uint64_t modifiers;
        EGLHelper::instance()->queryDmaBuf(size.width(), size.height(), &fd, &stride, &offset,
                                           &modifiers);
        if (fd == -1)
            qFatal("Failed to query DRM FD for EGL.");

        const uint64_t planeSize = uint64_t(size.width()) * size.height() * 4;
        gfx::NativePixmapPlane plane(stride, offset, planeSize, base::ScopedFD(::dup(fd)));

        handle.planes.push_back(std::move(plane));
        handle.modifier = modifiers;
    }

    return base::MakeRefCounted<gfx::NativePixmapDmaBuf>(size, format, std::move(handle));
#else
    return nullptr;
#endif // QT_CONFIG(opengl)
}

void SurfaceFactoryQt::CreateNativePixmapAsync(
        gfx::AcceleratedWidget widget,
        gpu::VulkanDeviceQueue *device_queue,
        gfx::Size size,
        gfx::BufferFormat format,
        gfx::BufferUsage usage,
        NativePixmapCallback callback)
{
    if (!SupportsNativePixmaps()) {
        std::move(callback).Run(nullptr);
        return;
    }

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
    if (!SupportsNativePixmaps())
        return nullptr;

#if QT_CONFIG(opengl)
    gfx::NativePixmapHandle bufferHandle;

#if BUILDFLAG(IS_OZONE_X11)
    if (GLContextHelper::getGlxPlatformInterface()) {
        auto gbmBuffer = ui::GpuMemoryBufferSupportX11::GetInstance()->CreateBufferFromHandle(
                size, format, std::move(handle));
        if (!gbmBuffer)
            qFatal("Failed to create GBM buffer for GLX.");
        bufferHandle = gbmBuffer->ExportHandle();
    }
#endif

    if (GLContextHelper::getEglPlatformInterface()) {
        const size_t numPlanes = handle.planes.size();
        const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);

        std::vector<EGLAttrib> attrs;
        attrs.push_back(EGL_WIDTH);
        attrs.push_back(size.width());
        attrs.push_back(EGL_HEIGHT);
        attrs.push_back(size.height());
        attrs.push_back(EGL_LINUX_DRM_FOURCC_EXT);
        attrs.push_back(fourccFormat);
        for (size_t planeIndex = 0; planeIndex < numPlanes; ++planeIndex) {
            attrs.push_back(EGL_DMA_BUF_PLANE0_FD_EXT + planeIndex * 3);
            attrs.push_back(handle.planes[planeIndex].fd.get());
            attrs.push_back(EGL_DMA_BUF_PLANE0_OFFSET_EXT + planeIndex * 3);
            attrs.push_back(handle.planes[planeIndex].offset);
            attrs.push_back(EGL_DMA_BUF_PLANE0_PITCH_EXT + planeIndex * 3);
            attrs.push_back(handle.planes[planeIndex].stride);
            attrs.push_back(EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT + planeIndex * 2);
            attrs.push_back(handle.modifier & 0xffffffff);
            attrs.push_back(EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT + planeIndex * 2);
            attrs.push_back(handle.modifier >> 32);
        }
        attrs.push_back(EGL_NONE);

        EGLDisplay eglDisplay = GLContextHelper::getEGLDisplay();
        EGLHelper *eglHelper = EGLHelper::instance();
        auto *eglFun = eglHelper->functions();

        EGLImage eglImage =
                eglFun->eglCreateImage(eglDisplay, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
                                       (EGLClientBuffer)NULL, attrs.data());
        if (eglImage == EGL_NO_IMAGE_KHR) {
            qFatal() << "Failed to import EGLImage:"
                     << ui::GetEGLErrorString(eglFun->eglGetError());
        }

        Q_ASSERT(numPlanes <= 3);
        int fds[3];
        int strides[3];
        int offsets[3];
        if (!eglFun->eglExportDMABUFImageMESA(eglDisplay, eglImage, fds, strides, offsets)) {
            qFatal() << "Failed to export EGLImage:"
                     << ui::GetEGLErrorString(eglFun->eglGetError());
        }

        bufferHandle.modifier = handle.modifier;
        for (size_t i = 0; i < numPlanes; ++i) {
            int fd = fds[i];
            int stride = strides[i];
            int offset = offsets[i];
            int planeSize = handle.planes[i].size;

            if (fd == -1) {
                fd = fds[0];
                stride = handle.planes[i].stride;
                offset = handle.planes[i].offset;
            }

            gfx::NativePixmapPlane plane(stride, offset, planeSize, base::ScopedFD(::dup(fd)));
            bufferHandle.planes.push_back(std::move(plane));
        }

        eglFun->eglDestroyImage(eglDisplay, eglImage);
    }

    return base::MakeRefCounted<gfx::NativePixmapDmaBuf>(size, format, std::move(bufferHandle));
#else
    return nullptr;
#endif // QT_CONFIG(opengl)
}

bool SurfaceFactoryQt::SupportsNativePixmaps()
{
#if QT_CONFIG(opengl)
#if BUILDFLAG(IS_OZONE_X11)
    if (GLContextHelper::getGlxPlatformInterface())
        return ui::GpuMemoryBufferSupportX11::GetInstance()->has_gbm_device();
#endif // BUILDFLAG(IS_OZONE_X11)

    if (GLContextHelper::getEglPlatformInterface())
        return EGLHelper::instance()->isDmaBufSupported();
#endif // QT_CONFIG(opengl)

    return false;
}

} // namespace QtWebEngineCore
#endif // defined(USE_OZONE)

