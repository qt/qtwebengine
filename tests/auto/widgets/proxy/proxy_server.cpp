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

#include "proxy_server.h"
#include <QDataStream>
#include <QTcpSocket>
#include <QDebug>

ProxyServer::ProxyServer(QObject *parent) : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &ProxyServer::handleNewConnection);
}

void ProxyServer::setCredentials(const QByteArray &user, const QByteArray password)
{
    m_auth.append(user);
    m_auth.append(QChar(':'));
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
    if (!m_server.listen(QHostAddress::LocalHost, 5555))
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
}
