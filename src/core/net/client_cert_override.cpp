// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "client_cert_override.h"

#include "base/bind.h"
#include "base/task/post_task.h"
#include "base/callback_forward.h"
#include "content/public/browser/browser_task_traits.h"
#include "crypto/crypto_buildflags.h"
#include "net/ssl/client_cert_store.h"
#include "net/ssl/ssl_cert_request_info.h"
#include "net/ssl/ssl_private_key.h"
#include "net/cert/x509_certificate.h"
#include "third_party/boringssl/src/include/openssl/pem.h"
#include "third_party/boringssl/src/include/openssl/err.h"
#include "third_party/boringssl/src/include/openssl/evp.h"

#include "client_cert_store_data.h"
#include "profile_io_data_qt.h"

#include <QtNetwork/qtnetworkglobal.h>

#if BUILDFLAG(USE_NSS_CERTS)
#include "net/ssl/client_cert_store_nss.h"
#endif

#if defined(Q_OS_WIN)
#include "net/ssl/client_cert_store_win.h"
#endif

#if BUILDFLAG(IS_MAC)
#include "net/ssl/client_cert_store_mac.h"
#endif

namespace {

class ClientCertIdentityOverride : public net::ClientCertIdentity
{
public:
    ClientCertIdentityOverride(scoped_refptr<net::X509Certificate> cert, scoped_refptr<net::SSLPrivateKey> key)
            : net::ClientCertIdentity(std::move(cert)), m_key(std::move(key)) {}
    ~ClientCertIdentityOverride() override = default;

    void AcquirePrivateKey(base::OnceCallback<void(scoped_refptr<net::SSLPrivateKey>)> private_key_callback) override
    {
        std::move(private_key_callback).Run(m_key);
    }

private:
    scoped_refptr<net::SSLPrivateKey> m_key;
};

} // namespace

namespace QtWebEngineCore {

ClientCertOverrideStore::ClientCertOverrideStore(ClientCertificateStoreData *storeData)
    : ClientCertStore()
    , m_storeData(storeData)
    , m_nativeStore(createNativeStore())
{
}

ClientCertOverrideStore::~ClientCertOverrideStore() = default;

#if QT_CONFIG(ssl)
net::ClientCertIdentityList ClientCertOverrideStore::GetClientCertsOnUIThread(const net::SSLCertRequestInfo &cert_request_info)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    const auto &clientCertOverrideData = m_storeData->extraCerts;
    // Look for certificates in memory store
    for (int i = 0; i < clientCertOverrideData.length(); i++) {
        scoped_refptr<net::X509Certificate> cert = clientCertOverrideData[i]->certPtr;
        if (cert != NULL && cert->IsIssuedByEncoded(cert_request_info.cert_authorities)) {
            net::ClientCertIdentityList selected_identities;
            selected_identities.push_back(std::make_unique<ClientCertIdentityOverride>(cert, clientCertOverrideData[i]->keyPtr));
            return selected_identities;
        }
    }
    return net::ClientCertIdentityList();
}

void ClientCertOverrideStore::GetClientCertsReturn(const net::SSLCertRequestInfo &cert_request_info,
                                                   ClientCertListCallback callback,
                                                   net::ClientCertIdentityList &&result)
{
    // Continue with native cert store if matching certificatse were not found in memory
    if (result.empty() && m_nativeStore)
        m_nativeStore->GetClientCerts(cert_request_info, std::move(callback));
    else
        std::move(callback).Run(std::move(result));
}

#endif // QT_CONFIG(ssl)

void ClientCertOverrideStore::GetClientCerts(const net::SSLCertRequestInfo &cert_request_info,
                                             ClientCertListCallback callback)
{
#if QT_CONFIG(ssl)
    // Access the user-provided data from the UI thread, but return on whatever thread this is.
    bool ok = base::PostTaskAndReplyWithResult(
            FROM_HERE, { content::BrowserThread::UI },
            base::BindOnce(&ClientCertOverrideStore::GetClientCertsOnUIThread,
                           base::Unretained(this), std::cref(cert_request_info)),
            base::BindOnce(&ClientCertOverrideStore::GetClientCertsReturn,
                           base::Unretained(this), std::cref(cert_request_info), std::move(callback)));
    DCHECK(ok); // callback is already moved and we can't really recover here.
#else
    if (m_nativeStore)
        m_nativeStore->GetClientCerts(cert_request_info, std::move(callback));
    else
        std::move(callback).Run(net::ClientCertIdentityList());
#endif // QT_CONFIG(ssl)
}

// static
std::unique_ptr<net::ClientCertStore> ClientCertOverrideStore::createNativeStore()
{
#if BUILDFLAG(USE_NSS_CERTS)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreNSS(net::ClientCertStoreNSS::PasswordDelegateFactory()));
#elif defined(Q_OS_WIN)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreWin());
#elif BUILDFLAG(IS_MAC)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreMac());
#else
    return nullptr;
#endif
}
} // namespace QtWebEngineCore
