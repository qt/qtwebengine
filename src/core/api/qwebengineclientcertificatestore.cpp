// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineclientcertificatestore.h"

#include "net/client_cert_store_data.h"

#include <QList>

QT_BEGIN_NAMESPACE

#if QT_CONFIG(ssl)

/*!
    \class QWebEngineClientCertificateStore
    \inmodule QtWebEngineCore
    \since 5.13
    \brief The QWebEngineClientCertificateStore class provides an in-memory store for client certificates.

    The class allows to store client certificates in an in-memory store.
    When a web site requests an SSL client certificate, the QWebEnginePage::selectClientCertificate
    signal is emitted with matching certificates from the native certificate store or the in-memory store.

    The class instance can be obtained with the QWebEngineProfile::clientCertificateStore() method.

    \code
    QFile certFile(":/resouces/certificate.crt");
    certFile.open(QIODevice::ReadOnly);
    const QSslCertificate cert(certFile.readAll(), QSsl::Pem);

    QFile keyFile(":/resources/privatekey.key");
    keyFile.open(QIODevice::ReadOnly);
    const QSslKey sslKey(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "");

    QWebEngineProfile profile;
    profile.clientCertificateStore()->add(cert, sslKey);
    \endcode
*/

QWebEngineClientCertificateStore::QWebEngineClientCertificateStore(QtWebEngineCore::ClientCertificateStoreData *storeData)
    : m_storeData(storeData)
{}

/*!
    Destroys this QWebEngineClientCertificateStore object.
*/

QWebEngineClientCertificateStore::~QWebEngineClientCertificateStore()
{
    // Just in case user has not deleted in-memory certificates
    clear();
}

/*!
    Adds a \a certificate with the \a privateKey to the in-memory client certificate store.
*/

void QWebEngineClientCertificateStore::add(const QSslCertificate &certificate, const QSslKey &privateKey)
{
    m_storeData->add(certificate, privateKey);
}

/*!
    Returns a list of the client certificates in the in-memory store.
    Returns an empty list if the store does not contain any certificates.
*/

QList<QSslCertificate> QWebEngineClientCertificateStore::certificates() const
{
    QList<QSslCertificate> certificateList;
    for (auto data : std::as_const(m_storeData->extraCerts))
        certificateList.append(data->certificate);
    return certificateList;
}

/*!
    Deletes all the instances of the client certificate in the in-memory client certificate store
    that matches the certificate \a certificate.
*/

void QWebEngineClientCertificateStore::remove(const QSslCertificate &certificate)
{
    m_storeData->remove(certificate);
}

/*!
    Clears all the client certificates from the in-memory store.
*/

void QWebEngineClientCertificateStore::clear()
{
    m_storeData->clear();
}

#endif // QT_CONFIG(ssl)

QT_END_NAMESPACE
