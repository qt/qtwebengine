// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "gl_share_context_qt.h"
#include <QtGui/qtgui-config.h>
#include <qpa/qplatformnativeinterface.h>

#include "ui/gl/gl_context_egl.h"
#include "ui/gl/gl_implementation.h"

#if QT_CONFIG(opengl)
#include <QtGui/qopenglcontext_platform.h>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#endif // QT_CONFIG(opengl)

namespace QtWebEngineCore {

QtShareGLContext::QtShareGLContext(QOpenGLContext *context)
    : gl::GLContext(nullptr), m_handle(nullptr)
{
#if QT_CONFIG(opengl)
#if defined(Q_OS_MACOS)
    qFatal("macOS only support using ANGLE.");
#endif
#if defined(Q_OS_WIN)
    auto *win_ctx = context->nativeInterface<QNativeInterface::QWGLContext>();
    if (win_ctx && !m_handle)
        m_handle = (void *)win_ctx->nativeContext();
#endif
#if QT_CONFIG(xcb_glx_plugin)
    auto *glx_ctx = context->nativeInterface<QNativeInterface::QGLXContext>();
    if (glx_ctx && !m_handle)
        m_handle = (void *)glx_ctx->nativeContext();
#endif
#if QT_CONFIG(egl)
    auto *egl_ctx = context->nativeInterface<QNativeInterface::QEGLContext>();
    if (egl_ctx  && !m_handle)
        m_handle = (void *)egl_ctx->nativeContext();
#endif
    if (!m_handle)
        qFatal("Could not get handle for shared context.");
#endif // QT_CONFIG(opengl)
}

unsigned int QtShareGLContext::CheckStickyGraphicsResetStatusImpl()
{
#if QT_CONFIG(opengl)
    if (QOpenGLContext *context = QOpenGLContext::globalShareContext()) {
        if (context->format().testOption(QSurfaceFormat::ResetNotification))
            return context->extraFunctions()->glGetGraphicsResetStatus();
    }
#endif
    return 0 /*GL_NO_ERROR*/;
}

void ShareGroupQt::AboutToAddFirstContext()
{
    if (gl::GetGLImplementation() == gl::kGLImplementationEGLANGLE) {
        m_shareContextQt = new gl::GLContextEGL(nullptr);
        return;
    }

#if QT_CONFIG(opengl)
    // This currently has to be setup by ::main in all applications using QQuickWebEngineView with
    // delegated rendering.
    QOpenGLContext *shareContext = QOpenGLContext::globalShareContext();
    if (!shareContext) {
        qFatal("QWebEngine: OpenGL resource sharing is not set up in QtQuick. Please make sure to "
               "call QtWebEngineQuick::initialize() in your main() function before "
               "QCoreApplication is created.");
    }
    m_shareContextQt = new QtShareGLContext(shareContext);
#endif // QT_CONFIG(opengl)
}

} // namespace
