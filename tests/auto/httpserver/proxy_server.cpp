// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "proxy_server.h"
#include <QDataStream>
#include <QTcpSocket>
#include <QDebug>

ProxyServer::ProxyServer(QObject *parent) : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &ProxyServer::handleNewConnection);
}

void ProxyServer::setPort(int port)
{
    m_port = port;
}

void ProxyServer::setCredentials(const QByteArray &user, const QByteArray password)
{
    m_auth.append(user);
    m_auth.append(':');
    m_auth.append(password);
    m_auth = m_auth.toBase64();
    m_authenticate = true;
}

void ProxyServer::setCookie(const QByteArray &cookie)
{
    m_cookie.append(QByteArrayLiteral("Cookie: "));
    m_cookie.append(cookie);
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

    if (!m_data.endsWith("\r\n\r\n"))
        return;

    if (m_authenticate && !m_data.contains(QByteArrayLiteral("Proxy-Authorization: Basic"))) {
        socket->write("HTTP/1.1 407 Proxy Authentication Required\nProxy-Authenticate: "
                      "Basic realm=\"Proxy requires authentication\"\r\n"
                      "content-length: 0\r\n"
                      "\r\n");
        return;
    }

    if (m_authenticate && m_data.contains(m_auth)) {
        emit authenticationSuccess();
    }

    if (m_data.contains(m_cookie)) {
        emit cookieMatch();
    }
    m_data.clear();
    emit requestReceived();
}

#include "moc_proxy_server.cpp"
