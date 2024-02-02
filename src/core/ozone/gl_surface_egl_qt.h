// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GL_SURFACE_EGL_QT_H_
#define GL_SURFACE_EGL_QT_H_

#include "gl_surface_qt.h"
#include <EGL/egl.h>

namespace gl {

class GLDisplayEGL;

class GLSurfaceEGLQt: public GLSurfaceQt {
public:
    explicit GLSurfaceEGLQt(gl::GLDisplayEGL *display, const gfx::Size& size);

    static gl::GLDisplay *InitializeOneOff(gl::GpuPreference preference);
    static bool InitializeExtensionSettingsOneOff();

    bool Initialize(GLSurfaceFormat format) override;
    void Destroy() override;
    void* GetHandle() override;
    bool Resize(const gfx::Size& size, float scale_factor,
                const gfx::ColorSpace &color_space, bool has_alpha) override;


protected:
    ~GLSurfaceEGLQt();

public:
   static bool g_egl_surfaceless_context_supported;

private:
   EGLSurface m_surfaceBuffer;
   static bool s_initialized;
};

// The following comment is cited from chromium/ui/gl/gl_surface_egl.cc:
// SurfacelessEGL is used as Offscreen surface when platform supports
// KHR_surfaceless_context and GL_OES_surfaceless_context. This would avoid the
// need to create a dummy EGLsurface in case we render to client API targets.

class GLSurfacelessQtEGL : public GLSurfaceQt {
public:
    explicit GLSurfacelessQtEGL(gl::GLDisplayEGL *display, const gfx::Size& size);

public:
    bool Initialize(GLSurfaceFormat format) override;
    void Destroy() override;
    bool IsSurfaceless() const override;
    bool Resize(const gfx::Size& size, float scale_factor,
                const gfx::ColorSpace &color_space, bool has_alpha) override;
    EGLSurface GetHandle() override;
    void* GetShareHandle() override;
};
}

#endif
