// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINESINGLETON_P_H
#define QQUICKWEBENGINESINGLETON_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <QtQml/qqml.h>
#include <QtWebEngineCore/qwebenginescript.h>
#include <QtWebEngineQuick/private/qtwebenginequickglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuickWebEngineSettings;
class QQuickWebEngineProfile;

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineSingleton : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQuickWebEngineSettings* settings READ settings CONSTANT FINAL)
    Q_PROPERTY(QQuickWebEngineProfile* defaultProfile READ defaultProfile CONSTANT FINAL REVISION(1,1))
    QML_SINGLETON
    QML_NAMED_ELEMENT(WebEngine)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
public:
    QQuickWebEngineSettings *settings() const;
    QQuickWebEngineProfile *defaultProfile() const;
    Q_INVOKABLE QWebEngineScript script() const;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINESINGLETON_P_H
