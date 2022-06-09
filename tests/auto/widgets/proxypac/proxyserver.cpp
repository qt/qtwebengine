// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "proxyserver.h"
#include <QDataStream>
#include <QTcpSocket>
#include <QDebug>

ProxyServer::ProxyServer(QObject *parent) : QObject(parent),
    m_port(5555)
{
    connect(&m_server, &QTcpServer::newConnection, this, &ProxyServer::handleNewConnection);
}

void ProxyServer::setPort(int port)
{
    m_port = port;
}

bool ProxyServer::isListening()
{
    return m_server.isListening();
}

void ProxyServer::run()
{
    if (!m_server.listen(QHostAddress::LocalHost, m_port))
        qFatal("Could not start the test server");
}

void ProxyServer::handleNewConnection()
{
    // do one connection at the time
    Q_ASSERT(m_data.isEmpty());
    QTcpSocket *socket = m_server.nextPendingConnection();
    Q_ASSERT(socket);
    connect(socket, &QAbstractSocket::disconnected, socket, &QObject::deleteLater);
    connect(socket, &QAbstractSocket::readyRead, this, &ProxyServer::handleReadReady);
}

void ProxyServer::handleReadReady()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    Q_ASSERT(socket);

    m_data.append(socket->readAll());

    //simply wait for whole request
    if (!m_data.endsWith("\r\n\r\n"))
        return;

    // add fake proxy authetication
    socket->write("HTTP/1.1 407 Proxy Auth Required\nProxy-Authenticate: "
                      "Basic realm=\"Proxy requires authentication\"\r\n\r\n");

    m_data.clear();
    socket->disconnectFromHost();
    emit requestReceived();
}
