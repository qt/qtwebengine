// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef UI_OZONE_GLX_QT_H_
#define UI_OZONE_GLX_QT_H_

#include "ui/gl/gl_implementation.h"
#include "ui/ozone/public/gl_ozone.h"

namespace ui {

class GLOzoneGLXQt : public GLOzone {

public:
    GLOzoneGLXQt() {}
    ~GLOzoneGLXQt() override {}

    gl::GLDisplay *InitializeGLOneOffPlatform(bool, std::vector<gl::DisplayType>, gl::GpuPreference) override;
    bool InitializeStaticGLBindings(const gl::GLImplementationParts &implementation) override;
    bool InitializeExtensionSettingsOneOffPlatform(gl::GLDisplay *display) override;
    void ShutdownGL(gl::GLDisplay *display) override;
    void SetDisabledExtensionsPlatform(
        const std::string& disabled_extensions) override;
    bool GetGLWindowSystemBindingInfo(
        const gl::GLVersionInfo &gl_info,
        gl::GLWindowSystemBindingInfo *info) override;

    bool CanImportNativePixmap() override;
    std::unique_ptr<ui::NativePixmapGLBinding> ImportNativePixmap(
            scoped_refptr<gfx::NativePixmap>, gfx::BufferFormat, gfx::BufferPlane,
            gfx::Size, const gfx::ColorSpace &, GLenum, GLuint) override;

    scoped_refptr<gl::GLContext> CreateGLContext(
            gl::GLShareGroup* share_group,
            gl::GLSurface* compatible_surface,
            const gl::GLContextAttribs& attribs) override;

    scoped_refptr<gl::GLSurface> CreateViewGLSurface(
            gl::GLDisplay* display,
            gfx::AcceleratedWidget window) override;

    scoped_refptr<gl::Presenter> CreateSurfacelessViewGLSurface(
            gl::GLDisplay* display,
            gfx::AcceleratedWidget window) override;

    scoped_refptr<gl::GLSurface> CreateOffscreenGLSurface(
            gl::GLDisplay* display,
            const gfx::Size& size) override;
};

}  // namespace ui

#endif  // UI_OZONE_GLX_QT_H_
