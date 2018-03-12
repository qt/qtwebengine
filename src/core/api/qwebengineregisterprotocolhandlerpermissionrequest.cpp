/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qwebengineregisterprotocolhandlerpermissionrequest.h"

#include "register_protocol_handler_permission_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineRegisterProtocolHandlerPermissionRequest
    \inmodule QtWebEngineCore
    \since 5.11
    \brief The QWebEngineRegisterProtocolHandlerPermissionRequest type enables
    accepting or rejecting requests from the \l registerProtocolHandler API.

    \sa QWebEnginePage::registerProtocolHandlerPermissionRequested
*/

/*! \fn QWebEngineRegisterProtocolHandlerPermissionRequest::QWebEngineRegisterProtocolHandlerPermissionRequest()
    \internal
*/

/*! \internal */
QWebEngineRegisterProtocolHandlerPermissionRequest::QWebEngineRegisterProtocolHandlerPermissionRequest(
    QSharedPointer<QtWebEngineCore::RegisterProtocolHandlerPermissionController> d_ptr)
    : d_ptr(std::move(d_ptr))
{}

/*!
    Rejects the request.

    Subsequent calls to accept() and reject() are ignored.
*/
void QWebEngineRegisterProtocolHandlerPermissionRequest::reject()
{
    d_ptr->reject();
}

/*!
    Accepts the request

    Subsequent calls to accept() and reject() are ignored.
*/
void QWebEngineRegisterProtocolHandlerPermissionRequest::accept()
{
    d_ptr->accept();
}

/*!
    \property QWebEngineRegisterProtocolHandlerPermissionRequest::origin
    \brief The URL template for the protocol handler.

    This is the second parameter from the \l registerProtocolHandler call.
*/
QUrl QWebEngineRegisterProtocolHandlerPermissionRequest::origin() const
{
    return d_ptr->origin();
}

/*!
    \property QWebEngineRegisterProtocolHandlerPermissionRequest::scheme
    \brief The URL scheme for the protocol handler.

    This is the first parameter from the \l registerProtocolHandler call.
*/
QString QWebEngineRegisterProtocolHandlerPermissionRequest::scheme() const
{
    return d_ptr->scheme();
}

/*! \fn bool QWebEngineRegisterProtocolHandlerPermissionRequest::operator==(const QWebEngineRegisterProtocolHandlerPermissionRequest &that) const
    Returns \c true if \a that points to the same object as this protocol
    handler permission request.
*/

/*! \fn bool QWebEngineRegisterProtocolHandlerPermissionRequest::operator!=(const QWebEngineRegisterProtocolHandlerPermissionRequest &that) const
    Returns \c true if \a that points to a different object than this protocol
    handler permission request.
*/

QT_END_NAMESPACE
