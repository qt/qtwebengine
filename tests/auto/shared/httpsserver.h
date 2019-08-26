/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef HTTPSSERVER_H
#define HTTPSSERVER_H

#include "httpreqrep.h"
#include "httpserver.h"

#include <QDebug>
#include <QFile>
#include <QSslKey>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QTcpServer>

struct SslTcpServer : QTcpServer
{
    SslTcpServer() {
        sslconf.setLocalCertificateChain(QSslCertificate::fromPath(":/resources/cert.pem"));
        sslconf.setPrivateKey(readKey(":/resources/key.pem"));
    }

    void incomingConnection(qintptr d) override {
        auto socket = new QSslSocket(this);
        socket->setSslConfiguration(sslconf);

        if (!socket->setSocketDescriptor(d)) {
            qWarning() << "Failed to setup ssl socket!";
            delete socket;
            return;
        }

        connect(socket, QOverload<QSslSocket::SocketError>::of(&QSslSocket::error),
                [] (QSslSocket::SocketError e) { qWarning() << "! Socket Error:" << e; });
        connect(socket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
                [] (const QList<QSslError> &le) { qWarning() << "! SSL Errors:\n" << le; });

        addPendingConnection(socket);
        socket->startServerEncryption();
    }

    QSslKey readKey(const QString &path) const {
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        return QSslKey(file.readAll(), QSsl::Rsa, QSsl::Pem);
    }

    QSslConfiguration sslconf;
};

struct HttpsServer : HttpServer
{
    HttpsServer(QObject *parent = nullptr) : HttpServer(new SslTcpServer, "https", parent) { }
};

#endif
