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

#include "api/qwebengineclientcertificatestore.h"
#include "client_cert_override_key_p.h"
#include "client_cert_override_p.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/task_scheduler/post_task.h"
#include "base/callback_forward.h"

#include "net/ssl/client_cert_store.h"
#include "net/ssl/ssl_cert_request_info.h"
#include "net/cert/x509_certificate.h"

#include "third_party/boringssl/src/include/openssl/pem.h"
#include "third_party/boringssl/src/include/openssl/err.h"
#include "third_party/boringssl/src/include/openssl/evp.h"

#if defined(USE_NSS_CERTS)
#include "net/ssl/client_cert_store_nss.h"
#endif

#if defined(OS_WIN)
#include "net/ssl/client_cert_store_win.h"
#endif

#if defined(OS_MACOSX)
#include "net/ssl/client_cert_store_mac.h"
#endif

#include <QByteArray>
#include <QList>

QT_BEGIN_NAMESPACE

typedef struct OverrideData {
    QSslKey key;
    QSslCertificate certificate;
    scoped_refptr<net::X509Certificate> certPtr;
    scoped_refptr<net::SSLPrivateKey> keyPtr;
} OverrideData;

struct QWebEngineClientCertificateStoreData {
    QList<OverrideData*> deletedCerts;
};

static QList<OverrideData*> ClientCertOverrideData;
QWebEngineClientCertificateStore *QWebEngineClientCertificateStore::m_instance = NULL;

/*!
    \class QWebEngineClientCertificateStore::Entry
    \inmodule QtWebEngineCore
    \since 5.13
    \brief This structure holds the certificate and the private key.
*/

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

QWebEngineClientCertificateStore::QWebEngineClientCertificateStore()
{
    this->d_ptr = new QWebEngineClientCertificateStoreData;
}

/*!
    Destroys this QWebEngineClientCertificateStore object.
*/

QWebEngineClientCertificateStore::~QWebEngineClientCertificateStore()
{
    // Just in case user has not deleted in-memory certificates
    clear();

    qDeleteAll(d_ptr->deletedCerts);
    delete d_ptr;
}

/*!
    Returns an in-memory client certificate store.
*/

QWebEngineClientCertificateStore *QWebEngineClientCertificateStore::getInstance()
{
    if (!m_instance)
        m_instance = new QWebEngineClientCertificateStore;
    return m_instance;
}

/*!
    Adds a \a certificate with the \a privateKey to the in-memory client certificate store.
*/

void QWebEngineClientCertificateStore::add(const QSslCertificate &certificate, const QSslKey &privateKey)
{

    QByteArray sslKeyInBytes = privateKey.toPem();
    QByteArray certInBytes = certificate.toDer();

    OverrideData* data = new OverrideData;
    data->keyPtr = net::WrapOpenSSLPrivateKey(sslKeyInBytes);
    data->certPtr = net::X509Certificate::CreateFromBytes(
                certInBytes.data(), certInBytes.length());
    data->key = privateKey;
    data->certificate = certificate;
    ClientCertOverrideData.append(data);
}

/*!
    Returns a list of private and public keys of client certificates in the in-memory store.
    Returns an empty list if the in-memory store does not contain certificates.
*/

QList<QWebEngineClientCertificateStore::Entry> QWebEngineClientCertificateStore::toList() const
{
    QList<Entry> certificateList;
    for (auto data : ClientCertOverrideData) {
        Entry entry;
        entry.certificate = data->certificate;
        entry.privateKey = data->key;
        certificateList.append(entry);
    }
    return certificateList;
}

/*!
    Deletes all the instances of the client certificate in the in-memory client certificate store
    that matches the certificate in the \a entry.
*/

void QWebEngineClientCertificateStore::remove(Entry entry)
{
    QMutableListIterator<OverrideData*> iterator(ClientCertOverrideData);
    while (iterator.hasNext()) {
        auto overrideData = iterator.next();
        if (entry.certificate.toDer() == overrideData->certificate.toDer()) {
            d_ptr->deletedCerts.append(overrideData);
            iterator.remove();
        }
    }
}

/*!
    Clears all the client certificates from the in-memory store.
*/

void QWebEngineClientCertificateStore::clear()
{
    for (auto data : ClientCertOverrideData)
        d_ptr->deletedCerts.append(data);
    ClientCertOverrideData.clear();
}

QT_END_NAMESPACE

namespace net {

namespace {

class ClientCertIdentityOverride : public ClientCertIdentity {
public:
    ClientCertIdentityOverride(
            scoped_refptr<net::X509Certificate> cert,
            scoped_refptr<net::SSLPrivateKey> key)
        : ClientCertIdentity(std::move(cert)),
          key_(std::move(key)) {}
    ~ClientCertIdentityOverride() override = default;

    void AcquirePrivateKey(
            const base::Callback<void(scoped_refptr<SSLPrivateKey>)>&
            private_key_callback) override
    {
        private_key_callback.Run(key_);
    }

#if defined(OS_MACOSX)
    SecIdentityRef sec_identity_ref() const override
    {
        return nullptr;
    }
#endif

private:
    scoped_refptr<net::SSLPrivateKey> key_;
};

}  // namespace


ClientCertOverrideStore::ClientCertOverrideStore()
    : ClientCertStore()
{
}

ClientCertOverrideStore::~ClientCertOverrideStore()
{
}

void ClientCertOverrideStore::GetClientCerts(const SSLCertRequestInfo &cert_request_info,
                                             const ClientCertListCallback &callback)
{
    // Look for certificates in memory store
    for (int i = 0; i < ClientCertOverrideData.length(); i++) {
        scoped_refptr<net::X509Certificate> cert = ClientCertOverrideData[i]->certPtr;
        if (cert != NULL && cert->IsIssuedByEncoded(cert_request_info.cert_authorities)) {
            ClientCertIdentityList selected_identities;
            selected_identities.push_back(std::make_unique<ClientCertIdentityOverride>(cert, ClientCertOverrideData[i]->keyPtr));
            callback.Run(std::move(selected_identities));
            return;
        }
    }

    // Continue with native cert store if matching certificate is not found in memory
    std::unique_ptr<net::ClientCertStore> store = getNativeStore();
    if (store != NULL) {
        store->GetClientCerts(cert_request_info, callback);
        return;
    }

    callback.Run(ClientCertIdentityList());
    return;
}

std::unique_ptr<net::ClientCertStore> ClientCertOverrideStore::getNativeStore()
{
#if defined(USE_NSS_CERTS)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreNSS(net::ClientCertStoreNSS::PasswordDelegateFactory()));
#elif defined(OS_WIN)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreWin());
#elif defined(OS_MACOSX)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreMac());
#else
    return nullptr;
#endif
}
}
