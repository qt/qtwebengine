// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QGuiApplication>
#include "gl_ozone_glx_qt.h"
#include "gl_surface_glx_qt.h"
#include "gl_context_qt.h"

#include "media/gpu/buildflags.h"
#include "ui/gl/gl_context_glx.h"
#include "ui/gl/gl_gl_api_implementation.h"
#include "ui/gl/gl_glx_api_implementation.h"
#include "ui/gl/presenter.h"
#include "ui/ozone/platform/x11/native_pixmap_glx_binding.h"

#include <dlfcn.h>

namespace ui {

gl::GLDisplay *GLOzoneGLXQt::InitializeGLOneOffPlatform(bool, std::vector<gl::DisplayType>, gl::GpuPreference preference)
{
    return gl::GLSurfaceGLXQt::InitializeOneOff(preference);
}

bool GLOzoneGLXQt::InitializeStaticGLBindings(
        const gl::GLImplementationParts &implementation) {
    Q_UNUSED(implementation);

    base::NativeLibrary library = dlopen(NULL, RTLD_LAZY);
    if (!library) {
        LOG(ERROR) << "Failed to open GL context " << dlerror();
        return false;
    }

    gl::GLGetProcAddressProc get_proc_address =
            reinterpret_cast<gl::GLGetProcAddressProc>(
                base::GetFunctionPointerFromNativeLibrary(library,
                                                          "glXGetProcAddress"));
    if (!get_proc_address) {
    // glx handle not loaded, fallback to qpa
        QFunctionPointer address = GLContextHelper::getGlXGetProcAddress();
        get_proc_address = reinterpret_cast<gl::GLGetProcAddressProc>(address);
    }

    if (!get_proc_address) {
        LOG(ERROR) << "glxGetProcAddress not found.";
        base::UnloadNativeLibrary(library);
        return false;
    }

    gl::SetGLGetProcAddressProc(get_proc_address);
    gl::AddGLNativeLibrary(library);
    gl::SetGLImplementation(gl::kGLImplementationDesktopGL);

    gl::InitializeStaticGLBindingsGL();
    gl::InitializeStaticGLBindingsGLX();

    return true;
}

void GLOzoneGLXQt::SetDisabledExtensionsPlatform(
        const std::string& disabled_extensions) {
    gl::SetDisabledExtensionsGLX(disabled_extensions);
}

void GLOzoneGLXQt::ShutdownGL(gl::GLDisplay *) {
    gl::ClearBindingsGL();
    gl::ClearBindingsGLX();
}

bool GLOzoneGLXQt::GetGLWindowSystemBindingInfo(
        const gl::GLVersionInfo &gl_info,
        gl::GLWindowSystemBindingInfo *info)
{
    return gl::GetGLWindowSystemBindingInfoGLX(gl_info, info);
}

scoped_refptr<gl::GLContext> GLOzoneGLXQt::CreateGLContext(
        gl::GLShareGroup* share_group,
        gl::GLSurface* compatible_surface,
        const gl::GLContextAttribs& attribs) {
    return gl::InitializeGLContext(new gl::GLContextGLX(share_group),
                                   compatible_surface, attribs);
}

scoped_refptr<gl::GLSurface> GLOzoneGLXQt::CreateViewGLSurface(
        gl::GLDisplay* display,
        gfx::AcceleratedWidget window) {
    return nullptr;
}

scoped_refptr<gl::Presenter> GLOzoneGLXQt::CreateSurfacelessViewGLSurface(
        gl::GLDisplay* display,
        gfx::AcceleratedWidget window) {
    return nullptr;
}

scoped_refptr<gl::GLSurface> GLOzoneGLXQt::CreateOffscreenGLSurface(
        gl::GLDisplay* display,
        const gfx::Size& size) {
    scoped_refptr<gl::GLSurface> surface = new gl::GLSurfaceGLXQt(size);
    if (surface->Initialize(gl::GLSurfaceFormat()))
        return surface;
    LOG(WARNING) << "Failed to create offscreen GL surface";
    return nullptr;
}

bool GLOzoneGLXQt::CanImportNativePixmap()
{
    return false;
}

std::unique_ptr<ui::NativePixmapGLBinding> GLOzoneGLXQt::ImportNativePixmap(
        scoped_refptr<gfx::NativePixmap> pixmap, gfx::BufferFormat plane_format, gfx::BufferPlane plane,
        gfx::Size plane_size, const gfx::ColorSpace &, GLenum target, GLuint texture_id)
{
#if BUILDFLAG(USE_VAAPI_X11)
    return NativePixmapGLXBinding::Create(pixmap, plane_format, plane, plane_size,
                                          target, texture_id);
#else
    return nullptr;
#endif
}

bool GLOzoneGLXQt::InitializeExtensionSettingsOneOffPlatform(gl::GLDisplay *)
{
    return gl::GLSurfaceGLXQt::InitializeExtensionSettingsOneOff();
}

}  // namespace ui
