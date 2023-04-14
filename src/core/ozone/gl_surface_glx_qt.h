// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GL_SURFACE_GLX_QT_H_
#define GL_SURFACE_GLX_QT_H_

#include "gl_surface_qt.h"

namespace gl {

class GLSurfaceGLXQt: public GLSurfaceQt {
public:
    explicit GLSurfaceGLXQt(const gfx::Size& size);

    static gl::GLDisplay *InitializeOneOff(gl::GpuPreference preference);
    static bool InitializeExtensionSettingsOneOff();

    bool Initialize(GLSurfaceFormat format) override;
    void Destroy() override;
    void* GetHandle() override;

protected:
    ~GLSurfaceGLXQt();

private:
    static bool s_initialized;
    int m_surfaceBuffer;
};

}
#endif
