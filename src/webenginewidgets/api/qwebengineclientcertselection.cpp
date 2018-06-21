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

#include "qwebengineclientcertselection.h"

#if QT_CONFIG(ssl)

#include "client_cert_select_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineClientCertSelection
    \brief The QWebEngineClientCertSelection class wraps a client certificate selection.
    \since 5.12
    \inmodule QtWebEngineWidgets

    Provides access to the certicates to choose from, and a method for selecting one.

    The selection is asynchronous. If no certificate is selected and no copy of the
    object is kept alive, loading will continue without a certificate.
*/

/*! \internal
*/
QWebEngineClientCertSelection::QWebEngineClientCertSelection(QSharedPointer<ClientCertSelectController> selectController)
        : d_ptr(selectController)
{}

QWebEngineClientCertSelection::QWebEngineClientCertSelection(const QWebEngineClientCertSelection &other)
        : d_ptr(other.d_ptr)
{}

QWebEngineClientCertSelection &QWebEngineClientCertSelection::operator=(const QWebEngineClientCertSelection &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

QWebEngineClientCertSelection::~QWebEngineClientCertSelection()
{
}

/*!
    Returns the client certificates available to choose from.

    \sa select()
*/
QVector<QSslCertificate> QWebEngineClientCertSelection::certificates() const
{
    return d_ptr->certificates();
}

/*!
    Selects the client certificate \a certificate. The certificate must be one
    of those offered in certificates().

    \sa certificates(), selectNone()
*/
void QWebEngineClientCertSelection::select(const QSslCertificate &certificate)
{
    d_ptr->select(certificate);
}

/*!
    Continue without using any of the offered certificates. This is the same
    action as taken when destroying the last copy of this object if no
    selection has been made.

    \sa select()
*/
void QWebEngineClientCertSelection::selectNone()
{
    d_ptr->selectNone();
}

/*!
    Returns the host and port of the server requesting the client certificate.
*/
QUrl QWebEngineClientCertSelection::host() const
{
    return d_ptr->hostAndPort();
}

QT_END_NAMESPACE

#endif // QT_CONFIG(ssl)
