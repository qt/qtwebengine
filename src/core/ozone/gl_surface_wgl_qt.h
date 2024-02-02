// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GL_SURFACE_WGL_QT_H
#define GL_SURFACE_WGL_QT_H

#include "gl_surface_qt.h"

#if BUILDFLAG(IS_WIN)

namespace gl {

class PbufferGLSurfaceWGL;

class GLSurfaceWGLQt: public GLSurfaceQt {
public:
    explicit GLSurfaceWGLQt(const gfx::Size& size);

    static gl::GLDisplay *InitializeOneOff(gl::GpuPreference gpu_preference);

    bool Initialize(GLSurfaceFormat format) override;
    void Destroy() override;
    void *GetHandle() override;
    GLDisplay *GetGLDisplay() override;
    void *GetConfig() override;

protected:
    ~GLSurfaceWGLQt();

private:
    scoped_refptr<PbufferGLSurfaceWGL> m_surfaceBuffer;
};

}
#endif // BUILDFLAG(IS_WIN)
#endif // GL_SURFACE_WGL_QT_H

