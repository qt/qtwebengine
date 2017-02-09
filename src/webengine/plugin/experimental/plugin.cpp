/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include <QtQml/qqmlextensionplugin.h>

#include "qquickwebengineview_p.h"
#include "qquickwebengineview_p_p.h"
#include "qquickwebenginecontextmenudata_p.h"

QT_BEGIN_NAMESPACE

class QQuickWebEngineViewExperimentalExtension : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQuickWebEngineViewExperimental* experimental READ experimental CONSTANT FINAL)
public:
    QQuickWebEngineViewExperimentalExtension(QObject *parent = 0) : QObject(parent) { }
    QQuickWebEngineViewExperimental* experimental() { return static_cast<QQuickWebEngineView*>(parent())->d_func()->experimental(); }
};

class QtWebEngineExperimentalPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")
public:
    virtual void registerTypes(const char *uri)
    {
        qWarning("\nWARNING: This project is using the experimental QML API extensions for QtWebEngine and is therefore tied to a specific QtWebEngine release.\n"
                 "WARNING: The experimental API will change from version to version, or even be removed. You have been warned!\n");

        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtWebEngine.experimental"));

        qmlRegisterExtendedType<QQuickWebEngineView, QQuickWebEngineViewExperimentalExtension>(uri, 1, 0, "WebEngineView");
        qmlRegisterUncreatableType<QQuickWebEngineViewExperimental>(uri, 1, 0, "WebEngineViewExperimental",
            tr("Cannot create a separate instance of WebEngineViewExperimental"));
        qmlRegisterUncreatableType<QQuickWebEngineViewport>(uri, 1, 0, "WebEngineViewport",
            tr("Cannot create a separate instance of WebEngineViewport"));
        qmlRegisterUncreatableType<const QQuickWebEngineContextMenuData>(uri, 1, 0, "WebEngineContextMenuData",
            tr("Cannot create a separate instance of WebEngineContextMenuData"));

        // Use the latest revision of QQuickWebEngineView when importing QtWebEngine.experimental 1.0
        qmlRegisterRevision<QQuickWebEngineView, LATEST_WEBENGINEVIEW_REVISION>(uri, 1, 0);
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
