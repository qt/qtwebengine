// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "ozone_util_qt.h"

#include <QtGui/qguiapplication.h>
#include <qpa/qplatformnativeinterface.h>

#if QT_CONFIG(opengl)
#include <QtGui/qopenglcontext.h>
#endif

QT_BEGIN_NAMESPACE

namespace OzoneUtilQt {
void *getXDisplay()
{
#if QT_CONFIG(xcb)
    auto *x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (x11Application)
        return x11Application->display();
#endif

    return nullptr;
}

QOpenGLContext *getQOpenGLContext()
{
#if QT_CONFIG(opengl)
    if (auto *shareContext = QOpenGLContext::globalShareContext())
        return shareContext;

    if (auto *currentContext = QOpenGLContext::currentContext())
        return currentContext;

    static QOpenGLContext *tmpGLContext = []() {
        auto tmpGLContext = new QOpenGLContext();
        tmpGLContext->create();
        QObject::connect(qGuiApp, &QGuiApplication::aboutToQuit, [=]() { delete tmpGLContext; });
        return tmpGLContext;
    }();

    return tmpGLContext;
#else
    return nullptr;
#endif
}

bool usingGLX()
{
#if QT_CONFIG(opengl) && QT_CONFIG(xcb_glx_plugin)
    static bool result = []() {
        QOpenGLContext *context = getQOpenGLContext();
        return context->nativeInterface<QNativeInterface::QGLXContext>() != nullptr;
    }();

    return result;
#else
    return false;
#endif
}

bool usingEGL()
{
#if QT_CONFIG(opengl) && QT_CONFIG(egl)
    static bool result = []() {
        QOpenGLContext *context = getQOpenGLContext();
        return context->nativeInterface<QNativeInterface::QEGLContext>() != nullptr;
    }();

    return result;
#else
    return false;
#endif
}
} // namespace OzoneUtilQt

QT_END_NAMESPACE
