// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "surface_factory_qt.h"

#include "compositor/compositor.h"
#include "ozone/gbm_buffer_factory.h"
#include "ozone/gl_ozone_qt.h"
#include "ozone/ozone_util_qt.h"
#include "qtwebenginecoreglobal_p.h"

#include "media/gpu/buildflags.h"
#include "ui/base/ozone_buildflags.h"
#include "ui/gfx/buffer_format_util.h"
#include "ui/gfx/linux/gbm_buffer.h"
#include "ui/gfx/linux/native_pixmap_dmabuf.h"

#include <QDebug>
#include <QtGui/qtgui-config.h>

#if QT_CONFIG(opengl) && BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
#include "ozone/glx_helper.h"
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
    m_impls.push_back({ gl::GLImplementationParts(gl::kGLImplementationEGLGLES2),
                        std::make_unique<ui::GLOzoneEGLQt>() });
#endif
    m_impls.push_back({ gl::GLImplementationParts(gl::kGLImplementationStubGL), nullptr });
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

    gfx::NativePixmapHandle bufferHandle;

#if BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
    if (OzoneUtilQt::usingGLX()) {
        if (auto *gbm = GLXHelper::instance()->gbmFactory()) {
            const auto &modifiers = GLXHelper::instance()->getSupportedModifiers();
            if (auto gbmBuffer = gbm->createBufferWithModifiers(format, size, usage, modifiers))
                bufferHandle = gbmBuffer->ExportHandle();
            else
                qWarning("Failed to create GBM buffer for GLX.");
        }
    }
#endif

#if QT_CONFIG(egl)
    if (OzoneUtilQt::usingEGL()) {
        // Multi-planar formats are only supported when creating NativePixmap from an existing
        // handle (eg. for hardware video decoding), which is handled by
        // CreateNativePixmapFromHandle().
        if (gfx::BufferFormatIsMultiplanar(format)) {
            qFatal("Direct allocation of multi-planar GBM buffer (format: %s) via EGL is "
                   "currently unsupported.",
                   gfx::BufferFormatToString(format));
        }

        if (auto *gbm = EGLHelper::instance()->gbmFactory()) {
            const auto &modifiers = EGLHelper::instance()->getSupportedModifiers(format);
            if (auto gbmBuffer = gbm->createBufferWithModifiers(format, size, usage, modifiers))
                bufferHandle = gbmBuffer->ExportHandle();
            else
                qWarning("Failed to create GBM buffer for EGL.");
        }

        if (bufferHandle.planes.empty()) {
            qCDebug(QtWebEngineCore::lcWebEngineCompositor,
                    "Fallback to EGL-based buffer allocation.");
            bufferHandle = EGLHelper::instance()->exportHandleFromEGLImage(size);
        }
    }
#endif

    if (bufferHandle.planes.empty()) {
        // TODO(QTBUG-120761): Try to fallback to software rendering instead of aborting.
        qFatal("Failed to create GBM buffer. NativePixmap allocation failed.");
        return nullptr;
    }

    return base::MakeRefCounted<gfx::NativePixmapDmaBuf>(size, format, std::move(bufferHandle));
#else
    return nullptr;
#endif // QT_CONFIG(opengl)
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
        if (auto *gbm = GLXHelper::instance()->gbmFactory()) {
            if (auto gbmBuffer = gbm->createBufferFromHandle(format, size, std::move(handle)))
                bufferHandle = gbmBuffer->ExportHandle();
            else
                qWarning("Failed to create GBM buffer for GLX.");
        }
    }
#endif

#if QT_CONFIG(egl)
    if (OzoneUtilQt::usingEGL()) {
        if (auto *gbm = EGLHelper::instance()->gbmFactory()) {
            // Keep the original handle valid for potential fallback.
            gfx::NativePixmapHandle clonedHandle = gfx::CloneHandleForIPC(handle);
            if (auto gbmBuffer =
                        gbm->createBufferFromHandle(format, size, std::move(clonedHandle))) {
                bufferHandle = gbmBuffer->ExportHandle();
            } else {
                qWarning("Failed to create GBM buffer for EGL.");
            }
        }

        if (bufferHandle.planes.empty()) {
            qCDebug(QtWebEngineCore::lcWebEngineCompositor,
                    "Fallback to EGL-based buffer allocation.");
            bufferHandle = EGLHelper::instance()->exportHandleFromEGLImage(format, size,
                                                                           std::move(handle));
        }
    }
#endif // QT_CONFIG(egl)

    if (bufferHandle.planes.empty()) {
        // TODO(QTBUG-120761): Try to fallback to software rendering instead of aborting.
        qFatal("Failed to create GBM buffer. NativePixmap allocation failed.");
        return nullptr;
    }

    return base::MakeRefCounted<gfx::NativePixmapDmaBuf>(size, format, std::move(bufferHandle));
#else
    return nullptr;
#endif // QT_CONFIG(opengl)
}

bool SurfaceFactoryQt::CanCreateNativePixmapForFormat(gfx::BufferFormat format)
{
#if QT_CONFIG(opengl)
#if BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
    if (OzoneUtilQt::usingGLX())
        return GLXHelper::instance()->canCreateNativePixmapForFormat(format);
#endif

#if QT_CONFIG(egl)
    if (OzoneUtilQt::usingEGL())
        return EGLHelper::instance()->canCreateNativePixmapForFormat(format);
#endif
#endif // QT_CONFIG(opengl)

    return false;
}

// static
bool SurfaceFactoryQt::SupportsNativePixmaps()
{
#if QT_CONFIG(opengl)
#if BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
    if (OzoneUtilQt::usingGLX())
        return GLXHelper::instance()->isDmaBufSupported();
#endif

#if QT_CONFIG(egl)
    if (OzoneUtilQt::usingEGL())
        return EGLHelper::instance()->isDmaBufSupported();
#endif
#endif // QT_CONFIG(opengl)

    return false;
}

} // namespace QtWebEngineCore
