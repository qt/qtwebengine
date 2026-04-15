// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "client_cert_qt.h"

#include "base/functional/bind.h"
#include "base/functional/callback_forward.h"
#include "content/public/browser/browser_thread.h"
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

#include <QtNetwork/qtnetworkglobal.h>

#if BUILDFLAG(USE_NSS_CERTS)
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread_restrictions.h"
#include "crypto/nss_crypto_module_delegate.h"
#include "net/ssl/client_cert_store_nss.h"
#include "profile_qt.h"
#endif

#if defined(Q_OS_WIN)
#include "net/ssl/client_cert_store_win.h"
#endif

#if BUILDFLAG(IS_MAC)
#include "net/ssl/client_cert_store_mac.h"
#endif

namespace {

class ClientCertIdentityQt : public net::ClientCertIdentity
{
public:
    ClientCertIdentityQt(scoped_refptr<net::X509Certificate> cert, scoped_refptr<net::SSLPrivateKey> key)
            : net::ClientCertIdentity(std::move(cert)), m_key(std::move(key)) {}
    ~ClientCertIdentityQt() override = default;

    void AcquirePrivateKey(base::OnceCallback<void(scoped_refptr<net::SSLPrivateKey>)> private_key_callback) override
    {
        std::move(private_key_callback).Run(m_key);
    }

private:
    scoped_refptr<net::SSLPrivateKey> m_key;
};

} // namespace

namespace QtWebEngineCore {

ClientCertStoreQt::ClientCertStoreQt(ClientCertificateStoreData *storeData, content::BrowserContext *browser_context)
    : ClientCertStore()
    , m_profile(browser_context)
    , m_storeData(storeData)
    , m_nativeStore(createNativeStore())
{
}

ClientCertStoreQt::~ClientCertStoreQt() = default;

#if QT_CONFIG(ssl)
net::ClientCertIdentityList ClientCertStoreQt::GetClientCertsOnUIThread(scoped_refptr<const net::SSLCertRequestInfo> cert_request_info)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    const auto &clientCertOverrideData = m_storeData->extraCerts;

    // Look for certificates in memory store
    net::ClientCertIdentityList selected_identities;
    for (int i = 0; i < clientCertOverrideData.length(); i++) {
        scoped_refptr<net::X509Certificate> cert = clientCertOverrideData[i]->certPtr;
        if (cert) {
            if (cert->HasExpired()) {
                qWarning() << "Expired certificate" << clientCertOverrideData[i];
                continue;
            }
            if (cert_request_info->cert_authorities.empty()
                || cert->IsIssuedByEncoded(cert_request_info->cert_authorities)) {
                selected_identities.push_back(std::make_unique<ClientCertIdentityQt>(
                        cert, clientCertOverrideData[i]->keyPtr));
            }
        }
    }
    return selected_identities;
}

void ClientCertStoreQt::GetClientCertsReturn(scoped_refptr<const net::SSLCertRequestInfo> cert_request_info,
                                             ClientCertListCallback callback,
                                             net::ClientCertIdentityList &&result)
{
    // Continue with native cert store and append them after memory certificates
    if (m_nativeStore) {
        ClientCertListCallback callback2 = base::BindOnce(
                [](ClientCertStoreQt::ClientCertListCallback callback,
                   net::ClientCertIdentityList result1, net::ClientCertIdentityList result2) {
                    while (!result2.empty()) {
                        result1.push_back(std::move(result2.back()));
                        result2.pop_back();
                    }
                    std::move(callback).Run(std::move(result1));
                },
                std::move(callback), std::move(result));
        m_nativeStore->GetClientCerts(cert_request_info, std::move(callback2));
    } else {
        std::move(callback).Run(std::move(result));
    }
}

#endif // QT_CONFIG(ssl)

void ClientCertStoreQt::GetClientCerts(scoped_refptr<const net::SSLCertRequestInfo> cert_request_info,
                                       ClientCertListCallback callback)
{
#if QT_CONFIG(ssl)
    // Access the user-provided data from the UI thread, but return on whatever thread this is.
    bool ok = content::GetUIThreadTaskRunner({})->PostTaskAndReplyWithResult(
            FROM_HERE,
            base::BindOnce(&ClientCertStoreQt::GetClientCertsOnUIThread,
                           base::Unretained(this), cert_request_info),
            base::BindOnce(&ClientCertStoreQt::GetClientCertsReturn,
                           base::Unretained(this), cert_request_info, std::move(callback)));
    DCHECK(ok); // callback is already moved and we can't really recover here.
#else
    if (m_nativeStore)
        m_nativeStore->GetClientCerts(cert_request_info, std::move(callback));
    else
        std::move(callback).Run(net::ClientCertIdentityList());
#endif // QT_CONFIG(ssl)
}

#if BUILDFLAG(USE_NSS_CERTS)
// based on ChromeNSSCryptoModuleDelegate in chrome/browser/ui/crypto_module_delegate_nss.cc
class NSSCryptoModuleDelegateQt : public crypto::CryptoModuleBlockingPasswordDelegate
{
public:
    // Create a NSSCryptoModuleDelegateQt.
    NSSCryptoModuleDelegateQt(content::BrowserContext *profile,
                              const net::HostPortPair& server);

    NSSCryptoModuleDelegateQt(const NSSCryptoModuleDelegateQt&) = delete;
    NSSCryptoModuleDelegateQt& operator=(const NSSCryptoModuleDelegateQt&) = delete;

    // crypto::CryptoModuleBlockingPasswordDelegate implementation.
    std::string RequestPassword(const std::string& slot_name,
                                bool retry, bool* cancelled) override;

private:
    ~NSSCryptoModuleDelegateQt() override;

    void RequestPasswordFromUI(const std::string& slot_name, bool retry);
    void GotPassword(const std::string& password);

    content::BrowserContext *m_profile;
    net::HostPortPair m_server;

    // Event to block worker thread while waiting for dialog on UI thread.
    base::WaitableEvent m_event;

    // Stores the results from the dialog for access on worker thread.
    std::string m_password;
};

NSSCryptoModuleDelegateQt::NSSCryptoModuleDelegateQt(content::BrowserContext *profile,
                                                     const net::HostPortPair& server)
        : m_profile(profile),
          m_server(server),
          m_event(base::WaitableEvent::ResetPolicy::AUTOMATIC, base::WaitableEvent::InitialState::NOT_SIGNALED)
{}

NSSCryptoModuleDelegateQt::~NSSCryptoModuleDelegateQt() = default;

std::string NSSCryptoModuleDelegateQt::RequestPassword(const std::string& slot_name,
                                                       bool retry,
                                                       bool* cancelled)
{
    DCHECK(!m_event.IsSignaled());
    m_event.Reset();

    if (content::GetUIThreadTaskRunner({})->PostTask(
            FROM_HERE,
            base::BindOnce(&NSSCryptoModuleDelegateQt::RequestPasswordFromUI,
                            // This method blocks on |event_| until the task
                            // completes, so there's no need to ref-count.
                            base::Unretained(this), slot_name, retry))) {
        // This should always be invoked on a worker sequence with the
        // base::MayBlock() trait.
        base::ScopedAllowBaseSyncPrimitives allow_wait;
        m_event.Wait();
    }
    *cancelled = m_password.empty();
    return m_password;
}

void NSSCryptoModuleDelegateQt::RequestPasswordFromUI(const std::string &slot_name, bool retry)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    static_cast<ProfileQt*>(m_profile)->
            requestCryptoModulePassword(slot_name, retry, m_server.host(),
                                        base::BindOnce(&NSSCryptoModuleDelegateQt::GotPassword,
                                                       // RequestPassword is blocked on |event_| until GotPassword
                                                       // is called, so there's no need to ref-count.
                                                       base::Unretained(this)));
}

void NSSCryptoModuleDelegateQt::GotPassword(const std::string &password)
{
    m_password = password;
    m_event.Signal();
}

crypto::CryptoModuleBlockingPasswordDelegate *CreateCryptoModuleBlockingPasswordDelegate(content::BrowserContext *profile,
                                                                                         const net::HostPortPair& server)
{
    return new NSSCryptoModuleDelegateQt(profile, server);
}
#endif

// static
std::unique_ptr<net::ClientCertStore> ClientCertStoreQt::createNativeStore()
{
#if BUILDFLAG(USE_NSS_CERTS)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreNSS(
            base::BindRepeating(&CreateCryptoModuleBlockingPasswordDelegate, m_profile)));
#elif defined(Q_OS_WIN)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreWin());
#elif BUILDFLAG(IS_MAC)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreMac());
#else
    return nullptr;
#endif
}
} // namespace QtWebEngineCore
