// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);

public slots:
    void run();

private slots:
    void handleNewConnection();
    void handleReadReady();

private:
    QTcpServer m_server;
    QByteArray m_data;
};

#endif // SERVER_H
