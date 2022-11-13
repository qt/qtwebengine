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

    bool InitializeGLOneOffPlatform() override;
    bool InitializeStaticGLBindings(const gl::GLImplementationParts &implementation) override;
    bool InitializeExtensionSettingsOneOffPlatform() override;
    void ShutdownGL() override;
    void SetDisabledExtensionsPlatform(
        const std::string& disabled_extensions) override;
    bool GetGLWindowSystemBindingInfo(
        const gl::GLVersionInfo &gl_info,
        gl::GLWindowSystemBindingInfo *info) override;

    scoped_refptr<gl::GLContext> CreateGLContext(
            gl::GLShareGroup* share_group,
            gl::GLSurface* compatible_surface,
            const gl::GLContextAttribs& attribs) override;

    scoped_refptr<gl::GLSurface> CreateViewGLSurface(
            gfx::AcceleratedWidget window) override;

    scoped_refptr<gl::GLSurface> CreateSurfacelessViewGLSurface(
            gfx::AcceleratedWidget window) override;

    scoped_refptr<gl::GLSurface> CreateOffscreenGLSurface(
            const gfx::Size& size) override;
};

}  // namespace ui

#endif  // UI_OZONE_GLX_QT_H_
