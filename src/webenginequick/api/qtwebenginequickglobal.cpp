// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtCore/qcoreapplication.h>
#include <QtQuick/qquickwindow.h>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>

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
    \deprecated [6.12] This function is not any longer in use.

    Sets up an OpenGL Context that can be shared between threads. This has to be done before
    QGuiApplication is created and before window's QPlatformOpenGLContext is created.

    This has the same effect as setting the Qt::AA_ShareOpenGLContexts
    attribute with QCoreApplication::setAttribute before constructing
    QGuiApplication.
*/
#if QT_DEPRECATED_SINCE(6, 12)
void initialize() { }
#endif
} // namespace QtWebEngineQuick

QT_END_NAMESPACE
