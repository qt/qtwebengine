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

#ifndef CLIENT_CERT_OVERRIDE_KEY_H
#define CLIENT_CERT_OVERRIDE_KEY_H

#include "client_cert_override_key_p.h"

#include "third_party/boringssl/src/include/openssl/ssl.h"
#include "third_party/boringssl/src/include/openssl/digest.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "third_party/boringssl/src/include/openssl/rsa.h"
#include "third_party/boringssl/src/include/openssl/pem.h"

#include <utility>
#include <QByteArray>

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "net/base/net_errors.h"
#include "net/ssl/ssl_platform_key_util.h"
#include "net/ssl/ssl_private_key.h"
#include "net/ssl/threaded_ssl_private_key.h"

namespace net {

namespace {

class SSLPlatformKeyOverride : public ThreadedSSLPrivateKey::Delegate {
public:
    SSLPlatformKeyOverride(const QByteArray &sslKeyInBytes)
    {
        mem_ = BIO_new_mem_buf(sslKeyInBytes, -1);
        key_ = PEM_read_bio_PrivateKey(mem_, NULL, 0, NULL);
    }

    ~SSLPlatformKeyOverride() override {
        if (key_)
            EVP_PKEY_free(key_);
        if (mem_)
            BIO_free(mem_);
    }

    Error Sign(uint16_t algorithm,
               base::span<const uint8_t> input,
               std::vector<uint8_t>* signature) override {
        bssl::ScopedEVP_MD_CTX ctx;
        EVP_PKEY_CTX* pctx;
        if (!EVP_DigestSignInit(ctx.get(), &pctx,
                                SSL_get_signature_algorithm_digest(algorithm),
                                nullptr, key_)) {
            return ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED;
        }

        if (SSL_is_signature_algorithm_rsa_pss(algorithm)) {
            if (!EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PSS_PADDING) ||
                    !EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, -1 /* hash length */)) {
                return ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED;
            }
        }
        size_t sig_len = 0;
        if (!EVP_DigestSign(ctx.get(), NULL, &sig_len, input.data(), input.size()))
            return ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED;
        signature->resize(sig_len);
        if (!EVP_DigestSign(ctx.get(), signature->data(), &sig_len, input.data(),
                            input.size())) {
            return ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED;
        }
        signature->resize(sig_len);
        return OK;
    }

    std::vector<uint16_t> GetAlgorithmPreferences() override {
        return {
            SSL_SIGN_RSA_PKCS1_SHA1, SSL_SIGN_RSA_PKCS1_SHA512,
                    SSL_SIGN_RSA_PKCS1_SHA384, SSL_SIGN_RSA_PKCS1_SHA256,
        };
    }

private:
    EVP_PKEY* key_;
    BIO * mem_;

    DISALLOW_COPY_AND_ASSIGN(SSLPlatformKeyOverride);
};

}  // namespace

scoped_refptr<SSLPrivateKey> WrapOpenSSLPrivateKey(const QByteArray &sslKeyInBytes) {
    if (sslKeyInBytes.isEmpty())
        return nullptr;

    return base::MakeRefCounted<ThreadedSSLPrivateKey>(
                std::make_unique<SSLPlatformKeyOverride>(sslKeyInBytes),
                GetSSLPlatformKeyTaskRunner());
}

}  // namespace net

#endif
