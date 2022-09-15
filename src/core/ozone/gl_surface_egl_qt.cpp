// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gl_context_qt.h"
#include "ozone/gl_surface_egl_qt.h"

#if !BUILDFLAG(IS_MAC)
#include "ui/gl/egl_util.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_display.h"
#include "ui/gl/gl_surface_egl.h"
#include "ui/gl/init/gl_factory.h"

// From ANGLE's egl/eglext.h.
#ifndef EGL_ANGLE_surface_d3d_texture_2d_share_handle
#define EGL_ANGLE_surface_d3d_texture_2d_share_handle 1
#define EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE 0x3200
#endif

using ui::GetLastEGLErrorString;

namespace gl {

bool GLSurfaceEGL::InitializeExtensionSettingsOneOff(GLDisplayEGL* display)
{
    return GLSurfaceEGLQt::InitializeExtensionSettingsOneOff();
}

EGLDisplay GLDisplayEGL::GetHardwareDisplay()
{
    return GLSurfaceQt::g_display ? static_cast<EGLDisplay>(GLSurfaceQt::g_display->GetDisplay()) : EGL_NO_DISPLAY;
}

bool GLDisplayEGL::IsCreateContextRobustnessSupported()
{
    return GLContextHelper::isCreateContextRobustnessSupported() && HasEGLExtension("EGL_EXT_create_context_robustness");
}

bool GLDisplayEGL::IsCreateContextBindGeneratesResourceSupported()
{
    return false;
}

bool GLDisplayEGL::IsCreateContextWebGLCompatabilitySupported()
{
    return false;
}
bool GLDisplayEGL::IsEGLSurfacelessContextSupported()
{
    return GLSurfaceEGLQt::g_egl_surfaceless_context_supported;
}
bool GLDisplayEGL::IsEGLContextPrioritySupported()
{
    return false;
}

bool GLDisplayEGL::IsRobustResourceInitSupported()
{
    return false;
}

bool GLDisplayEGL::IsDisplayTextureShareGroupSupported()
{
    return false;
}

bool GLDisplayEGL::IsCreateContextClientArraysSupported()
{
    return false;
}

bool GLDisplayEGL::IsPixelFormatFloatSupported()
{
    return false;
}

bool GLDisplayEGL::IsANGLEFeatureControlSupported()
{
    return false;
}

bool GLDisplayEGL::IsANGLEPowerPreferenceSupported()
{
    return false;
}

bool GLDisplayEGL::IsANGLEExternalContextAndSurfaceSupported()
{
    return false;
}

bool GLDisplayEGL::IsDisplaySemaphoreShareGroupSupported()
{
    return false;
}

bool GLDisplayEGL::IsRobustnessVideoMemoryPurgeSupported()
{
    return false;
}

bool GLDisplayEGL::IsANGLEContextVirtualizationSupported()
{
    return false;
}

bool GLDisplayEGL::IsANGLEVulkanImageSupported()
{
     return false;
}

bool GLDisplayEGL::IsEGLQueryDeviceSupported()
{
    return false;
}

void GLSurfaceEGL::ShutdownOneOff(GLDisplayEGL *)
{
}

const char *GetEGLClientExtensions()
{
    return GLSurfaceQt::g_client_extensions.c_str();
}

const char *GetEGLExtensions()
{
    return GLSurfaceQt::g_extensions.c_str();
}

bool GLDisplayEGL::HasEGLClientExtension(const char *name)
{
    return GLSurface::ExtensionsContain(GetEGLClientExtensions(), name);
}

bool GLDisplayEGL::HasEGLExtension(const char *name)
{
    return GLSurface::ExtensionsContain(GetEGLExtensions(), name);
}

GLDisplayEGL *GLSurfaceEGL::InitializeOneOff(gl::EGLDisplayPlatform /*native_display*/, uint64_t system_id)
{
    return static_cast<GLDisplayEGL *>(GLSurfaceEGLQt::InitializeOneOff(system_id));
}

bool GLDisplayEGL::IsEGLNoConfigContextSupported()
{
    return false;
}

bool GLDisplayEGL::IsAndroidNativeFenceSyncSupported()
{
     return false;
}

GLDisplayEGL *GLSurfaceEGL::GetGLDisplayEGL()
{
     return static_cast<GLDisplayEGL *>(GLSurfaceEGLQt::g_display);
}

DisplayType GLDisplayEGL::GetDisplayType()
{
     return DisplayType::DEFAULT;
}

GLSurface *GLSurfaceEGL::createSurfaceless(const gfx::Size& size)
{
    return new GLSurfacelessQtEGL(size);
}

bool GLSurfaceEGLQt::g_egl_surfaceless_context_supported = false;
bool GLSurfaceEGLQt::s_initialized = false;

GLSurfaceEGLQt::GLSurfaceEGLQt(const gfx::Size& size)
    : GLSurfaceQt(size),
      m_surfaceBuffer(0)
{
}

GLSurfaceEGLQt::~GLSurfaceEGLQt()
{
    Destroy();
}

gl::GLDisplay *GLSurfaceEGLQt::InitializeOneOff(uint64_t system_device_id)
{
    if (s_initialized)
        return g_display;

    auto *egl_display = new GLDisplayEGL(system_device_id);
    g_display = egl_display;
    egl_display->SetDisplay(GLContextHelper::getEGLDisplay());
    if (!g_display->GetDisplay()) {
        LOG(ERROR) << "GLContextHelper::getEGLDisplay() failed.";
        return nullptr;
    }

    g_config = GLContextHelper::getEGLConfig();
    if (!g_config) {
        LOG(ERROR) << "GLContextHelper::getEGLConfig() failed.";
        return nullptr;
    }

    if (!eglInitialize(g_display->GetDisplay(), NULL, NULL)) {
        LOG(ERROR) << "eglInitialize failed with error " << GetLastEGLErrorString();
        return nullptr;
    }

    g_client_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    g_extensions = eglQueryString(g_display->GetDisplay(), EGL_EXTENSIONS);
    g_egl_surfaceless_context_supported = ExtensionsContain(g_extensions.c_str(), "EGL_KHR_surfaceless_context");
    if (g_egl_surfaceless_context_supported) {
        scoped_refptr<GLSurface> surface = new GLSurfacelessQtEGL(gfx::Size(1, 1));
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
    return g_display;
}

bool GLSurfaceEGLQt::InitializeExtensionSettingsOneOff()
{
    return s_initialized;
}

bool GLSurfaceEGLQt::Initialize(GLSurfaceFormat format)
{
    Q_ASSERT(!m_surfaceBuffer);
    m_format = format;

    EGLDisplay display = g_display->GetDisplay();
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
        if (!eglDestroySurface(g_display->GetDisplay(), m_surfaceBuffer))
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

GLSurfacelessQtEGL::GLSurfacelessQtEGL(const gfx::Size& size)
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
#endif // !BUILDFLAG(IS_MAC)

namespace gl {

std::string DisplayExtensionsEGL::GetPlatformExtensions(void*)
{
    EGLDisplay display = GLContextHelper::getEGLDisplay();
    if (display == EGL_NO_DISPLAY)
        return "";

    const char* str = eglQueryString(display, EGL_EXTENSIONS);
    return str ? std::string(str) : "";
}
} // namespace gl
