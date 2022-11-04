// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef CLIENT_CERT_SELECT_CONTROLLER_H
#define CLIENT_CERT_SELECT_CONTROLLER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QtNetwork/qtnetwork-config.h>

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtNetwork/QSslCertificate>
#include <memory>

namespace content {
class ClientCertificateDelegate;
}

namespace net {
class ClientCertIdentity;
class SSLCertRequestInfo;
}

namespace QtWebEngineCore {

class Q_WEBENGINECORE_PRIVATE_EXPORT ClientCertSelectController {
public:
    ClientCertSelectController(net::SSLCertRequestInfo *certRequestInfo,
                               std::vector<std::unique_ptr<net::ClientCertIdentity>> clientCerts,
                               std::unique_ptr<content::ClientCertificateDelegate> delegate);
    ~ClientCertSelectController();

    QUrl hostAndPort() const { return m_hostAndPort; }
    void selectNone();
    void select(const QSslCertificate &certificate);
    void select(int index);

    QList<QSslCertificate> certificates() const;

private:
    QUrl m_hostAndPort;
    std::vector<std::unique_ptr<net::ClientCertIdentity>> m_clientCerts;
    std::unique_ptr<content::ClientCertificateDelegate> m_delegate;
    mutable QList<QSslCertificate> m_certificates;
    bool m_selected;
};

}

#endif // CLIENT_CERT_SELECT_CONTROLLER_H
