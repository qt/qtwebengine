/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#include <QtQml/qqmlextensionplugin.h>

#include "qtwebengineversion.h"
#include "qquickwebengineview_p.h"
#include "qquickwebengineloadrequest_p.h"
#include "qquickwebenginenewviewrequest_p.h"

QT_BEGIN_NAMESPACE

class QQuickWebEngineVersionBumper : public QObject {
    Q_OBJECT
};

class QtWebEnginePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")
public:
    virtual void registerTypes(const char *uri) Q_DECL_OVERRIDE
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtWebEngine"));

        qmlRegisterType<QQuickWebEngineView>(uri, 0, 9, "WebEngineView");
        qmlRegisterUncreatableType<QQuickWebEngineLoadRequest>(uri, 0, 9, "WebEngineLoadRequest", QObject::tr("Cannot create separate instance of WebEngineLoadRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineNewViewRequest>(uri, 0, 9, "WebEngineNewViewRequest", QObject::tr("Cannot create separate instance of WebEngineNewViewRequest"));

        // The QML type loader relies on the minimum and maximum minor version of registered types
        // to validate imports. We want to tie our import version to the module version, so register
        // a dummy type in order to allow importing the latest version even if it didn't include
        // an API update that would appear here in a registered type.
        int major = QTWEBENGINE_VERSION >> 16;
        int minor = QTWEBENGINE_VERSION >> 8;
        qmlRegisterUncreatableType<QQuickWebEngineVersionBumper>(uri, major, minor, "WebEngineVersionBumper", QObject::tr("This is a dummy type and cannot be created."));
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
