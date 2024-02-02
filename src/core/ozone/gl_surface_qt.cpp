// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "qtwebenginecoreglobal_p.h"

#if !defined(Q_OS_MACOS)

#include "gl_surface_qt.h"

#include "base/logging.h"

#if BUILDFLAG(IS_WIN)
#include "web_engine_context.h"
#include "ozone/gl_surface_wgl_qt.h"

#include "gpu/ipc/service/image_transport_surface.h"
#include "ui/gl/init/gl_display_initializer.h"
#include "ui/gl/direct_composition_support.h"
#include "ui/gl/gl_display.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface_egl.h"
#include "ui/gl/gl_utils.h"
#include "ui/gl/vsync_provider_win.h"
#endif


namespace gl {

GLDisplay *GLSurfaceQt::g_display = nullptr;
void *GLSurfaceQt::g_config = nullptr;
std::string GLSurfaceQt::g_extensions;

GLSurfaceQt::~GLSurfaceQt()
{
}

GLSurfaceQt::GLSurfaceQt()
{
}

GLSurfaceQt::GLSurfaceQt(const gfx::Size& size)
    : m_size(size)
{
    // Some implementations of Pbuffer do not support having a 0 size. For such
    // cases use a (1, 1) surface.
    if (m_size.GetArea() == 0)
        m_size.SetSize(1, 1);
}

bool GLSurfaceQt::HasEGLExtension(const char* name)
{
    return ExtensionsContain(g_extensions.c_str(), name);
}

bool GLSurfaceQt::IsOffscreen()
{
    return true;
}

gfx::SwapResult GLSurfaceQt::SwapBuffers(PresentationCallback callback, gfx::FrameData data)
{
    LOG(ERROR) << "Attempted to call SwapBuffers on a pbuffer.";
    Q_UNREACHABLE();
    return gfx::SwapResult::SWAP_FAILED;
}

gfx::Size GLSurfaceQt::GetSize()
{
    return m_size;
}

GLSurfaceFormat GLSurfaceQt::GetFormat()
{
    return m_format;
}

GLDisplay *GLSurfaceQt::GetGLDisplay()
{
    return g_display;
}

void* GLSurfaceQt::GetConfig()
{
    return g_config;
}

#if BUILDFLAG(IS_WIN)
namespace init {

gl::GLDisplay *InitializeGLOneOffPlatform(gl::GpuPreference gpu_preference)
{
    VSyncProviderWin::InitializeOneOff();

    if (GetGLImplementation() == kGLImplementationDesktopGL || GetGLImplementation() == kGLImplementationDesktopGLCoreProfile)
        return GLSurfaceWGLQt::InitializeOneOff(gpu_preference);

    GLDisplayEGL *display = GetDisplayEGL(gpu_preference);
    switch (GetGLImplementation()) {
    case kGLImplementationEGLANGLE:
    case kGLImplementationEGLGLES2:
        if (!InitializeDisplay(display, EGLDisplayPlatform(GetDC(nullptr)))) {
            LOG(ERROR) << "GLDisplayEGL::Initialize failed.";
            return nullptr;
        }
        InitializeDirectComposition(display);
        break;
    case kGLImplementationMockGL:
    case kGLImplementationStubGL:
        break;
    default:
        NOTREACHED();
    }
    return display;
}

bool usingSoftwareDynamicGL()
{
#if QT_CONFIG(opengl)
    return QtWebEngineCore::usingSoftwareDynamicGL();
#else
    return false;
#endif // QT_CONFIG(opengl)
}

scoped_refptr<GLSurface>
CreateOffscreenGLSurfaceWithFormat(GLDisplay *display, const gfx::Size& size, GLSurfaceFormat format)
{
    scoped_refptr<GLSurface> surface;
    switch (GetGLImplementation()) {
    case kGLImplementationDesktopGLCoreProfile:
    case kGLImplementationDesktopGL: {
        surface = new GLSurfaceWGLQt(size);
        if (surface->Initialize(format))
            return surface;
        break;
    }
    case kGLImplementationEGLANGLE:
    case kGLImplementationEGLGLES2: {
        GLDisplayEGL *display_egl = display->GetAs<gl::GLDisplayEGL>();
        if (display_egl->IsEGLSurfacelessContextSupported() && size.width() == 0 && size.height() == 0)
            return InitializeGLSurfaceWithFormat(new SurfacelessEGL(display_egl, size), format);
        return InitializeGLSurfaceWithFormat(new PbufferGLSurfaceEGL(display_egl, size), format);
    }
    default:
        break;
    }
    LOG(ERROR) << "Requested OpenGL implementation is not supported. Implementation: " << GetGLImplementation();
    Q_UNREACHABLE();
    return nullptr;
}

scoped_refptr<GLSurface>
CreateViewGLSurface(GLDisplay *display, gfx::AcceleratedWidget window)
{
    return nullptr;
}

} // namespace init
#endif  // BUILDFLAG(IS_WIN)
} // namespace gl

#endif // !defined(Q_OS_MACOS)
