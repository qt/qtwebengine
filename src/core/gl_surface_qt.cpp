/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "gl_surface_qt.h"

#if !defined(OS_MACOSX)

#include <QGuiApplication>
#include "gl_context_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "web_engine_context.h"
#include "ozone/gl_surface_egl_qt.h"

#include "base/logging.h"
#include "gpu/ipc/service/image_transport_surface.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/init/gl_initializer.h"
#include "ui/gl/init/gl_factory.h"

#if defined(OS_WIN)
#include "ui/gl/gl_surface_wgl.h"
#include "ui/gl/gl_context_wgl.h"
#include "ui/gl/vsync_provider_win.h"
#endif

#if defined(USE_X11)
#include "ozone/gl_surface_glx_qt.h"
#endif

#include "ozone/gl_surface_egl_qt.h"

namespace gl {

namespace {
bool g_initializedEGL = false;
}

void* GLSurfaceQt::g_display = NULL;
void* GLSurfaceQt::g_config = NULL;
const char* GLSurfaceQt::g_extensions = NULL;

GLSurfaceQt::~GLSurfaceQt()
{
}

#if defined(OS_WIN)

class GLSurfaceQtWGL: public GLSurfaceQt {
public:
    explicit GLSurfaceQtWGL(const gfx::Size& size);

    static bool InitializeOneOff();

    bool Initialize(GLSurfaceFormat format) override;
    void Destroy() override;
    void *GetHandle() override;
    void *GetDisplay() override;
    void *GetConfig() override;

protected:
    ~GLSurfaceQtWGL();

private:
    scoped_refptr<PbufferGLSurfaceWGL> m_surfaceBuffer;
    DISALLOW_COPY_AND_ASSIGN(GLSurfaceQtWGL);
};

GLSurfaceQtWGL::GLSurfaceQtWGL(const gfx::Size& size)
    : GLSurfaceQt(size),
      m_surfaceBuffer(0)
{
}

GLSurfaceQtWGL::~GLSurfaceQtWGL()
{
    Destroy();
}

bool GLSurfaceQtWGL::InitializeOneOff()
{
    return GLSurfaceWGL::InitializeOneOff();
}

bool GLSurfaceQtWGL::Initialize(GLSurfaceFormat format)
{
    m_surfaceBuffer = new PbufferGLSurfaceWGL(m_size);
    m_format = format;

    return m_surfaceBuffer->Initialize(format);
}

void GLSurfaceQtWGL::Destroy()
{
    m_surfaceBuffer = 0;
}

void *GLSurfaceQtWGL::GetHandle()
{
    return m_surfaceBuffer->GetHandle();
}

void *GLSurfaceQtWGL::GetDisplay()
{
    return m_surfaceBuffer->GetDisplay();
}

void *GLSurfaceQtWGL::GetConfig()
{
    return m_surfaceBuffer->GetConfig();
}

#endif // defined(OS_WIN)

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
    return ExtensionsContain(g_extensions, name);
}

bool GLSurfaceQt::IsOffscreen()
{
    return true;
}

gfx::SwapResult GLSurfaceQt::SwapBuffers(const PresentationCallback &callback)
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

void* GLSurfaceQt::GetDisplay()
{
    return g_display;
}

void* GLSurfaceQt::GetConfig()
{
    return g_config;
}

namespace init {

bool InitializeGLOneOffPlatform()
{
#if defined(OS_WIN)
    VSyncProviderWin::InitializeOneOff();
#endif

    if (GetGLImplementation() == kGLImplementationOSMesaGL)
        return false;

    if (GetGLImplementation() == kGLImplementationEGLGLES2)
        return GLSurfaceEGLQt::InitializeOneOff();

    if (GetGLImplementation() == kGLImplementationDesktopGL) {
#if defined(OS_WIN)
        return GLSurfaceQtWGL::InitializeOneOff();
#elif defined(USE_X11)
        if (GLSurfaceGLXQt::InitializeOneOff())
            return true;
#endif
        // Fallback to trying EGL with desktop GL.
        if (GLSurfaceEGLQt::InitializeOneOff()) {
            g_initializedEGL = true;
            return true;
        }
    }

    return false;
}

bool usingSoftwareDynamicGL()
{
    return QtWebEngineCore::usingSoftwareDynamicGL();
}

scoped_refptr<GLSurface>
CreateOffscreenGLSurfaceWithFormat(const gfx::Size& size, GLSurfaceFormat format)
{
    scoped_refptr<GLSurface> surface;
    switch (GetGLImplementation()) {
    case kGLImplementationDesktopGLCoreProfile:
    case kGLImplementationDesktopGL: {
#if defined(OS_WIN)
        surface = new GLSurfaceQtWGL(size);
        if (surface->Initialize(format))
            return surface;
        break;
#elif defined(USE_X11)
        if (!g_initializedEGL) {
            surface = new GLSurfaceGLXQt(size);
            if (surface->Initialize(format))
                return surface;
        }
        Q_FALLTHROUGH();
#endif
    }
    case kGLImplementationEGLGLES2: {
        surface = new GLSurfaceEGLQt(size);
        if (surface->Initialize(format))
            return surface;

        // Surfaceless context will be used ONLY if pseudo surfaceless context
        // is not available since some implementations of surfaceless context
        // have problems. (e.g. QTBUG-57290)
        if (GLSurfaceEGLQt::g_egl_surfaceless_context_supported) {
            surface = new GLSurfacelessQtEGL(size);
            if (surface->Initialize(format))
                return surface;
        }
        LOG(WARNING) << "Failed to create offscreen GL surface";
        break;
    }
    default:
        break;
    }
    LOG(ERROR) << "Requested OpenGL implementation is not supported. Implementation: " << GetGLImplementation();
    Q_UNREACHABLE();
    return NULL;
}

scoped_refptr<GLSurface>
CreateViewGLSurface(gfx::AcceleratedWidget window)
{
    QT_NOT_USED
    return NULL;
}

} // namespace init
}  // namespace gl

namespace gpu {
class GpuCommandBufferStub;
class GpuChannelManager;
scoped_refptr<gl::GLSurface> ImageTransportSurface::CreateNativeSurface(base::WeakPtr<ImageTransportSurfaceDelegate>,
                                                                        SurfaceHandle, gl::GLSurfaceFormat)
{
    QT_NOT_USED
    return scoped_refptr<gl::GLSurface>();
}
}

#endif // !defined(OS_MACOSX)
