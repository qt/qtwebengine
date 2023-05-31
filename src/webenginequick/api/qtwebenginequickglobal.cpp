// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/qcoreapplication.h>
#include <QtQuick/qquickwindow.h>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>

namespace QtWebEngineCore
{
    extern void initialize();
}

QT_BEGIN_NAMESPACE

namespace QtWebEngineQuick {

/*!
    \namespace QtWebEngineQuick
    \inmodule QtWebEngineQuick
    \ingroup qtwebengine-namespaces
    \keyword QtWebEngine Namespace

    \brief Helper functions for the \QWE (Qt Quick) module.

    The \l[CPP]{QtWebEngineQuick} namespace is part of the \QWE module.
*/

/*!
    \fn QtWebEngineQuick::initialize()

    Sets up an OpenGL Context that can be shared between threads. This has to be done before
    QGuiApplication is created and before window's QPlatformOpenGLContext is created.

    This has the same effect as setting the Qt::AA_ShareOpenGLContexts
    attribute with QCoreApplication::setAttribute before constructing
    QGuiApplication.
*/
void initialize()
{
    if (!QCoreApplication::startingUp()) {
        qWarning("QtWebEngineQuick::initialize() called with QCoreApplication object already created and should be call before. "\
                 "This is depreciated and may fail in the future.");
        QtWebEngineCore::initialize();
        return;
    }

    // call initialize the same way as widgets do
    qAddPreRoutine(QtWebEngineCore::initialize);
    auto api = QQuickWindow::graphicsApi();
    if (api != QSGRendererInterface::OpenGL && api != QSGRendererInterface::Vulkan
            && api != QSGRendererInterface::Metal && api != QSGRendererInterface::Direct3D11)
        QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
}
} // namespace QtWebEngineQuick

QT_END_NAMESPACE
