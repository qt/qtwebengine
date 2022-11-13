// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINEDOWNLOADREQUEST_P_H
#define QQUICKWEBENGINEDOWNLOADREQUEST_P_H

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

#include <QtWebEngineQuick/private/qtwebenginequickglobal_p.h>
#include <QtWebEngineQuick/private/qquickwebengineview_p.h>
#include <QtWebEngineCore/qwebenginedownloadrequest.h>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE

class QQuickWebEngineProfilePrivate;

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineDownloadRequest : public QWebEngineDownloadRequest
{
    Q_OBJECT
public:
    Q_PROPERTY(QQuickWebEngineView *view READ view CONSTANT FINAL)
    QML_NAMED_ELEMENT(WebEngineDownloadRequest)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")

    QQuickWebEngineView *view() const;
private:
    Q_DISABLE_COPY(QQuickWebEngineDownloadRequest)
    friend class QQuickWebEngineProfilePrivate;
    QQuickWebEngineDownloadRequest(QWebEngineDownloadRequestPrivate *, QObject *parent = nullptr);
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEDOWNLOADREQUEST_P_H
