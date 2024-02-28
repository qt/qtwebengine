// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GL_GL_CONTEXT_QT_H_
#define GL_GL_CONTEXT_QT_H_

#include <QObject>
#include <QtCore/qscopedpointer.h>

#include "ui/gl/gl_context.h"

#if defined(USE_OZONE)
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "base/files/scoped_file.h"
#endif

namespace gl {
class GLSurface;
}

namespace ui {
class GbmDevice;
}

QT_BEGIN_NAMESPACE

class GLContextHelper : public QObject {
    Q_OBJECT
public:
    static void initialize();
    static void destroy();
    static bool initializeContext(gl::GLContext* context, gl::GLSurface* surface, gl::GLContextAttribs attribs);

    static void* getEGLConfig();
    static void* getGlXConfig();
    static void* getEGLDisplay();
    static void* getXDisplay();
    static void* getNativeDisplay();
    static QFunctionPointer getGlXGetProcAddress();
    static QFunctionPointer getEglGetProcAddress();
    static bool isCreateContextRobustnessSupported();
    static void *getGlxPlatformInterface();
    static void *getEglPlatformInterface();

private:
    Q_INVOKABLE bool initializeContextOnBrowserThread(gl::GLContext* context, gl::GLSurface* surface, gl::GLContextAttribs attribs);

    static GLContextHelper* contextHelper;
    bool m_robustness = false;
};

#if defined(USE_OZONE)
#undef eglQueryDevices
#undef eglQueryDeviceString
#undef eglQueryString

class EGLHelper
{
public:
    struct EGLFunctions
    {
        EGLFunctions();

        PFNEGLQUERYDEVICESEXTPROC eglQueryDevices;
        PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceString;
        PFNEGLQUERYSTRINGPROC eglQueryString;
    };

    static EGLHelper *instance();

    EGLFunctions *functions() const { return m_functions.get(); }
    EGLDeviceEXT getEGLDevice() const { return m_eglDevice; }
    ui::GbmDevice *getGbmDevice();

private:
    EGLHelper();

    QScopedPointer<EGLFunctions> m_functions;
    EGLDeviceEXT m_eglDevice = EGL_NO_DEVICE_EXT;
    base::ScopedFD m_drmRenderNodeFd;
    std::unique_ptr<ui::GbmDevice> m_gbmDevice;
    bool m_isDmaBufSupported = false;
};
#endif // defined(USE_OZONE)

QT_END_NAMESPACE

#endif

