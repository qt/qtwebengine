// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
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
    SslTcpServer(const QString &certPath, const QString &keyPath) {
        sslconf.setLocalCertificateChain(QSslCertificate::fromPath(certPath));
        sslconf.setPrivateKey(readKey(keyPath));
    }

    void incomingConnection(qintptr d) override {
        auto socket = new QSslSocket(this);
        socket->setSslConfiguration(sslconf);

        if (!socket->setSocketDescriptor(d)) {
            qWarning() << "Failed to setup ssl socket!";
            delete socket;
            return;
        }

        connect(socket, QOverload<QSslSocket::SocketError>::of(&QSslSocket::errorOccurred),
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
    HttpsServer(const QString &certPath, const QString &keyPath, QObject *parent = nullptr)
        : HttpServer(new SslTcpServer(certPath, keyPath), "https", QHostAddress::LocalHost, 0,
                     parent)
    {
    }
};

#endif
