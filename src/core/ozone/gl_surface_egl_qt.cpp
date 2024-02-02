// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gl_context_qt.h"
#include "ozone/gl_surface_egl_qt.h"

#include "ui/gl/egl_util.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_display.h"
#include "ui/gl/gl_display_manager.h"
#include "ui/gl/init/gl_factory.h"

#if !BUILDFLAG(IS_MAC) && !BUILDFLAG(IS_WIN)

using ui::GetLastEGLErrorString;

namespace gl {

bool GLSurfaceEGLQt::g_egl_surfaceless_context_supported = false;
bool GLSurfaceEGLQt::s_initialized = false;

GLSurfaceEGLQt::GLSurfaceEGLQt(gl::GLDisplayEGL *display, const gfx::Size& size)
    : GLSurfaceQt(size),
      m_surfaceBuffer(0)
{
}

GLSurfaceEGLQt::~GLSurfaceEGLQt()
{
    Destroy();
}

gl::GLDisplay *GLSurfaceEGLQt::InitializeOneOff(gl::GpuPreference preference)
{
    if (s_initialized)
        return g_display;

    auto *egl_display = GLDisplayManagerEGL::GetInstance()->GetDisplay(preference);
    g_display = egl_display;
    egl_display->SetDisplay(GLContextHelper::getEGLDisplay());
    if (!egl_display->GetDisplay()) {
        LOG(ERROR) << "GLContextHelper::getEGLDisplay() failed.";
        return nullptr;
    }

    g_config = GLContextHelper::getEGLConfig();
    if (!g_config) {
        LOG(ERROR) << "GLContextHelper::getEGLConfig() failed.";
        return nullptr;
    }

    if (!eglInitialize(egl_display->GetDisplay(), NULL, NULL)) {
        LOG(ERROR) << "eglInitialize failed with error " << GetLastEGLErrorString();
        return nullptr;
    }

    g_extensions = eglQueryString(egl_display->GetDisplay(), EGL_EXTENSIONS);
    g_egl_surfaceless_context_supported = ExtensionsContain(g_extensions.c_str(), "EGL_KHR_surfaceless_context");
    if (g_egl_surfaceless_context_supported) {
        scoped_refptr<GLSurface> surface = new GLSurfacelessQtEGL(egl_display, gfx::Size(1, 1));
        gl::GLContextAttribs attribs;
        scoped_refptr<GLContext> context = init::CreateGLContext(
            NULL, surface.get(), attribs);

        if (!context->MakeCurrent(surface.get()))
            g_egl_surfaceless_context_supported = false;

        // Ensure context supports GL_OES_surfaceless_context.
        if (g_egl_surfaceless_context_supported) {
            g_egl_surfaceless_context_supported = context->HasExtension(
                "GL_OES_surfaceless_context");
            context->ReleaseCurrent(surface.get());
        }
    }

    s_initialized = true;
    return egl_display;
}

bool GLSurfaceEGLQt::InitializeExtensionSettingsOneOff()
{
    return s_initialized;
}

bool GLSurfaceEGLQt::Initialize(GLSurfaceFormat format)
{
    Q_ASSERT(!m_surfaceBuffer);
    m_format = format;

    EGLDisplay display = GLContextHelper::getEGLDisplay();
    if (!display) {
        LOG(ERROR) << "Trying to create surface with invalid display.";
        return false;
    }

    const EGLint pbuffer_attributes[] = {
        EGL_WIDTH, m_size.width(),
        EGL_HEIGHT, m_size.height(),
        EGL_LARGEST_PBUFFER, EGL_FALSE,
        EGL_NONE
    };

    m_surfaceBuffer = eglCreatePbufferSurface(display,
                                        g_config,
                                        pbuffer_attributes);
    if (!m_surfaceBuffer) {
        VLOG(1) << "eglCreatePbufferSurface failed with error " << GetLastEGLErrorString();
        Destroy();
        return false;
    }

    return true;
}

void GLSurfaceEGLQt::Destroy()
{
    if (m_surfaceBuffer) {
        if (!eglDestroySurface(GLContextHelper::getEGLDisplay(), m_surfaceBuffer))
            LOG(ERROR) << "eglDestroySurface failed with error " << GetLastEGLErrorString();

        m_surfaceBuffer = 0;
    }
}

bool GLSurfaceEGLQt::Resize(const gfx::Size& size, float scale_factor,
                            const gfx::ColorSpace &color_space, bool has_alpha)
{
    if (size == m_size)
        return true;

    GLContext *currentContext = GLContext::GetCurrent();
    bool wasCurrent = currentContext && currentContext->IsCurrent(this);
    if (wasCurrent)
        currentContext->ReleaseCurrent(this);

    Destroy();

    m_size = size;

    if (!Initialize(GetFormat())) {
        LOG(ERROR) << "Failed to resize pbuffer.";
        return false;
    }

    if (wasCurrent)
        return currentContext->MakeCurrent(this);

    return true;
}

void* GLSurfaceEGLQt::GetHandle()
{
    return reinterpret_cast<void*>(m_surfaceBuffer);
}

GLSurfacelessQtEGL::GLSurfacelessQtEGL(GLDisplayEGL *display, const gfx::Size& size)
    : GLSurfaceQt(size)
{
}

bool GLSurfacelessQtEGL::Initialize(GLSurfaceFormat format)
{
    m_format = format;
    return true;
}

void GLSurfacelessQtEGL::Destroy()
{
}

bool GLSurfacelessQtEGL::IsSurfaceless() const
{
    return true;
}

bool GLSurfacelessQtEGL::Resize(const gfx::Size& size, float scale_factor,
                                const gfx::ColorSpace &color_space,  bool has_alpha)
{
    m_size = size;
    return true;
}

EGLSurface GLSurfacelessQtEGL::GetHandle()
{
    return EGL_NO_SURFACE;
}

void* GLSurfacelessQtEGL::GetShareHandle()
{
    return NULL;
}

} // namespace gl
#endif // !BUILDFLAG(IS_MAC) && !BUILDFLAG(IS_WIN)
