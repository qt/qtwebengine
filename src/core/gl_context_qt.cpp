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
#include <QThread>
#include "ui/gl/gl_context_egl.h"

#include <private/qopenglcontext_p.h>
#include <private/qsgcontext_p.h>
#include <qpa/qplatformnativeinterface.h>

QT_BEGIN_NAMESPACE

GLContextHelper* GLContextHelper::contextHelper = 0;

void GLContextHelper::initialize()
{
    if (!contextHelper)
        contextHelper = new GLContextHelper;
}

void GLContextHelper::destroy()
{
    delete contextHelper;
    contextHelper = 0;
}

bool GLContextHelper::initializeContextOnBrowserThread(gfx::GLContext* context, gfx::GLSurface* surface)
{
    return context->Initialize(surface, gfx::PreferDiscreteGpu);
}

bool GLContextHelper::initializeContext(gfx::GLContext* context, gfx::GLSurface* surface)
{
    bool ret = false;
    Qt::ConnectionType connType = (QThread::currentThread() == qApp->thread()) ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
    QMetaObject::invokeMethod(contextHelper, "initializeContextOnBrowserThread", connType,
            Q_RETURN_ARG(bool, ret),
            Q_ARG(gfx::GLContext*, context),
            Q_ARG(gfx::GLSurface*, surface));
    return ret;
}

void* GLContextHelper::getEglConfig()
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
    QOpenGLContext *shareContext = QSGContext::sharedOpenGLContext();
#else
    QOpenGLContext *shareContext = QOpenGLContextPrivate::globalShareContext();
#endif
    return qApp->platformNativeInterface()->nativeResourceForContext(QByteArrayLiteral("eglconfig"), shareContext);
}

void* GLContextHelper::getEglDisplay()
{
    return qApp->platformNativeInterface()->nativeResourceForIntegration(QByteArrayLiteral("egldisplay"));
}

void* GLContextHelper::getNativeDisplay()
{
    return qApp->platformNativeInterface()->nativeResourceForIntegration(QByteArrayLiteral("nativedisplay"));
}

QT_END_NAMESPACE

#if defined(USE_OZONE) || defined(OS_ANDROID) || defined(OS_WIN)

namespace gfx {

scoped_refptr<GLContext> GLContext::CreateGLContext(GLShareGroup* share_group, GLSurface* compatible_surface, GpuPreference gpu_preference)
{
    scoped_refptr<GLContext> context(new GLContextEGL(share_group));
    if (!GLContextHelper::initializeContext(context.get(), compatible_surface))
        return NULL;

    return context;
}

}  // namespace gfx

#endif // defined(USE_OZONE) || defined(OS_ANDROID) || defined(OS_WIN)


