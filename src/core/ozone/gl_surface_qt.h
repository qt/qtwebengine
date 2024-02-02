// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GL_SURFACE_QT_H_
#define GL_SURFACE_QT_H_

#include <string>

#include "ui/gfx/geometry/size.h"
#include "ui/gl/gl_surface.h"

namespace gl {

class GLSurfaceQt: public GLSurface {
public:
    explicit GLSurfaceQt(const gfx::Size& size);

    static bool HasEGLExtension(const char* name);

    // Implement GLSurface.
    GLDisplay *GetGLDisplay() override;
    void *GetConfig() override;
    bool IsOffscreen() override;
    gfx::SwapResult SwapBuffers(PresentationCallback callback, gfx::FrameData data) override;
    gfx::Size GetSize() override;
    GLSurfaceFormat GetFormat() override;

protected:
    GLSurfaceQt();
    virtual ~GLSurfaceQt();

    gfx::Size m_size;
    GLSurfaceFormat m_format;

public:
    static void* g_config;
    static GLDisplay *g_display;
    static std::string g_extensions;
};

} // namespace gl

#endif // GL_SURFACE_QT_H_
