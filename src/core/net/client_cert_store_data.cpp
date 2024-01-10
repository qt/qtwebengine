// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "net/client_cert_store_data.h"

#if QT_CONFIG(ssl)
#include "net/base/net_errors.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/ssl_platform_key_util.h"
#include "net/ssl/ssl_private_key.h"
#include "net/ssl/threaded_ssl_private_key.h"

#include "third_party/boringssl/src/include/openssl/ssl.h"
#include "third_party/boringssl/src/include/openssl/digest.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "third_party/boringssl/src/include/openssl/rsa.h"
#include "third_party/boringssl/src/include/openssl/pem.h"

#include "QtCore/qbytearray.h"

namespace {

class SSLPlatformKeyQt : public net::ThreadedSSLPrivateKey::Delegate
{
public:
    SSLPlatformKeyQt(const QByteArray &sslKeyInBytes)
    {
        m_mem = BIO_new_mem_buf(sslKeyInBytes, -1);
        m_key = PEM_read_bio_PrivateKey(m_mem, nullptr, nullptr, nullptr);
    }

    ~SSLPlatformKeyQt() override
    {
        if (m_key)
            EVP_PKEY_free(m_key);
        if (m_mem)
            BIO_free(m_mem);
    }

    net::Error Sign(uint16_t algorithm, base::span<const uint8_t> input, std::vector<uint8_t> *signature) override
    {
        bssl::ScopedEVP_MD_CTX ctx;
        EVP_PKEY_CTX *pctx;
        if (!EVP_DigestSignInit(ctx.get(), &pctx,
                                SSL_get_signature_algorithm_digest(algorithm),
                                nullptr, m_key)) {
            return net::ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED;
        }

        if (SSL_is_signature_algorithm_rsa_pss(algorithm)) {
            if (!EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PSS_PADDING) ||
                    !EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, -1 /* hash length */)) {
                return net::ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED;
            }
        }
        size_t sig_len = 0;
        if (!EVP_DigestSign(ctx.get(), NULL, &sig_len, input.data(), input.size()))
            return net::ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED;
        signature->resize(sig_len);
        if (!EVP_DigestSign(ctx.get(), signature->data(), &sig_len, input.data(), input.size()))
            return net::ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED;
        signature->resize(sig_len);
        return net::OK;
    }

    std::vector<uint16_t> GetAlgorithmPreferences() override
    {
        return net::SSLPrivateKey::DefaultAlgorithmPreferences(EVP_PKEY_id(m_key),
                                                               /* supports pss */ true);
    }
    std::string GetProviderName() override {
        return "qtwebengine";
    }
private:
    EVP_PKEY *m_key;
    BIO *m_mem;
};

scoped_refptr<net::SSLPrivateKey> wrapOpenSSLPrivateKey(const QByteArray &sslKeyInBytes)
{
    if (sslKeyInBytes.isEmpty())
        return nullptr;

    return base::MakeRefCounted<net::ThreadedSSLPrivateKey>(
                std::make_unique<SSLPlatformKeyQt>(sslKeyInBytes),
                net::GetSSLPlatformKeyTaskRunner());
}

} // namespace

namespace QtWebEngineCore {

void ClientCertificateStoreData::add(const QSslCertificate &certificate, const QSslKey &privateKey)
{
    QByteArray sslKeyInBytes = privateKey.toPem();
    QByteArray certInBytes = certificate.toDer();

    Entry *data = new Entry;
    data->keyPtr = wrapOpenSSLPrivateKey(sslKeyInBytes);
    data->certPtr = net::X509Certificate::CreateFromBytes(base::make_span((const unsigned char *)certInBytes.data(),
                                                                          (unsigned long)certInBytes.length()));
    data->key = privateKey;
    data->certificate = certificate;
    extraCerts.append(data);
}

void ClientCertificateStoreData::remove(const QSslCertificate &certificate)
{
    auto it = extraCerts.begin();
    while (it != extraCerts.end()) {
        const QtWebEngineCore::ClientCertificateStoreData::Entry *overrideData = *it;
        if (certificate.toDer() == overrideData->certificate.toDer()) {
            it = extraCerts.erase(it);
            delete overrideData;
            continue;
        }
        ++it;
    }
}

void ClientCertificateStoreData::clear()
{
    qDeleteAll(extraCerts);
    extraCerts.clear();
}

} // namespace QtWebEngineCore

#endif
