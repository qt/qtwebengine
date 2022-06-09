// Copyright (C) 2018 The Qt Company Ltd.
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

    void setPort(int port);
    bool isListening();

signals:
    void requestReceived();

public slots:
    void run();

private slots:
    void handleNewConnection();
    void handleReadReady();

private:
    int m_port;
    QByteArray m_data;
    QTcpServer m_server;

};

#endif // PROXY_SERVER_H
