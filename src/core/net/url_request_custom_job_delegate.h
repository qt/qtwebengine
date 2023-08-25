// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#ifndef URL_REQUEST_CUSTOM_JOB_DELEGATE_H_
#define URL_REQUEST_CUSTOM_JOB_DELEGATE_H_

#include "base/memory/ref_counted.h"
#include "qtwebenginecoreglobal_p.h"

#include <QMap>
#include <QObject>
#include <QUrl>

QT_FORWARD_DECLARE_CLASS(QIODevice)

namespace QtWebEngineCore {

class URLRequestCustomJobProxy;

class Q_WEBENGINECORE_PRIVATE_EXPORT URLRequestCustomJobDelegate : public QObject
{
    Q_OBJECT
public:
    ~URLRequestCustomJobDelegate();

    enum Error {
        NoError = 0,
        UrlNotFound,
        UrlInvalid,
        RequestAborted,
        RequestDenied,
        RequestFailed
    };

    QUrl url() const;
    QByteArray method() const;
    QUrl initiator() const;
    QMap<QByteArray, QByteArray> requestHeaders() const;

    void
    setAdditionalResponseHeaders(const QMultiMap<QByteArray, QByteArray> &additionalResponseHeaders);
    void reply(const QByteArray &contentType, QIODevice *device);
    void redirect(const QUrl &url);
    void abort();
    void fail(Error);

private Q_SLOTS:
    void slotReadyRead();

private:
    URLRequestCustomJobDelegate(URLRequestCustomJobProxy *proxy,
                                const QUrl &url,
                                const QByteArray &method,
                                const QUrl &initiatorOrigin,
                                const QMap<QByteArray, QByteArray> &requestHeaders);

    friend class URLRequestCustomJobProxy;
    scoped_refptr<URLRequestCustomJobProxy> m_proxy;
    QUrl m_request;
    QByteArray m_method;
    QUrl m_initiatorOrigin;
    const QMap<QByteArray, QByteArray> m_requestHeaders;
    QMultiMap<QByteArray, QByteArray> m_additionalResponseHeaders;
};

} // namespace

#endif // URL_REQUEST_CUSTOM_JOB_DELEGATE_H_
