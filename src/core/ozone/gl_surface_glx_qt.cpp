// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gl_context_qt.h"
#include "ozone/gl_surface_glx_qt.h"

#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_display.h"
#include "ui/gl/gl_display_manager.h"
#include "ui/gl/gl_surface_glx.h"

namespace gl {

void GLSurfaceGLX::ShutdownOneOff()
{
}

bool GLSurfaceGLX::IsCreateContextSupported()
{
    return HasGLXExtension("GLX_ARB_create_context");
}

bool GLSurfaceGLX::IsCreateContextRobustnessSupported()
{
    return GLContextHelper::isCreateContextRobustnessSupported() && HasGLXExtension("GLX_ARB_create_context_robustness");
}

bool GLSurfaceGLX::IsEXTSwapControlSupported()
{
    return HasGLXExtension("GLX_EXT_swap_control");
}

bool GLSurfaceGLX::IsMESASwapControlSupported()
{
    return HasGLXExtension("GLX_MESA_swap_control");
}

bool GLSurfaceGLX::IsCreateContextProfileSupported()
{
    return false; // ExtensionsContain(g_extensions, "GLX_ARB_create_context_profile");
}

bool GLSurfaceGLX::IsCreateContextES2ProfileSupported()
{
    return HasGLXExtension("GLX_ARB_create_context_es2_profile");
}

bool GLSurfaceGLX::IsOMLSyncControlSupported()
{
    return false; // ExtensionsContain(g_extensions, "GLX_OML_sync_control");
}

bool GLSurfaceGLX::HasGLXExtension(const char *name)
{
    return ExtensionsContain(GLSurfaceQt::g_extensions.c_str(), name);
}

bool GLSurfaceGLX::IsTextureFromPixmapSupported()
{
    return HasGLXExtension("GLX_EXT_texture_from_pixmap");
}

bool GLSurfaceGLX::IsRobustnessVideoMemoryPurgeSupported()
{
    return false;
}

const char* GLSurfaceGLX::GetGLXExtensions()
{
    return GLSurfaceQt::g_extensions.c_str();
}


bool GLSurfaceGLXQt::s_initialized = false;

GLSurfaceGLXQt::GLSurfaceGLXQt(const gfx::Size& size)
    : GLSurfaceQt(size),
      m_surfaceBuffer(0)
{
}

GLSurfaceGLXQt::~GLSurfaceGLXQt()
{
    Destroy();
}

GLDisplay *GLSurfaceGLXQt::InitializeOneOff(gl::GpuPreference preference)
{
    if (s_initialized)
        return g_display;

    g_display = GLDisplayManagerX11::GetInstance()->GetDisplay(preference);
    if (!g_display->GetDisplay()) {
        LOG(ERROR) << "GLContextHelper::getXDisplay() failed.";
        return nullptr;
    }

    g_config = GLContextHelper::getGlXConfig();
    if (!g_config) {
        LOG(ERROR) << "GLContextHelper::getGlxConfig() failed.";
        return nullptr;
    }

    Display* display = static_cast<Display*>(g_display->GetDisplay());
    int major, minor;
    if (!glXQueryVersion(display, &major, &minor)) {
        LOG(ERROR) << "glxQueryVersion failed.";
        return nullptr;
    }

    if (major == 1 && minor < 3) {
        LOG(ERROR) << "GLX 1.3 or later is required.";
        return nullptr;
    }

    s_initialized = true;
    return g_display;
}


bool GLSurfaceGLXQt::InitializeExtensionSettingsOneOff()
{
    if (!s_initialized)
        return false;

    Display* display = static_cast<Display*>(g_display->GetDisplay());
    GLSurfaceQt::g_extensions = glXQueryExtensionsString(display, 0);
    g_driver_glx.InitializeExtensionBindings(g_extensions.c_str());
    return true;
}

bool GLSurfaceGLX::InitializeExtensionSettingsOneOff()
{
    return GLSurfaceGLXQt::InitializeExtensionSettingsOneOff();
}

bool GLSurfaceGLXQt::Initialize(GLSurfaceFormat format)
{
    Q_ASSERT(!m_surfaceBuffer);

    Display* display = static_cast<Display*>(g_display->GetDisplay());
    const int pbuffer_attributes[] = {
        GLX_PBUFFER_WIDTH, m_size.width(),
        GLX_PBUFFER_HEIGHT, m_size.height(),
        GLX_LARGEST_PBUFFER, GL_FALSE,
        GLX_PRESERVED_CONTENTS, GL_FALSE,
        GL_NONE // MEMO doc: ...must be terminated with None or NULL
    };

    m_surfaceBuffer = glXCreatePbuffer(display, static_cast<GLXFBConfig>(g_config), pbuffer_attributes);
    m_format = format;

    if (!m_surfaceBuffer) {
        Destroy();
        LOG(ERROR) << "glXCreatePbuffer failed.";
        return false;
    }
    return true;
}

void GLSurfaceGLXQt::Destroy()
{
    if (m_surfaceBuffer) {
        glXDestroyPbuffer(static_cast<Display*>(g_display->GetDisplay()), m_surfaceBuffer);
        m_surfaceBuffer = 0;
    }
}

void* GLSurfaceGLXQt::GetHandle()
{
    return reinterpret_cast<void*>(m_surfaceBuffer);
}

} //namespace gl

