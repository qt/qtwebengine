// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gl_ozone_angle_qt.h"

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

extern "C" {
typedef void (*__eglMustCastToProperFunctionPointerType)(void);
extern __eglMustCastToProperFunctionPointerType EGL_GetProcAddress(const char *procname);
}

namespace ui {
namespace {
// Based on //ui/ozone/platform/x11/x11_surface_factory.cc
enum class NativePixmapSupportType {
    // Importing native pixmaps not supported.
    kNone,

    // Native pixmaps are imported directly into EGL using the
    // EGL_EXT_image_dma_buf_import extension.
    kDMABuf,

    // Native pixmaps are first imported as X11 pixmaps using DRI3 and then into
    // EGL.
    kX11Pixmap,
};

NativePixmapSupportType GetNativePixmapSupportType()
{
    if (gl::GLSurfaceEGL::GetGLDisplayEGL()->ext->b_EGL_EXT_image_dma_buf_import)
        return NativePixmapSupportType::kDMABuf;

#if BUILDFLAG(IS_OZONE_X11)
    if (NativePixmapEGLX11Binding::CanImportNativeGLXPixmap())
        return NativePixmapSupportType::kX11Pixmap;
#endif

    return NativePixmapSupportType::kNone;
}
} // namespace

bool GLOzoneANGLEQt::LoadGLES2Bindings(const gl::GLImplementationParts & /*implementation*/)
{
    gl::SetGLGetProcAddressProc(&EGL_GetProcAddress);
    return true;
}

bool GLOzoneANGLEQt::InitializeStaticGLBindings(const gl::GLImplementationParts &implementation)
{
    return GLOzoneEGL::InitializeStaticGLBindings(implementation);
}

bool GLOzoneANGLEQt::InitializeExtensionSettingsOneOffPlatform(gl::GLDisplay *display)
{
    return GLOzoneEGL::InitializeExtensionSettingsOneOffPlatform(
            static_cast<gl::GLDisplayEGL *>(display));
}

scoped_refptr<gl::GLSurface> GLOzoneANGLEQt::CreateViewGLSurface(gl::GLDisplay * /*display*/,
                                                                 gfx::AcceleratedWidget /*window*/)
{
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
    static EGLNativeDisplayType nativeDisplay =
            reinterpret_cast<EGLNativeDisplayType>(OzoneUtilQt::getXDisplay());
    if (nativeDisplay)
        return gl::EGLDisplayPlatform(nativeDisplay);
#endif

    if (gl::g_driver_egl.client_ext.b_EGL_MESA_platform_surfaceless)
        return gl::EGLDisplayPlatform(EGL_DEFAULT_DISPLAY, EGL_PLATFORM_SURFACELESS_MESA);

    return gl::EGLDisplayPlatform(EGL_DEFAULT_DISPLAY);
}

bool GLOzoneANGLEQt::CanImportNativePixmap(gfx::BufferFormat format)
{
    switch (GetNativePixmapSupportType()) {
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
GLOzoneANGLEQt::ImportNativePixmap(scoped_refptr<gfx::NativePixmap> pixmap,
                                   gfx::BufferFormat plane_format, gfx::BufferPlane plane,
                                   gfx::Size plane_size, const gfx::ColorSpace &color_space,
                                   GLenum target, GLuint texture_id)
{
    switch (GetNativePixmapSupportType()) {
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

} // namespace ui
