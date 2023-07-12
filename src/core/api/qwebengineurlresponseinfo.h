// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEURLRESPONSEINFO_H
#define QWEBENGINEURLRESPONSEINFO_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/QUrl>
#include <QtCore/QHash>
#include <QtCore/QObject>

namespace QtWebEngineCore {
class InterceptedRequest;
}

QT_BEGIN_NAMESPACE

class QWebEngineUrlResponseInfoPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineUrlResponseInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl requestUrl READ requestUrl CONSTANT FINAL)
    Q_PROPERTY(QMultiHash<QByteArray, QByteArray> requestHeaders READ requestHeaders CONSTANT FINAL)
    Q_PROPERTY(QHash<QByteArray,QByteArray> responseHeaders READ responseHeaders WRITE
                       setResponseHeaders)

public:
    ~QWebEngineUrlResponseInfo() override;

    QUrl requestUrl() const;
    QMultiHash<QByteArray, QByteArray> requestHeaders() const;
    QHash<QByteArray, QByteArray> responseHeaders() const;

    void setResponseHeaders(const QHash<QByteArray, QByteArray> &newResponseHeaders);

private:
    friend class QtWebEngineCore::InterceptedRequest;
    Q_DECLARE_PRIVATE(QWebEngineUrlResponseInfo)
    QWebEngineUrlResponseInfoPrivate *d_ptr;

    explicit QWebEngineUrlResponseInfo(const QUrl &requestUrl,
                                       const QMultiHash<QByteArray, QByteArray> &requestHeaders,
                                       const QHash<QByteArray, QByteArray> &responseHeaders,
                                       QObject *p = nullptr);
};

QT_END_NAMESPACE

#endif // QWEBENGINEURLRESPONSEINFO_H
