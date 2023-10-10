// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <QObject>
#include <QTcpServer>

class ProxyServer : public QObject
{
    Q_OBJECT

public:
    explicit ProxyServer(QObject *parent = nullptr);
    void setCredentials(const QByteArray &user, const QByteArray password);
    void setCookie(const QByteArray &cookie);
    bool isListening();
    void setPort(int port);

public slots:
    void run();

private slots:
    void handleNewConnection();
    void handleReadReady();

signals:
    void authenticationSuccess();
    void cookieMatch();
    void requestReceived();

private:
    int m_port = 5555;
    QByteArray m_data;
    QTcpServer m_server;
    QByteArray m_auth;
    QByteArray m_cookie;
    bool m_authenticate = false;
};

#endif // PROXY_SERVER_H
