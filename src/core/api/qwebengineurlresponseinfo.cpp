// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineurlresponseinfo.h"
#include "qwebengineurlresponseinfo_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineUrlResponseInfo
    \brief A utility type for the QWebEngineUrlResponseInterceptor.
    \inmodule QtWebEngineCore
    \since 6.6

    Contains information about the request that has caused the response
    intercepted by a QWebEngineUrlResponseInterceptor.

    \sa QWebEngineUrlResponseInterceptor
*/
QWebEngineUrlResponseInfo::QWebEngineUrlResponseInfo(
        const QUrl &requestUrl, const QMultiHash<QByteArray, QByteArray> &requestHeaders,
        const QHash<QByteArray, QByteArray> &responseHeaders, QObject *p)
    : QObject(p)
    , d_ptr(new QWebEngineUrlResponseInfoPrivate(requestUrl, requestHeaders, responseHeaders))
{
}

QWebEngineUrlResponseInfo::~QWebEngineUrlResponseInfo(){};

/*!
    \property QWebEngineUrlResponseInfo::requestUrl
    \brief Holds the URL of the URL load request.
*/
QUrl QWebEngineUrlResponseInfo::requestUrl() const
{
    Q_D(const QWebEngineUrlResponseInfo);
    return d->requestUrl;
}

/*!
    \property QWebEngineUrlResponseInfo::requestHeaders
    \brief Holds the request headers of the URL load request.
*/
QMultiHash<QByteArray, QByteArray> QWebEngineUrlResponseInfo::requestHeaders() const
{
    Q_D(const QWebEngineUrlResponseInfo);
    return d->requestHeaders;
}

/*!
    \property QWebEngineUrlResponseInfo::responseHeaders
    \brief Holds the response headers of the URL load request.
*/
QHash<QByteArray, QByteArray> QWebEngineUrlResponseInfo::responseHeaders() const
{
    Q_D(const QWebEngineUrlResponseInfo);
    return d->responseHeaders;
}

/*!
    \fn void QWebEngineUrlResponseInfo::setResponseHeaders(
        const QMultiMap<QByteArray, QByteArray> &newResponseHeaders)
    \brief Sets the response headers to \a newResponseHeaders.

    Sets the response headers to \a newResponseHeaders. If \a newResponseHeaders
    differ from the current response headers then
    QWebEngineUrlResponseInfo::isModified() will now return \c true.
*/
void QWebEngineUrlResponseInfo::setResponseHeaders(
        const QHash<QByteArray, QByteArray> &newResponseHeaders)
{
    Q_D(QWebEngineUrlResponseInfo);
    if (d->responseHeaders != newResponseHeaders) {
        d->responseHeaders = newResponseHeaders;
        d->isModified = true;
    }
}

QT_END_NAMESPACE

#include "moc_qwebengineurlresponseinfo.cpp"
