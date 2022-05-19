// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "gl_surface_wgl_qt.h"

#if BUILDFLAG(IS_WIN)
#include "ui/gl/gl_surface_wgl.h"

namespace gl {

GLSurfaceWGLQt::GLSurfaceWGLQt(const gfx::Size& size)
    : GLSurfaceQt(size),
      m_surfaceBuffer(nullptr)
{
}

GLSurfaceWGLQt::~GLSurfaceWGLQt()
{
    Destroy();
}

bool GLSurfaceWGLQt::InitializeOneOff()
{
    return GLSurfaceWGL::InitializeOneOff();
}

bool GLSurfaceWGLQt::Initialize(GLSurfaceFormat format)
{
    m_surfaceBuffer = new PbufferGLSurfaceWGL(m_size);
    m_format = format;
    return m_surfaceBuffer->Initialize(format);
}

void GLSurfaceWGLQt::Destroy()
{
    m_surfaceBuffer = nullptr;
}

void *GLSurfaceWGLQt::GetHandle()
{
    return m_surfaceBuffer->GetHandle();
}

GLDisplay *GLSurfaceWGLQt::GetGLDisplay()
{
    return m_surfaceBuffer->GetGLDisplay();
}

void *GLSurfaceWGLQt::GetConfig()
{
    return m_surfaceBuffer->GetConfig();
}

} //namespace gl

#endif // BUILDFLAG(IS_WIN)
