// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:execute-external-code

// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gl_ozone_qt.h"
#include "surface_factory_qt.h"
#include "ozone_util_qt.h"

#include <QtCore/private/qconfig_p.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qopenglcontext.h>

#include "ui/base/ozone_buildflags.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_display.h"
#include "ui/gl/gl_surface.h"
#include "ui/gl/gl_utils.h"
#include "ui/ozone/common/native_pixmap_egl_binding.h"

#if BUILDFLAG(IS_OZONE_X11)
#include "ozone_util_qt.h"

#include "ui/ozone/platform/x11/native_pixmap_egl_x11_binding.h"
#endif

#if QT_CONFIG(dlopen)
#include <dlfcn.h>
#endif

using namespace Qt::StringLiterals;

extern "C" {
typedef void (*__eglMustCastToProperFunctionPointerType)(void);
extern __eglMustCastToProperFunctionPointerType EGL_GetProcAddress(const char *procname);
}

namespace ui {

NativePixmapSupportType GLOzoneQt::getNativePixmapSupportType()
{
    if (!QtWebEngineCore::SurfaceFactoryQt::SupportsNativePixmaps())
        return NativePixmapSupportType::kNone;

    if (gl::GLSurfaceEGL::GetGLDisplayEGL()->ext->b_EGL_EXT_image_dma_buf_import)
        return NativePixmapSupportType::kDMABuf;

#if BUILDFLAG(IS_OZONE_X11)
    if (NativePixmapEGLX11Binding::CanImportNativeGLXPixmap())
        return NativePixmapSupportType::kX11Pixmap;
#endif

    return NativePixmapSupportType::kNone;
}

bool GLOzoneQt::LoadGLES2Bindings(const gl::GLImplementationParts & /*implementation*/)
{
    return false;
}

bool GLOzoneQt::InitializeStaticGLBindings(const gl::GLImplementationParts &implementation)
{
    return GLOzoneEGL::InitializeStaticGLBindings(implementation);
}

bool GLOzoneQt::InitializeExtensionSettingsOneOffPlatform(gl::GLDisplay *display)
{
    return GLOzoneEGL::InitializeExtensionSettingsOneOffPlatform(
            static_cast<gl::GLDisplayEGL *>(display));
}

scoped_refptr<gl::GLSurface> GLOzoneQt::CreateViewGLSurface(gl::GLDisplay * /*display*/,
                                                            gfx::AcceleratedWidget /*window*/)
{
    return nullptr;
}

// based on GLOzoneEGLX11::CreateOffscreenGLSurface() (x11_surface_factory.cc)
scoped_refptr<gl::GLSurface> GLOzoneQt::CreateOffscreenGLSurface(gl::GLDisplay *display,
                                                                 const gfx::Size &size)
{
    gl::GLDisplayEGL *eglDisplay = display->GetAs<gl::GLDisplayEGL>();

    if (eglDisplay->IsEGLSurfacelessContextSupported() && size.width() == 0 && size.height() == 0)
        return InitializeGLSurface(new gl::SurfacelessEGL(eglDisplay, size));

    return InitializeGLSurface(new gl::PbufferGLSurfaceEGL(eglDisplay, size));
}

gl::EGLDisplayPlatform GLOzoneQt::GetNativeDisplay()
{
#if BUILDFLAG(IS_OZONE_X11)
    static EGLNativeDisplayType nativeDisplay =
            reinterpret_cast<EGLNativeDisplayType>(OzoneUtilQt::getXDisplay());
    if (nativeDisplay)
        return gl::EGLDisplayPlatform(nativeDisplay);
#endif

    if (gl::g_driver_egl.client_ext.b_EGL_MESA_platform_surfaceless)
        return gl::EGLDisplayPlatform(EGL_DEFAULT_DISPLAY, EGL_PLATFORM_SURFACELESS_MESA);

    return gl::EGLDisplayPlatform(EGL_DEFAULT_DISPLAY);
}

bool GLOzoneQt::CanImportNativePixmap(gfx::BufferFormat format)
{
    switch (getNativePixmapSupportType()) {
    case NativePixmapSupportType::kDMABuf:
        return NativePixmapEGLBinding::IsBufferFormatSupported(format);
#if BUILDFLAG(IS_OZONE_X11)
    case NativePixmapSupportType::kX11Pixmap:
        return NativePixmapEGLX11Binding::IsBufferFormatSupported(format);
#endif
    default:
        return false;
    }
}

std::unique_ptr<NativePixmapGLBinding>
GLOzoneQt::ImportNativePixmap(scoped_refptr<gfx::NativePixmap> pixmap,
                              gfx::BufferFormat plane_format, gfx::BufferPlane plane,
                              gfx::Size plane_size, const gfx::ColorSpace &color_space,
                              GLenum target, GLuint texture_id)
{
    switch (getNativePixmapSupportType()) {
    case NativePixmapSupportType::kDMABuf:
        return NativePixmapEGLBinding::Create(pixmap, plane_format, plane, plane_size, color_space,
                                              target, texture_id);
#if BUILDFLAG(IS_OZONE_X11)
    case NativePixmapSupportType::kX11Pixmap:
        return NativePixmapEGLX11Binding::Create(pixmap, plane_format, plane_size, target,
                                                 texture_id);
#endif
    default:
        NOTREACHED();
        return nullptr;
    }
}

bool GLOzoneANGLEQt::LoadGLES2Bindings(const gl::GLImplementationParts & /*implementation*/)
{
    if (QGuiApplication::platformName() == "wayland"_L1) {
        const char eglPlatformVar[] = "EGL_PLATFORM";
        const QByteArray eglPlatform = qgetenv(eglPlatformVar);
        if (eglPlatform.isEmpty())
            qputenv(eglPlatformVar, "wayland");
        else if (eglPlatform != "wayland") {
            qWarning("EGL_PLATFORM environment variable is set to \"%s\". "
                     "This may break hardware rendering on Wayland.",
                     eglPlatform.constData());
        }
    }

    gl::SetGLGetProcAddressProc(&EGL_GetProcAddress);
    return true;
}

void GLOzoneEGLQt::ShutdownGL(gl::GLDisplay *display)
{
    GLOzoneEGL::ShutdownGL(display);
#if QT_CONFIG(dlopen)
    if (m_nativeEGLHandle)
        dlclose(m_nativeEGLHandle);
#endif
}

bool GLOzoneEGLQt::LoadGLES2Bindings(const gl::GLImplementationParts & /*implementation*/)
{
    gl::GLGetProcAddressProc getProcAddressPtr = nullptr;

#if QT_CONFIG(opengl) && QT_CONFIG(egl)
    if (OzoneUtilQt::usingEGL()) {
        QOpenGLContext *context = OzoneUtilQt::getQOpenGLContext();
        getProcAddressPtr = reinterpret_cast<gl::GLGetProcAddressProc>(
                context->getProcAddress("eglGetProcAddress"));
    }
#endif

#if QT_CONFIG(dlopen)
    if (getProcAddressPtr == nullptr) {
        const char *eglPath = "libEGL.so.1";
        m_nativeEGLHandle = dlopen(eglPath, RTLD_NOW);
        if (!m_nativeEGLHandle) {
            qWarning("Failed to load EGL library %s: %s", eglPath, dlerror());
            return false;
        }

        getProcAddressPtr = reinterpret_cast<gl::GLGetProcAddressProc>(
                dlsym(m_nativeEGLHandle, "eglGetProcAddress"));
    }
#endif // QT_CONFIG(dlopen)

    if (!getProcAddressPtr) {
        char *error = nullptr;
#if QT_CONFIG(dlopen)
        error = dlerror();
#endif
        qWarning("Failed to get address of eglGetProcAddress: %s", error ? error : "no error.");
        return false;
    }

    gl::SetGLGetProcAddressProc(getProcAddressPtr);
    // TODO: Log EGL driver information.
    //       Nvidia fails to make EGL context current if libEGL.so.1 is loaded directly. This could
    //       be because of loading the wrong driver.
    return true;
}

} // namespace ui
