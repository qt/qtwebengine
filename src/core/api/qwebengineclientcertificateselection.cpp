// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineclientcertificateselection.h"
#include "client_cert_select_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineClientCertificateSelection
    \brief The QWebEngineClientCertSelection class wraps a client certificate selection.
    \since 5.12
    \inmodule QtWebEngineCore

    When a web site requests an SSL client certificate, and one or more certificates
    are found in the system's client certificate store, this class provides access to
    the certificates to choose from, as well as a method for selecting one.

    The selection is asynchronous. If no certificate is selected and no copy of the
    object is kept alive, loading will continue without a certificate.

    \sa QWebEnginePage::selectClientCertificate()
*/

/*! \internal
*/
QWebEngineClientCertificateSelection::QWebEngineClientCertificateSelection(
        QSharedPointer<QtWebEngineCore::ClientCertSelectController> selectController)
    : d_ptr(selectController)
{}

QWebEngineClientCertificateSelection::QWebEngineClientCertificateSelection(const QWebEngineClientCertificateSelection &other)
        : d_ptr(other.d_ptr)
{}

QWebEngineClientCertificateSelection &QWebEngineClientCertificateSelection::operator=(const QWebEngineClientCertificateSelection &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

QWebEngineClientCertificateSelection::~QWebEngineClientCertificateSelection()
{
}

/*!
    Returns the client certificates available to choose from.

    \sa select()
*/
QList<QSslCertificate> QWebEngineClientCertificateSelection::certificates() const
{
    return d_ptr->certificates();
}

/*!
    Selects the client certificate \a certificate. The certificate must be one
    of those offered in certificates().

    \sa certificates(), selectNone()
*/
void QWebEngineClientCertificateSelection::select(const QSslCertificate &certificate)
{
    d_ptr->select(certificate);
}

/*!
    Continue without using any of the offered certificates. This is the same
    action as taken when destroying the last copy of this object if no
    selection has been made.

    \sa select()
*/
void QWebEngineClientCertificateSelection::selectNone()
{
    d_ptr->selectNone();
}

/*!
    Returns the host and port of the server requesting the client certificate.
*/
QUrl QWebEngineClientCertificateSelection::host() const
{
    return d_ptr->hostAndPort();
}

QT_END_NAMESPACE

