// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINECLIENTCERTIFICATESTORE_H
#define QWEBENGINECLIENTCERTIFICATESTORE_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtNetwork/qtnetwork-config.h>

#if QT_CONFIG(ssl)
#include <QtCore/qlist.h>
#include <QtNetwork/qsslcertificate.h>
#include <QtNetwork/qsslkey.h>

namespace QtWebEngineCore {
struct ClientCertificateStoreData;
class ProfileAdapter;
} // namespace QtWebEngineCore

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineClientCertificateStore
{

public:
    void add(const QSslCertificate &certificate, const QSslKey &privateKey);
    QList<QSslCertificate> certificates() const;
    void remove(const QSslCertificate &certificate);
    void clear();

private:
    friend class QtWebEngineCore::ProfileAdapter;
    Q_DISABLE_COPY(QWebEngineClientCertificateStore)

    QWebEngineClientCertificateStore(QtWebEngineCore::ClientCertificateStoreData *storeData);
    ~QWebEngineClientCertificateStore();
    QtWebEngineCore::ClientCertificateStoreData *m_storeData;
};

QT_END_NAMESPACE

#endif // QT_CONFIG(ssl)
#endif // QWebEngineClientCertificateStore_H
