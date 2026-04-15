// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef CLIENT_CERT_QT_P_H
#define CLIENT_CERT_QT_P_H

#include "base/functional/callback_forward.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/client_cert_store.h"

#include <memory>

namespace content {
class BrowserContext;
}

namespace net {
class SSLCertRequestInfo;
} // namespace net

namespace QtWebEngineCore {
struct ClientCertificateStoreData;

class ClientCertStoreQt : public net::ClientCertStore
{
public:
    ClientCertStoreQt(ClientCertificateStoreData *storeData, content::BrowserContext *profile);
    virtual ~ClientCertStoreQt() override;
    void GetClientCerts(scoped_refptr<const net::SSLCertRequestInfo> cert_request_info,
                        ClientCertListCallback callback) override;
private:
    std::unique_ptr<net::ClientCertStore> createNativeStore();
    net::ClientCertIdentityList GetClientCertsOnUIThread(scoped_refptr<const net::SSLCertRequestInfo> cert_request_info);
    void GetClientCertsReturn(scoped_refptr<const net::SSLCertRequestInfo> cert_request_info,
                              ClientCertListCallback callback,
                              net::ClientCertIdentityList &&result);
    content::BrowserContext *m_profile;
    ClientCertificateStoreData *m_storeData;
    std::unique_ptr<net::ClientCertStore> m_nativeStore;
};

} // QtWebEngineCore

#endif
