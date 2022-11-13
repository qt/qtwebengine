// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEQUOTAREQUEST_H
#define QWEBENGINEQUOTAREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qsharedpointer.h>
#include <QtCore/qurl.h>

namespace QtWebEngineCore {
class QuotaPermissionContextQt;
class QuotaRequestController;
} // namespace QtWebEngineCore

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineQuotaRequest
{
    Q_GADGET
    Q_PROPERTY(QUrl origin READ origin CONSTANT FINAL)
    Q_PROPERTY(qint64 requestedSize READ requestedSize CONSTANT FINAL)
public:
    QWebEngineQuotaRequest() {}
    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();
    QUrl origin() const;
    qint64 requestedSize() const;
    bool operator==(const QWebEngineQuotaRequest &that) const { return d_ptr == that.d_ptr; }
    bool operator!=(const QWebEngineQuotaRequest &that) const { return d_ptr != that.d_ptr; }

private:
    QWebEngineQuotaRequest(QSharedPointer<QtWebEngineCore::QuotaRequestController>);
    friend QtWebEngineCore::QuotaPermissionContextQt;
    QSharedPointer<QtWebEngineCore::QuotaRequestController> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEQUOTAREQUEST_H
