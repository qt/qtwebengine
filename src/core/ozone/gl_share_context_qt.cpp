/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include "gl_share_context_qt.h"
#include <QtGui/qtgui-config.h>
#include <qpa/qplatformnativeinterface.h>
#include <QtGui/qopenglcontext_platform.h>
#if defined(Q_OS_MACOS)
#include "macos_context_type_helper.h"
#endif
#if QT_CONFIG(opengl)
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#endif

namespace QtWebEngineCore {

QtShareGLContext::QtShareGLContext(QOpenGLContext *qtContext)
    : gl::GLContext(nullptr), m_handle(nullptr)
{
#if QT_CONFIG(opengl)
    QOpenGLContext *context = QOpenGLContext::globalShareContext();
#if defined(Q_OS_MACOS)
    auto *mac_ctx = context->nativeInterface<QNativeInterface::QCocoaGLContext>();
    if (mac_ctx)
        m_handle = cglContext(mac_ctx->nativeContext());
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
        qFatal("Could not get handle for shared contex");
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
#if QT_CONFIG(opengl)
    // This currently has to be setup by ::main in all applications using QQuickWebEngineView with
    // delegated rendering.
    QOpenGLContext *shareContext = QOpenGLContext::globalShareContext();
    if (!shareContext) {
        qFatal("QWebEngine: OpenGL resource sharing is not set up in QtQuick. Please make sure to "
               "call QtWebEngineCore::initialize() in your main() function before QCoreApplication is "
               "created.");
    }
    m_shareContextQt = new QtShareGLContext(shareContext);
#endif
}

} // namespace
