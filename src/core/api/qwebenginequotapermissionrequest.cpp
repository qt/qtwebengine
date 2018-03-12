/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qwebenginequotapermissionrequest.h"

#include "quota_permission_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineQuotaPermissionRequest
    \brief The QWebEngineQuotaPermissionRequest class enables accepting or rejecting
    requests for larger persistent storage than the application's current allocation
    in File System API.

    \since 5.11

    \inmodule QtWebEngineCore
*/

/*! \fn QWebEngineQuotaPermissionRequest::QWebEngineQuotaPermissionRequest()
    \internal
*/

/*! \internal */
QWebEngineQuotaPermissionRequest::QWebEngineQuotaPermissionRequest(QSharedPointer<QtWebEngineCore::QuotaPermissionController> controller)
    : d_ptr(controller)
{
}

/*!
    Rejects a request for larger persistent storage.
*/
void QWebEngineQuotaPermissionRequest::reject()
{
    d_ptr->reject();
}

/*!
    Accepts a request for larger persistent storage.
*/
void QWebEngineQuotaPermissionRequest::accept()
{
    d_ptr->accept();
}

/*!
    \property QWebEngineQuotaPermissionRequest::origin
    \brief The URL of the web page that issued the quota permission request.
*/

QUrl QWebEngineQuotaPermissionRequest::origin() const
{
    return d_ptr->origin();
}

/*!
    \property QWebEngineQuotaPermissionRequest::requestedSize
    \brief Contains the size of the requested disk space in bytes.
*/

qint64 QWebEngineQuotaPermissionRequest::requestedSize() const
{
    return d_ptr->requestedSize();
}

/*! \fn bool QWebEngineQuotaPermissionRequest::operator==(const QWebEngineQuotaPermissionRequest &that) const
    Returns \c true if \a that points to the same object as this quota
    permission request.
*/

/*! \fn bool QWebEngineQuotaPermissionRequest::operator!=(const QWebEngineQuotaPermissionRequest &that) const
    Returns \c true if \a that points to a different object than this quota
    permission request.
*/

QT_END_NAMESPACE
