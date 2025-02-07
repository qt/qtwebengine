// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "surface_factory_qt.h"

#include "qtwebenginecoreglobal_p.h"
#include "ozone/gl_ozone_angle_qt.h"
#include "ozone/ozone_util_qt.h"
#include "qtwebenginecoreglobal_p.h"

#include "media/gpu/buildflags.h"
#include "ui/base/ozone_buildflags.h"
#include "ui/gfx/buffer_format_util.h"
#include "ui/gfx/linux/drm_util_linux.h"
#include "ui/gfx/linux/gbm_buffer.h"
#include "ui/gfx/linux/native_pixmap_dmabuf.h"

#include <QDebug>
#include <QtGui/qtgui-config.h>

#if QT_CONFIG(opengl) && BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
#include "ozone/glx_helper.h"
#include "ui/gfx/linux/gpu_memory_buffer_support_x11.h"
#endif

#if QT_CONFIG(egl)
#include "ozone/egl_helper.h"
#endif

#if QT_CONFIG(webengine_vulkan)
#include "compositor/vulkan_implementation_qt.h"
#endif

namespace QtWebEngineCore {

SurfaceFactoryQt::SurfaceFactoryQt()
{
#if QT_CONFIG(opengl)
    m_impls.push_back({ gl::GLImplementationParts(gl::kGLImplementationEGLANGLE),
                        std::make_unique<ui::GLOzoneANGLEQt>() });
#endif
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

    qFatal("GLOzone not found for %s", gl::GetGLImplementationGLName(implementation));
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
#if QT_CONFIG(opengl)
#if BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
    if (OzoneUtilQt::usingGLX())
        return ui::GpuMemoryBufferSupportX11::GetInstance()->CanCreateNativePixmapForFormat(format);
#endif

#if QT_CONFIG(egl)
    if (OzoneUtilQt::usingEGL()) {
        // Multiplanar format support is not yet implemented. See EGLHelper::queryDmaBuf().
        if (gfx::BufferFormatIsMultiplanar(format))
            return false;
        return ui::SurfaceFactoryOzone::CanCreateNativePixmapForFormat(format);
    }
#endif
#endif // QT_CONFIG(opengl)

    return false;
}

scoped_refptr<gfx::NativePixmap> SurfaceFactoryQt::CreateNativePixmap(
        gfx::AcceleratedWidget widget,
        gpu::VulkanDeviceQueue *device_queue,
        gfx::Size size,
        gfx::BufferFormat format,
        gfx::BufferUsage usage,
        std::optional<gfx::Size> framebuffer_size)
{
    if (!SupportsNativePixmaps())
        return nullptr;

#if QT_CONFIG(opengl)
    if (framebuffer_size && !gfx::Rect(size).Contains(gfx::Rect(*framebuffer_size)))
        return nullptr;

    // Multiplanar format support is not yet implemented. It was not necessary with ANGLE at the
    // time when the assert was added.
    Q_ASSERT(!gfx::BufferFormatIsMultiplanar(format));

    gfx::NativePixmapHandle handle;

#if BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
    if (OzoneUtilQt::usingGLX()) {
        auto gbmBuffer =
                ui::GpuMemoryBufferSupportX11::GetInstance()->CreateBuffer(format, size, usage);
        if (!gbmBuffer)
            qFatal("Failed to create GBM buffer for GLX.");
        handle = gbmBuffer->ExportHandle();
    }
#endif

#if QT_CONFIG(egl)
    if (OzoneUtilQt::usingEGL()) {
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
#endif

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

#if BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
    if (OzoneUtilQt::usingGLX()) {
        auto gbmBuffer = ui::GpuMemoryBufferSupportX11::GetInstance()->CreateBufferFromHandle(
                size, format, std::move(handle));
        if (!gbmBuffer)
            qFatal("Failed to create GBM buffer for GLX.");
        bufferHandle = gbmBuffer->ExportHandle();
    }
#endif

#if QT_CONFIG(egl)
    if (OzoneUtilQt::usingEGL()) {
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

        EGLHelper *eglHelper = EGLHelper::instance();
        auto *eglFun = eglHelper->functions();
        EGLDisplay eglDisplay = eglHelper->getEGLDisplay();

        EGLImage eglImage =
                eglFun->eglCreateImage(eglDisplay, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
                                       (EGLClientBuffer)NULL, attrs.data());
        if (eglImage == EGL_NO_IMAGE_KHR) {
            qFatal("Failed to import EGLImage: %s", eglHelper->getLastEGLErrorString());
        }

        Q_ASSERT(numPlanes <= 3);
        int fds[3];
        int strides[3];
        int offsets[3];
        if (!eglFun->eglExportDMABUFImageMESA(eglDisplay, eglImage, fds, strides, offsets)) {
            qFatal("Failed to export EGLImage: %s", eglHelper->getLastEGLErrorString());
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
#endif // QT_CONFIG(egl)

    return base::MakeRefCounted<gfx::NativePixmapDmaBuf>(size, format, std::move(bufferHandle));
#else
    return nullptr;
#endif // QT_CONFIG(opengl)
}

bool SurfaceFactoryQt::SupportsNativePixmaps()
{
#if QT_CONFIG(opengl)
#if BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
    if (OzoneUtilQt::usingGLX())
        return GLXHelper::instance()->isDmaBufSupported();
#endif // BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)

#if QT_CONFIG(egl)
    if (OzoneUtilQt::usingEGL())
        return EGLHelper::instance()->isDmaBufSupported();
#endif // QT_CONFIG(egl)
#endif // QT_CONFIG(opengl)

    return false;
}

} // namespace QtWebEngineCore
