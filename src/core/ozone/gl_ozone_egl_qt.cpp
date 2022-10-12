// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#if defined(USE_OZONE)
#include "gl_context_qt.h"
#include "gl_ozone_egl_qt.h"
#include "gl_surface_egl_qt.h"

#include "base/files/file_path.h"
#include "base/native_library.h"
#include "ui/gl/gl_context_egl.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface.h"
#include "ui/gl/init/gl_factory.h"
#include "ui/gl/init/gl_initializer.h"

#include <EGL/egl.h>
#include <dlfcn.h>

namespace ui {

bool GLOzoneEGLQt::LoadGLES2Bindings(const gl::GLImplementationParts & /*implementation*/)
{
    base::NativeLibrary eglgles2Library = dlopen(NULL, RTLD_LAZY);
    if (!eglgles2Library) {
        LOG(ERROR) << "Failed to open EGL/GLES2 context " << dlerror();
        return false;
    }

    gl::GLGetProcAddressProc get_proc_address =
            reinterpret_cast<gl::GLGetProcAddressProc>(
                base::GetFunctionPointerFromNativeLibrary(eglgles2Library,
                                                          "eglGetProcAddress"));
    if (!get_proc_address) {
        // QTBUG-63341 most likely libgles2 not linked with libegl -> fallback to qpa
        get_proc_address =
                reinterpret_cast<gl::GLGetProcAddressProc>(GLContextHelper::getEglGetProcAddress());
    }

    if (!get_proc_address) {
        LOG(ERROR) << "eglGetProcAddress not found.";
        base::UnloadNativeLibrary(eglgles2Library);
        return false;
    }

    gl::SetGLGetProcAddressProc(get_proc_address);
    gl::AddGLNativeLibrary(eglgles2Library);
    return true;
}

gl::GLDisplay *GLOzoneEGLQt::InitializeGLOneOffPlatform(uint64_t system_device_id)
{
    return gl::GLSurfaceEGLQt::InitializeOneOff(system_device_id);
}

bool GLOzoneEGLQt::InitializeExtensionSettingsOneOffPlatform(gl::GLDisplay *display)
{
    Q_UNUSED(display);
    return gl::GLSurfaceEGLQt::InitializeExtensionSettingsOneOff();
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

} // namespace ui

#endif // defined(USE_OZONE)
