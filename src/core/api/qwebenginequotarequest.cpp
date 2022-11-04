// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginequotarequest.h"

#include "quota_request_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineQuotaRequest
    \brief The QWebEngineQuotaRequest class enables accepting or rejecting
    requests for larger persistent storage than the application's current
    allocation in File System API.

    \since 5.11
    \inmodule QtWebEngineCore

    This class is used by the QWebEnginePage::quotaRequested() signal to \l
    accept() or \l reject() a request for an increase in the persistent storage
    allocated to the application. The default quota is 0 bytes.
*/

/*! \fn QWebEngineQuotaRequest::QWebEngineQuotaRequest()
    \internal
*/

/*! \internal */
QWebEngineQuotaRequest::QWebEngineQuotaRequest(QSharedPointer<QtWebEngineCore::QuotaRequestController> controller)
    : d_ptr(controller)
{}

/*!
    Rejects a request for larger persistent storage.
*/
void QWebEngineQuotaRequest::reject()
{
    d_ptr->reject();
}

/*!
    Accepts a request for larger persistent storage.
*/
void QWebEngineQuotaRequest::accept()
{
    d_ptr->accept();
}

/*!
    \property QWebEngineQuotaRequest::origin
    \brief The URL of the web page that issued the quota request.
*/

QUrl QWebEngineQuotaRequest::origin() const
{
    return d_ptr->origin();
}

/*!
    \property QWebEngineQuotaRequest::requestedSize
    \brief Contains the size of the requested disk space in bytes.
*/

qint64 QWebEngineQuotaRequest::requestedSize() const
{
    return d_ptr->requestedSize();
}

/*! \fn bool QWebEngineQuotaRequest::operator==(const QWebEngineQuotaRequest &that) const
    Returns \c true if \a that points to the same object as this quota request.
*/

/*! \fn bool QWebEngineQuotaRequest::operator!=(const QWebEngineQuotaRequest &that) const
    Returns \c true if \a that points to a different object than this request.
*/

QT_END_NAMESPACE
#include "moc_qwebenginequotarequest.cpp"
