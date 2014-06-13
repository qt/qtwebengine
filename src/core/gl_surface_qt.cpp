/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "gl_context_qt.h"
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include "base/memory/scoped_ptr.h"
#include "ui/gl/egl_util.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface_egl.h"

#if defined(USE_OZONE) || defined(OS_ANDROID) || defined(OS_WIN)

using ui::GetLastEGLErrorString;

namespace gfx {

namespace {

EGLConfig g_config;
EGLDisplay g_display;
EGLNativeDisplayType g_native_display;

}  // namespace

bool GLSurface::InitializeOneOffInternal() {
    if (GetGLImplementation() == kGLImplementationOSMesaGL)
        return true;

    Q_ASSERT(GetGLImplementation() == kGLImplementationEGLGLES2);
    if (!GLSurfaceEGL::InitializeOneOff()) {
        qFatal("GLSurfaceEGL::InitializeOneOff failed.");
        return false;
    }
    return true;
}

bool GLSurfaceEGL::InitializeOneOff() {
    static bool initialized = false;
    if (initialized)
        return true;

    g_native_display = reinterpret_cast<EGLNativeDisplayType>(GLContextHelper::getNativeDisplay());
    if (!g_native_display)
         g_native_display = EGL_DEFAULT_DISPLAY;

    g_display = reinterpret_cast<EGLDisplay>(GLContextHelper::getEglDisplay());
    if (!g_display) {
        qFatal("GLContextHelper::getEglDisplay() failed.");
        return false;
    }

    g_config = reinterpret_cast<EGLConfig>(GLContextHelper::getEglConfig());
    if (!g_config) {
        qFatal("GLContextHelper::getEglConfig() failed.");
        return false;
    }

    if (!eglInitialize(g_display, NULL, NULL)) {
        qFatal("eglInitialize failed with error %s.", GetLastEGLErrorString());
        return false;
    }

    initialized = true;

    return true;
}

EGLDisplay GLSurfaceEGL::GetDisplay() {
    return g_display;
}

EGLDisplay GLSurfaceEGL::GetHardwareDisplay() {
    return g_display;
}

EGLNativeDisplayType GLSurfaceEGL::GetNativeDisplay() {
    return g_native_display;
}

EGLConfig PbufferGLSurfaceEGL::GetConfig() {
    return g_config;
}

// static
scoped_refptr<GLSurface>
GLSurface::CreateOffscreenGLSurface(const gfx::Size& size) {
    switch (GetGLImplementation()) {
        case kGLImplementationEGLGLES2: {
            scoped_refptr<GLSurface> surface = new PbufferGLSurfaceEGL(size);
            if (!surface->Initialize())
                return NULL;
            return surface;
        }
        default:
            NOTREACHED();
            return NULL;
    }
}

// static
scoped_refptr<GLSurface>
GLSurface::CreateViewGLSurface(gfx::AcceleratedWidget window) {
    return NULL;
}

}  // namespace gfx

#endif //defined(USE_OZONE) || defined(OS_ANDROID) || defined(OS_WIN)
