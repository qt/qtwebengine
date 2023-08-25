// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEURLREQUESTJOB_H
#define QWEBENGINEURLREQUESTJOB_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qobject.h>
#include <QtCore/qurl.h>

namespace QtWebEngineCore {
class URLRequestCustomJobDelegate;
class URLRequestCustomJobProxy;
} // namespace QtWebEngineCore

QT_BEGIN_NAMESPACE

class QIODevice;

class Q_WEBENGINECORE_EXPORT QWebEngineUrlRequestJob : public QObject
{
    Q_OBJECT
public:
    ~QWebEngineUrlRequestJob();

    enum Error {
        NoError = 0,
        UrlNotFound,
        UrlInvalid,
        RequestAborted,
        RequestDenied,
        RequestFailed
    };
    Q_ENUM(Error)

    QUrl requestUrl() const;
    QByteArray requestMethod() const;
    QUrl initiator() const;
    QMap<QByteArray, QByteArray> requestHeaders() const;

    void reply(const QByteArray &contentType, QIODevice *device);
    void fail(Error error);
    void redirect(const QUrl &url);
    void setAdditionalResponseHeaders(
            const QMultiMap<QByteArray, QByteArray> &additionalResponseHeaders) const;

private:
    QWebEngineUrlRequestJob(QtWebEngineCore::URLRequestCustomJobDelegate *);
    friend class QtWebEngineCore::URLRequestCustomJobProxy;

    QtWebEngineCore::URLRequestCustomJobDelegate *d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEURLREQUESTJOB_H
