// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#if defined(USE_OZONE)
#include "gl_context_qt.h"
#include "gl_ozone_angle_qt.h"
#include "gl_surface_egl_qt.h"

#include "ui/base/ozone_buildflags.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_display.h"
#include "ui/gl/gl_surface.h"
#include "ui/gl/gl_utils.h"
#include "ui/ozone/common/native_pixmap_egl_binding.h"

#if BUILDFLAG(IS_OZONE_X11)
#include "ui/gl/gl_glx_api_implementation.h"
#endif

extern "C" {
typedef void (*__eglMustCastToProperFunctionPointerType)(void);
extern __eglMustCastToProperFunctionPointerType EGL_GetProcAddress(const char *procname);
}

namespace ui {

bool GLOzoneANGLEQt::LoadGLES2Bindings(const gl::GLImplementationParts & /*implementation*/)
{
    gl::SetGLGetProcAddressProc(&EGL_GetProcAddress);
    return true;
}

bool GLOzoneANGLEQt::InitializeStaticGLBindings(const gl::GLImplementationParts &implementation)
{
    bool res = GLOzoneEGL::InitializeStaticGLBindings(implementation);

#if BUILDFLAG(IS_OZONE_X11)
    if (GLContextHelper::getGlxPlatformInterface()) {
        gl::SetGLGetProcAddressProc(reinterpret_cast<gl::GLGetProcAddressProc>(
                GLContextHelper::getGlXGetProcAddress()));
        gl::InitializeStaticGLBindingsGLX();
        gl::SetGLGetProcAddressProc(&EGL_GetProcAddress);
    }
#endif

    return res;
}

bool GLOzoneANGLEQt::InitializeExtensionSettingsOneOffPlatform(gl::GLDisplay *display)
{
    bool res = GLOzoneEGL::InitializeExtensionSettingsOneOffPlatform(
            static_cast<gl::GLDisplayEGL *>(display));

#if BUILDFLAG(IS_OZONE_X11)
    if (GLContextHelper::getGlxPlatformInterface()) {
        gl::SetGLGetProcAddressProc(reinterpret_cast<gl::GLGetProcAddressProc>(
                GLContextHelper::getGlXGetProcAddress()));
        std::string extensions =
                glXQueryExtensionsString((struct _XDisplay *)GLContextHelper::getXDisplay(), 0);
        gl::g_driver_glx.InitializeExtensionBindings(extensions.c_str());
        gl::SetGLGetProcAddressProc(&EGL_GetProcAddress);
    }
#endif

    return res;
}

scoped_refptr<gl::GLSurface> GLOzoneANGLEQt::CreateViewGLSurface(gl::GLDisplay *display,
                                                                 gfx::AcceleratedWidget window)
{
    Q_UNUSED(display);
    Q_UNUSED(window);
    return nullptr;
}

// based on GLOzoneEGLX11::CreateOffscreenGLSurface() (x11_surface_factory.cc)
scoped_refptr<gl::GLSurface> GLOzoneANGLEQt::CreateOffscreenGLSurface(gl::GLDisplay *display,
                                                                      const gfx::Size &size)
{
    gl::GLDisplayEGL *eglDisplay = display->GetAs<gl::GLDisplayEGL>();

    if (eglDisplay->IsEGLSurfacelessContextSupported() && size.width() == 0 && size.height() == 0)
        return InitializeGLSurface(new gl::SurfacelessEGL(eglDisplay, size));

    return InitializeGLSurface(new gl::PbufferGLSurfaceEGL(eglDisplay, size));
}

gl::EGLDisplayPlatform GLOzoneANGLEQt::GetNativeDisplay()
{
#if BUILDFLAG(IS_OZONE_X11)
    void *xdisplay = GLContextHelper::getXDisplay();
    if (xdisplay)
        return gl::EGLDisplayPlatform(reinterpret_cast<EGLNativeDisplayType>(xdisplay));
#endif

    if (gl::g_driver_egl.client_ext.b_EGL_MESA_platform_surfaceless)
        return gl::EGLDisplayPlatform(EGL_DEFAULT_DISPLAY, EGL_PLATFORM_SURFACELESS_MESA);

    return gl::EGLDisplayPlatform(EGL_DEFAULT_DISPLAY);
}

bool GLOzoneANGLEQt::CanImportNativePixmap()
{
    return gl::GLSurfaceEGL::GetGLDisplayEGL()->ext->b_EGL_EXT_image_dma_buf_import;
}

std::unique_ptr<NativePixmapGLBinding>
GLOzoneANGLEQt::ImportNativePixmap(scoped_refptr<gfx::NativePixmap> pixmap,
                                   gfx::BufferFormat plane_format, gfx::BufferPlane plane,
                                   gfx::Size plane_size, const gfx::ColorSpace &color_space,
                                   GLenum target, GLuint texture_id)
{
    return NativePixmapEGLBinding::Create(pixmap, plane_format, plane, plane_size, color_space,
                                          target, texture_id);
}

} // namespace ui

#endif // defined(USE_OZONE)
