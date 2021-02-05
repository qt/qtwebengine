/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qwebengineloadrequest.h>

#include <web_engine_error.h>

QT_BEGIN_NAMESPACE

using LoadStatus = QWebEngineLoadRequest::LoadStatus;
using ErrorDomain = QWebEngineLoadRequest::ErrorDomain;

Q_STATIC_ASSERT(static_cast<int>(WebEngineError::NoErrorDomain) == static_cast<int>(ErrorDomain::NoErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(WebEngineError::InternalErrorDomain) == static_cast<int>(ErrorDomain::InternalErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(WebEngineError::ConnectionErrorDomain) == static_cast<int>(ErrorDomain::ConnectionErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(WebEngineError::CertificateErrorDomain) == static_cast<int>(ErrorDomain::CertificateErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(WebEngineError::HttpErrorDomain) == static_cast<int>(ErrorDomain::HttpErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(WebEngineError::FtpErrorDomain) == static_cast<int>(ErrorDomain::FtpErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(WebEngineError::DnsErrorDomain) == static_cast<int>(ErrorDomain::DnsErrorDomain));

class QWebEngineLoadRequest::QWebEngineLoadRequestPrivate : public QSharedData {
public:
    QWebEngineLoadRequestPrivate(const QUrl& url, LoadStatus status, const QString& errorString, int errorCode, ErrorDomain errorDomain)
        : url(url)
        , status(status)
        , errorString(errorString)
        , errorCode(errorCode)
        , errorDomain(errorDomain)
    {
    }

    QUrl url;
    LoadStatus status;
    QString errorString;
    int errorCode;
    ErrorDomain errorDomain;
};

/*!
    \class QWebEngineLoadRequest
    \brief A utility type for the WebEngineView::loadingChanged signal.
    \inmodule QtWebEngineCore
    \since 6.2

    Contains information about a web page loading status change, such as the URL and
    current loading status (started, succeeded, stopped, failed).

    \sa QWebEnginePage::loadStarted, QWebEnginePage::loadFinished, WebEngineView::loadingChanged
*/
QWebEngineLoadRequest::QWebEngineLoadRequest(const QUrl& url, LoadStatus status, const QString& errorString,
                                           int errorCode, ErrorDomain errorDomain)
    : d_ptr(new QWebEngineLoadRequestPrivate(url, status, errorString, errorCode, errorDomain))
{
}

QWebEngineLoadRequest::QWebEngineLoadRequest(const QWebEngineLoadRequest &other) = default;
QWebEngineLoadRequest& QWebEngineLoadRequest::operator=(const QWebEngineLoadRequest &other) = default;
QWebEngineLoadRequest::QWebEngineLoadRequest(QWebEngineLoadRequest &&other) = default;
QWebEngineLoadRequest& QWebEngineLoadRequest::operator=(QWebEngineLoadRequest &&other) = default;

QWebEngineLoadRequest::~QWebEngineLoadRequest()
{
}
/*!
    \property QWebEngineLoadRequest::url
    \brief Holds the URL of the load request.
*/
/*!
    Returns the URL of the load request.
*/
QUrl QWebEngineLoadRequest::url() const
{
    Q_D(const QWebEngineLoadRequest);
    return d->url;
}
/*!
    \enum QWebEngineLoadRequest::status

    This enumeration represents the load status of a web page load request:

    \value  LoadStartedStatus Page is currently loading.
    \value  LoadStoppedStatus
            Loading the page was stopped by the stop() method or by the loader
            code or network stack in Chromium.
    \value  LoadSucceededStatus Page has been loaded with success.
    \value  LoadFailedStatus Page could not be loaded.
*/
/*!
    \property Holds the page's load status.
*/
/*!
    Returns the page's load status.
*/
LoadStatus QWebEngineLoadRequest::status() const
{
    Q_D(const QWebEngineLoadRequest);
    return d->status;
}
/*!
    \property QWebEngineLoadRequest::errorString
    \brief Holds the error message.
*/
/*
    Returns the error message.
*/
QString QWebEngineLoadRequest::errorString() const
{
    Q_D(const QWebEngineLoadRequest);
    return d->errorString;
}
/*!
    \enum enumeration QWebEngineLoadRequest::errorDomain
    This enumeration holds the type of a load error:

    \value  NoErrorDomain
            Error type is not known.
    \value  InternalErrorDomain
            Content cannot be interpreted by \QWE.
    \value  ConnectionErrorDomain
            Error results from a faulty network connection.
    \value  CertificateErrorDomain
            Error is related to the SSL/TLS certificate.
    \value  HttpErrorDomain
            Error is related to the HTTP connection.
    \value  FtpErrorDomain
            Error is related to the FTP connection.
    \value  DnsErrorDomain
            Error is related to the DNS connection.
*/
/*
    \property QWebEngineLoadRequest::errorDomain
    \brief Holds the error domain
*/
/*
    Returns the error domain.
*/
ErrorDomain QWebEngineLoadRequest::errorDomain() const
{
    Q_D(const QWebEngineLoadRequest);
    return d->errorDomain;
}

/*!
    \property int QWebEngineLoadRequest::errorCode
    \brief Holds the error code.
*/
/*
    Returns the error code.
*/
int QWebEngineLoadRequest::errorCode() const
{
    Q_D(const QWebEngineLoadRequest);
    return d->errorCode;
}

QT_END_NAMESPACE
