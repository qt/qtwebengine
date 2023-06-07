// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GL_OZONE_EGL_QT
#define GL_OZONE_EGL_QT

#if defined(USE_OZONE)

#include "ui/ozone/common/gl_ozone_egl.h"

namespace ui {

class GLOzoneEGLQt : public GLOzoneEGL {
public:
    gl::GLDisplay *InitializeGLOneOffPlatform(uint64_t system_device_id) override;
    bool InitializeExtensionSettingsOneOffPlatform(gl::GLDisplay *display) override;
    scoped_refptr<gl::GLSurface> CreateViewGLSurface(
            gl::GLDisplay *display,
            gfx::AcceleratedWidget window) override;
    scoped_refptr<gl::GLSurface> CreateOffscreenGLSurface(
            gl::GLDisplay *display,
            const gfx::Size &size) override;

protected:
    // Returns native platform display handle. This is used to obtain the EGL
    // display connection for the native display.
    gl::EGLDisplayPlatform GetNativeDisplay() override;

    // Sets up GL bindings for the native surface.
    bool LoadGLES2Bindings(const gl::GLImplementationParts &implementation) override;
};

} // namespace ui

#endif // defined(USE_OZONE)

#endif // GL_OZONE_EGL_QT
