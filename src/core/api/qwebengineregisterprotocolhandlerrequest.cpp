// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineregisterprotocolhandlerrequest.h"

#include "custom_handlers/register_protocol_handler_request_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineRegisterProtocolHandlerRequest
    \inmodule QtWebEngineCore
    \since 5.11
    \brief The QWebEngineRegisterProtocolHandlerRequest class enables
    accepting or rejecting requests from the \l registerProtocolHandler API.

    \sa QWebEnginePage::registerProtocolHandlerRequested()
*/

/*! \fn QWebEngineRegisterProtocolHandlerRequest::QWebEngineRegisterProtocolHandlerRequest()
    \internal
*/

/*! \internal */
QWebEngineRegisterProtocolHandlerRequest::QWebEngineRegisterProtocolHandlerRequest(
        QSharedPointer<QtWebEngineCore::RegisterProtocolHandlerRequestController> d_ptr)
    : d_ptr(std::move(d_ptr))
{}

/*!
    Rejects the request.

    Subsequent calls to accept() and reject() are ignored.
*/
void QWebEngineRegisterProtocolHandlerRequest::reject()
{
    d_ptr->reject();
}

/*!
    Accepts the request

    Subsequent calls to accept() and reject() are ignored.
*/
void QWebEngineRegisterProtocolHandlerRequest::accept()
{
    d_ptr->accept();
}

/*!
    \property QWebEngineRegisterProtocolHandlerRequest::origin
    \brief The URL template for the protocol handler.

    This is the second parameter from the \l registerProtocolHandler call.
*/
QUrl QWebEngineRegisterProtocolHandlerRequest::origin() const
{
    return d_ptr->origin();
}

/*!
    \property QWebEngineRegisterProtocolHandlerRequest::scheme
    \brief The URL scheme for the protocol handler.

    This is the first parameter from the \l registerProtocolHandler call.
*/
QString QWebEngineRegisterProtocolHandlerRequest::scheme() const
{
    return d_ptr->scheme();
}

/*! \fn bool QWebEngineRegisterProtocolHandlerRequest::operator==(const QWebEngineRegisterProtocolHandlerRequest &that) const
    Returns \c true if \a that points to the same object as this request.
*/

/*! \fn bool QWebEngineRegisterProtocolHandlerRequest::operator!=(const QWebEngineRegisterProtocolHandlerRequest &that) const
    Returns \c true if \a that points to a different object than this request.
*/

QT_END_NAMESPACE

#include "moc_qwebengineregisterprotocolhandlerrequest.cpp"
