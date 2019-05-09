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

#include "qwebengineclientcertificatestore.h"

#include "net/client_cert_store_data.h"

#include <QByteArray>
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
    The getInstance() method can be used to access the single instance of the class.
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

QVector<QSslCertificate> QWebEngineClientCertificateStore::certificates() const
{
    QVector<QSslCertificate> certificateList;
    for (auto data : qAsConst(m_storeData->extraCerts))
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
