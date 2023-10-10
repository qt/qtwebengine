// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "server.h"
#include <QDataStream>
#include <QTcpSocket>

Server::Server(QObject *parent) : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &Server::handleNewConnection);
}

void Server::run()
{
    if (!m_server.listen(QHostAddress::LocalHost, 5555))
        qWarning() << "Could not start the server -> http/proxy authentication dialog"
                      " will not work. Error:" << m_server.errorString();
}

void Server::handleNewConnection()
{
    QTcpSocket *socket = m_server.nextPendingConnection();
    connect(socket, &QAbstractSocket::disconnected, socket, &QObject::deleteLater);
    connect(socket, &QAbstractSocket::readyRead, this, &Server::handleReadReady);
}

void Server::handleReadReady()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    Q_ASSERT(socket);
    m_data.append(socket->readAll());

    // simply wait for whole request
    if (!m_data.endsWith("\r\n\r\n"))
        return;
    if (m_data.contains(QByteArrayLiteral("OPEN_AUTH"))) {
        socket->write("HTTP/1.1 401 Unauthorized\nWWW-Authenticate: "
                      "Basic realm=\"Very Restricted Area\"\r\n\r\n");
        m_data.clear();
        return;
    }

    socket->write("HTTP/1.1 407 Proxy Auth Required\nProxy-Authenticate: "
                  "Basic realm=\"Proxy requires authentication\"\r\n"
                  "content-length: 0\r\n\r\n");
    m_data.clear();
}
