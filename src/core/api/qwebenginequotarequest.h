// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEQUOTAREQUEST_H
#define QWEBENGINEQUOTAREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qurl.h>

#if QT_DEPRECATED_SINCE(6, 5)

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineQuotaRequest
{
    Q_GADGET
    Q_PROPERTY(QUrl origin READ origin CONSTANT FINAL)
    Q_PROPERTY(qint64 requestedSize READ requestedSize CONSTANT FINAL)
public:
    QT_DEPRECATED_VERSION_X_6_5("Requesting host quota is no longer supported.")
    QWebEngineQuotaRequest() {}
    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();
    QUrl origin() const;
    qint64 requestedSize() const;
    bool operator==(const QWebEngineQuotaRequest &) const { Q_UNREACHABLE(); }
    bool operator!=(const QWebEngineQuotaRequest &) const { Q_UNREACHABLE(); }
};

QT_END_NAMESPACE

#endif // QT_DEPRECATED_SINCE(6, 5)

#endif // QWEBENGINEQUOTAREQUEST_H
