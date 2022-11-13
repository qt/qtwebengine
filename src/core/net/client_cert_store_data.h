// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CLIENT_CERT_STORE_DATA_H
#define CLIENT_CERT_STORE_DATA_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtNetwork/qtnetworkglobal.h>

#if QT_CONFIG(ssl)
#include "base/memory/ref_counted.h"

#include <QtCore/qlist.h>
#include <QtNetwork/qsslcertificate.h>
#include <QtNetwork/qsslkey.h>

namespace net {
class SSLPrivateKey;
class X509Certificate;
}

namespace QtWebEngineCore {

struct ClientCertificateStoreData
{
    struct Entry
    {
        QSslKey key;
        QSslCertificate certificate;
        scoped_refptr<net::X509Certificate> certPtr;
        scoped_refptr<net::SSLPrivateKey> keyPtr;
    };

    void add(const QSslCertificate &certificate, const QSslKey &privateKey);
    void remove(const QSslCertificate &certificate);
    void clear();

    QList<Entry *> extraCerts;
};

} // namespace QtWebEngineCore

#endif
#endif // CLIENT_CERT_STORE_DATA_H
