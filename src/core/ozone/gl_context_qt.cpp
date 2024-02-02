// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "gl_context_qt.h"

#include <QGuiApplication>
#include <QOpenGLContext>
#include <QThread>
#include <QtGui/private/qtgui-config_p.h>
#include <qpa/qplatformnativeinterface.h>

#if BUILDFLAG(IS_WIN)
#include "ui/gl/gl_context_egl.h"
#include "ui/gl/gl_context_wgl.h"
#include "ui/gl/gl_implementation.h"
#endif

QT_BEGIN_NAMESPACE

Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
GLContextHelper* GLContextHelper::contextHelper = nullptr;

namespace {

inline void *resourceForContext(const QByteArray &resource)
{
#if QT_CONFIG(opengl)
    QOpenGLContext *shareContext = qt_gl_global_share_context();
    if (!shareContext) {
        qFatal("QWebEngine: OpenGL resource sharing is not set up in QtQuick. Please make sure to "
               "call QtWebEngineQuick::initialize() in your main() function.");
    }
    return qApp->platformNativeInterface()->nativeResourceForContext(resource, shareContext);
#else
    return nullptr;
#endif
}

inline void *resourceForIntegration(const QByteArray &resource)
{
    return qApp->platformNativeInterface()->nativeResourceForIntegration(resource);
}

}

void GLContextHelper::initialize()
{
    if (!contextHelper)
        contextHelper = new GLContextHelper;
#if QT_CONFIG(opengl)
    if (QGuiApplication::platformName() == QLatin1String("offscreen")){
        contextHelper->m_robustness = false;
        return;
    }

    if (QOpenGLContext *context = qt_gl_global_share_context())
        contextHelper->m_robustness = context->format().testOption(QSurfaceFormat::ResetNotification);
#endif
}

void GLContextHelper::destroy()
{
    delete contextHelper;
    contextHelper = nullptr;
}

bool GLContextHelper::initializeContextOnBrowserThread(gl::GLContext* context, gl::GLSurface* surface, gl::GLContextAttribs attribs)
{
    return context->Initialize(surface, attribs);
}

bool GLContextHelper::initializeContext(gl::GLContext* context, gl::GLSurface* surface, gl::GLContextAttribs attribs)
{
    bool ret = false;
    Qt::ConnectionType connType = (QThread::currentThread() == qApp->thread()) ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
    QMetaObject::invokeMethod(contextHelper, "initializeContextOnBrowserThread", connType,
            Q_RETURN_ARG(bool, ret),
            Q_ARG(gl::GLContext*, context),
            Q_ARG(gl::GLSurface*, surface),
            Q_ARG(gl::GLContextAttribs, attribs));
    return ret;
}

void* GLContextHelper::getEGLConfig()
{
    QByteArray resource = QByteArrayLiteral("eglconfig");
    return resourceForContext(resource);
}

void* GLContextHelper::getGlXConfig()
{
    QByteArray resource = QByteArrayLiteral("glxconfig");
    return resourceForContext(resource);
}

void* GLContextHelper::getEGLDisplay()
{
#if BUILDFLAG(IS_WIN)
    // Windows QPA plugin does not implement resourceForIntegration for "egldisplay".
    // Use resourceForContext instead.
    return resourceForContext(QByteArrayLiteral("egldisplay"));
#else
    return resourceForIntegration(QByteArrayLiteral("egldisplay"));
#endif
}

void* GLContextHelper::getXDisplay()
{
#if QT_CONFIG(xcb)
    auto *x11app = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    return x11app ? x11app->display() : nullptr;
#else
    return nullptr;
#endif
}

void* GLContextHelper::getNativeDisplay()
{
    return resourceForIntegration(QByteArrayLiteral("nativedisplay"));
}

QFunctionPointer GLContextHelper::getGlXGetProcAddress()
{
     QFunctionPointer get_proc_address = nullptr;
#if QT_CONFIG(xcb_glx)
    if (QOpenGLContext *context = qt_gl_global_share_context()) {
        get_proc_address = context->getProcAddress("glXGetProcAddress");
    }
#endif
    return get_proc_address;
}

QFunctionPointer GLContextHelper::getEglGetProcAddress()
{
     QFunctionPointer get_proc_address = nullptr;
#if QT_CONFIG(opengl)
    if (QOpenGLContext *context = qt_gl_global_share_context()) {
        get_proc_address = context->getProcAddress("eglGetProcAddress");
    }
#endif
    return get_proc_address;
}

void *GLContextHelper::getGlxPlatformInterface()
{
#if QT_CONFIG(xcb_glx)
    if (QOpenGLContext *context = qt_gl_global_share_context())
        return context->nativeInterface<QNativeInterface::QGLXContext>();
#endif
    return nullptr;
}

void *GLContextHelper::getEglPlatformInterface()
{
#if QT_CONFIG(opengl) && QT_CONFIG(egl)
    if (QOpenGLContext *context = qt_gl_global_share_context())
        return context->nativeInterface<QNativeInterface::QEGLContext>();
#endif
    return nullptr;
}

bool GLContextHelper::isCreateContextRobustnessSupported()
{
    return contextHelper->m_robustness;
}

QT_END_NAMESPACE

#if BUILDFLAG(IS_WIN)
namespace gl {
namespace init {

scoped_refptr<GLContext> CreateGLContext(GLShareGroup* share_group,
                                         GLSurface* compatible_surface,
                                         const GLContextAttribs& attribs)
{
    if (GetGLImplementation() == kGLImplementationDesktopGL) {
        scoped_refptr<GLContext> context = new GLContextWGL(share_group);
        if (!context->Initialize(compatible_surface, attribs))
            return nullptr;
        return context;
    }

    return InitializeGLContext(new GLContextEGL(share_group),
                               compatible_surface, attribs);
}

}  // namespace init
}  // namespace gl

#endif // BUILDFLAG(IS_WIN)
