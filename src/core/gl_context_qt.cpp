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

#include "ui/gl/gl_context.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_context_egl.h"
#include "ui/gl/egl_util.h"

#include <QDebug>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>

#define EGL_CONTEXT_CLIENT_VERSION        0x3098
#define EGL_NONE                          0x3038

QT_BEGIN_NAMESPACE

GLContextHelper* GLContextHelper::contextHelper = 0;

GLContextHelper::GLContextHelper(QOpenGLContext* share)
    : shareContext(share)
{ }

void GLContextHelper::initialize(QOpenGLContext* shareContext)
{
    contextHelper = new GLContextHelper(shareContext);
}

void GLContextHelper::createEGLContext(EGLDisplay *egldisplay, EGLConfig *eglconfig, EGLContext *eglcontext)
{
    QMetaObject::invokeMethod(contextHelper, "createEGLContextOnBrowserThread", Qt::BlockingQueuedConnection,
            Q_ARG(EGLDisplay*, egldisplay),
            Q_ARG(EGLConfig*, eglconfig),
            Q_ARG(EGLContext*, eglcontext));
}

void GLContextHelper::createEGLContextOnBrowserThread(EGLDisplay *egldisplay, EGLConfig *eglconfig, EGLContext *eglcontext)
{
#if defined(USE_OZONE)
    static const int32_t kContextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    QPlatformNativeInterface *pni = QGuiApplication::platformNativeInterface();
    *eglconfig = pni->nativeResourceForContext(QByteArrayLiteral("eglconfig"), shareContext);
    *egldisplay = pni->nativeResourceForIntegration(QByteArrayLiteral("egldisplay"));
    EGLContext eglshare = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), shareContext);
    *eglcontext = eglCreateContext(*egldisplay, *eglconfig, eglshare, kContextAttributes);
    if (!*eglcontext)
        qWarning() << "EGL Context Creation failed: " << ui::GetLastEGLErrorString();
#endif
}

QT_END_NAMESPACE

#if defined(USE_OZONE)

namespace gfx {

scoped_refptr<GLContext> GLContext::CreateGLContext(GLShareGroup* share_group, GLSurface* compatible_surface, GpuPreference gpu_preference)
{
    EGLDisplay egldisplay = 0;
    EGLConfig eglconfig = 0;
    EGLContext eglcontext = 0;
    GLContextHelper::createEGLContext(&egldisplay, &eglconfig, &eglcontext);
    scoped_refptr<GLContext> context(new GLContextEGL(egldisplay, eglconfig, eglcontext));
    if (!context->Initialize(compatible_surface, gpu_preference))
        return NULL;

    return context;
}

}  // namespace gfx

#endif // defined(USE_OZONE)


