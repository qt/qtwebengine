// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#if defined(USE_OZONE)
#include "gl_context_qt.h"
#include "gl_ozone_egl_qt.h"
#include "gl_surface_egl_qt.h"

#include "base/files/file_path.h"
#include "base/native_library.h"
#include "media/gpu/buildflags.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_context_egl.h"
#include "ui/gl/gl_display.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_egl_api_implementation.h"
#include "ui/gl/gl_surface.h"
#include "ui/gl/gl_utils.h"
#include "ui/gl/init/gl_factory.h"
#include "ui/gl/init/gl_initializer.h"
#include "ui/ozone/common/native_pixmap_egl_binding.h"

#include <EGL/egl.h>
#include <dlfcn.h>

namespace ui {

bool LoadQtEGLBindings()
{
    gl::GLGetProcAddressProc get_proc_address = reinterpret_cast<gl::GLGetProcAddressProc>(GLContextHelper::getEglGetProcAddress());
    if (!get_proc_address) {
        LOG(ERROR) << "eglGetProcAddress not found.";
        return false;
    }
    gl::SetGLGetProcAddressProc(get_proc_address);
    return true;
}

bool GLOzoneEGLQt::LoadGLES2Bindings(const gl::GLImplementationParts & /*implementation*/)
{
    return LoadQtEGLBindings();
}

gl::GLDisplay *GLOzoneEGLQt::InitializeGLOneOffPlatform(uint64_t system_device_id)
{
    if (auto display = gl::GLSurfaceEGLQt::InitializeOneOff(system_device_id)) {
        if (!static_cast<gl::GLDisplayEGL*>(display)->Initialize(GetNativeDisplay())) {
            LOG(ERROR) << "GLDisplayEGL::Initialize failed.";
            return nullptr;
        }
        return display;
    }
    return nullptr;
}

bool GLOzoneEGLQt::InitializeExtensionSettingsOneOffPlatform(gl::GLDisplay *display)
{
    return static_cast<gl::GLDisplayEGL*>(display)->InitializeExtensionSettings();
}

scoped_refptr<gl::GLSurface> GLOzoneEGLQt::CreateViewGLSurface(gl::GLDisplay* display, gfx::AcceleratedWidget window)
{
    Q_UNUSED(display);
    Q_UNUSED(window);
    return nullptr;
}

scoped_refptr<gl::GLSurface> GLOzoneEGLQt::CreateOffscreenGLSurface(gl::GLDisplay* display, const gfx::Size &size)
{
    scoped_refptr<gl::GLSurface> surface = new gl::GLSurfaceEGLQt(static_cast<gl::GLDisplayEGL*>(display), size);
    if (surface->Initialize(gl::GLSurfaceFormat()))
        return surface;

    surface = new gl::GLSurfacelessQtEGL(static_cast<gl::GLDisplayEGL*>(display), size);
    if (surface->Initialize(gl::GLSurfaceFormat()))
        return surface;

    LOG(WARNING) << "Failed to create offscreen GL surface";
    return nullptr;
}

gl::EGLDisplayPlatform GLOzoneEGLQt::GetNativeDisplay()
{
    static void *display = GLContextHelper::getNativeDisplay();
    static gl::EGLDisplayPlatform platform(display ? reinterpret_cast<intptr_t>(display) : EGL_DEFAULT_DISPLAY);
    return platform;
}

bool GLOzoneEGLQt::CanImportNativePixmap()
{
#if BUILDFLAG(USE_VAAPI)
    return gl::GLSurfaceEGL::GetGLDisplayEGL()->ext->b_EGL_EXT_image_dma_buf_import;
#else
    return false;
#endif
}

std::unique_ptr<NativePixmapGLBinding> GLOzoneEGLQt::ImportNativePixmap(
        scoped_refptr<gfx::NativePixmap> pixmap,
        gfx::BufferFormat plane_format,
        gfx::BufferPlane plane,
        gfx::Size plane_size,
        const gfx::ColorSpace &color_space,
        GLenum target,
        GLuint texture_id)
{
#if BUILDFLAG(USE_VAAPI)
    return NativePixmapEGLBinding::Create(pixmap, plane_format, plane,
                                          plane_size, color_space, target,
                                          texture_id);
#else
    return nullptr;
#endif
}

} // namespace ui

#endif // defined(USE_OZONE)
