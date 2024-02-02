// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CLIENT_CERT_QT_P_H
#define CLIENT_CERT_QT_P_H

#include "base/functional/callback_forward.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/client_cert_store.h"

namespace net {
class SSLCertRequestInfo;
} // namespace net

namespace QtWebEngineCore {
struct ClientCertificateStoreData;

class ClientCertStoreQt : public net::ClientCertStore
{
public:
    ClientCertStoreQt(ClientCertificateStoreData *storeData);
    virtual ~ClientCertStoreQt() override;
    void GetClientCerts(const net::SSLCertRequestInfo &cert_request_info,
                        ClientCertListCallback callback) override;
private:
    static std::unique_ptr<net::ClientCertStore> createNativeStore();
    net::ClientCertIdentityList GetClientCertsOnUIThread(const net::SSLCertRequestInfo &request);
    void GetClientCertsReturn(const net::SSLCertRequestInfo &cert_request_info,
                              ClientCertListCallback callback,
                              net::ClientCertIdentityList &&result);
    ClientCertificateStoreData *m_storeData;
    std::unique_ptr<net::ClientCertStore> m_nativeStore;
};

} // QtWebEngineCore

#endif
