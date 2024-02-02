// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINEDOWNLOADREQUEST_H
#define QQUICKWEBENGINEDOWNLOADREQUEST_H

#include <QtWebEngineCore/qwebenginedownloadrequest.h>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE

class QQuickWebEngineView;
class QQuickWebEngineProfilePrivate;

class Q_WEBENGINEQUICK_EXPORT QQuickWebEngineDownloadRequest : public QWebEngineDownloadRequest
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

#endif // QQUICKWEBENGINEDOWNLOADREQUEST_H
