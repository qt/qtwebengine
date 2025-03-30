// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "gl_surface_wgl_qt.h"
#include "web_engine_context.h"

#include "ui/gl/init/gl_display_initializer.h"
#include "ui/gl/direct_composition_support.h"
#include "ui/gl/gl_angle_util_win.h"
#include "ui/gl/gl_display.h"
#include "ui/gl/gl_display_manager.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface_egl.h"
#include "ui/gl/gl_surface_wgl.h"
#include "ui/gl/gl_utils.h"
#include "ui/gl/vsync_provider_win.h"

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

gl::GLDisplay *GLSurfaceWGLQt::InitializeOneOff(gl::GpuPreference gpu_preference)
{
    if (GLSurfaceWGL::InitializeOneOff())
        return GLDisplayManagerWGL::GetInstance()->GetDisplay(gpu_preference);

    return nullptr;
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

// Overrides for WGL:
namespace init {

gl::GLDisplay *InitializeGLOneOffPlatform(gl::GpuPreference gpu_preference)
{
    VSyncProviderWin::InitializeOneOff();

    if (GetGLImplementation() == kGLImplementationDesktopGL || GetGLImplementation() == kGLImplementationDesktopGLCoreProfile)
        return GLSurfaceWGLQt::InitializeOneOff(gpu_preference);

    GLDisplayEGL *display = GetDisplayEGL(gpu_preference);
    switch (GetGLImplementation()) {
    case kGLImplementationEGLANGLE:
        if (!InitializeDisplay(display, EGLDisplayPlatform(GetDC(nullptr)))) {
            LOG(ERROR) << "GLDisplayEGL::Initialize failed.";
            return nullptr;
        }
        if (auto d3d11_device = QueryD3D11DeviceObjectFromANGLE())
            InitializeDirectComposition(std::move(d3d11_device));
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

scoped_refptr<GLSurface> CreateOffscreenGLSurface(GLDisplay *display, const gfx::Size &size)
{
    scoped_refptr<GLSurface> surface;
    switch (GetGLImplementation()) {
    case kGLImplementationDesktopGLCoreProfile:
    case kGLImplementationDesktopGL: {
        surface = new GLSurfaceWGLQt(size);
        if (surface->Initialize(GLSurfaceFormat()))
            return surface;
        break;
    }
    case kGLImplementationEGLANGLE: {
        GLDisplayEGL *display_egl = display->GetAs<gl::GLDisplayEGL>();
        if (display_egl->IsEGLSurfacelessContextSupported() && size.width() == 0 && size.height() == 0)
            return InitializeGLSurface(new SurfacelessEGL(display_egl, size));
        return InitializeGLSurface(new PbufferGLSurfaceEGL(display_egl, size));
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
} // namespace gl
