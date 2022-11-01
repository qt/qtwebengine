// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef HTTPSSERVER_H
#define HTTPSSERVER_H

#include "httpreqrep.h"
#include "httpserver.h"

#include <QDebug>
#include <QtCore/qfile.h>
#include <QtNetwork/qsslkey.h>
#include <QtNetwork/qsslsocket.h>
#include <QtNetwork/qsslconfiguration.h>
#include <QtNetwork/qsslserver.h>

static QSslServer *createServer(const QString &certificateFileName, const QString &keyFileName)
{
    QSslConfiguration configuration(QSslConfiguration::defaultConfiguration());

    QFile keyFile(keyFileName);
    if (keyFile.open(QIODevice::ReadOnly)) {
        QSslKey key(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
        if (!key.isNull()) {
            configuration.setPrivateKey(key);
        } else {
            qCritical() << "Could not parse key: " << keyFileName;
        }
    } else {
        qCritical() << "Could not find key: " << keyFileName;
    }

    QList<QSslCertificate> localCerts = QSslCertificate::fromPath(certificateFileName);
    if (!localCerts.isEmpty()) {
        configuration.setLocalCertificateChain(localCerts);
    } else {
        qCritical() << "Could not find certificate: " << certificateFileName;
    }

    QSslServer *server = new QSslServer();
    server->setSslConfiguration(configuration);
    return server;
}

struct HttpsServer : HttpServer
{
    HttpsServer(const QString &certPath, const QString &keyPath, QObject *parent = nullptr)
        : HttpServer(createServer(certPath, keyPath), "https", QHostAddress::LocalHost, 0, parent)
    {
    }
};

#endif
