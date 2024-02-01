// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "httpreqrep.h"

#include <QTcpServer>
#include <QUrl>

// Listens on a TCP socket and creates HttpReqReps for each connection.
//
// Usage:
//
//     HttpServer server;
//     connect(&server, &HttpServer::newRequest, [](HttpReqRep *rr) {
//         if (rr->requestPath() == "/myPage.html") {
//             rr->setResponseBody("<html><body>Hello, World!</body></html>");
//             rr->sendResponse();
//         }
//     });
//     QVERIFY(server.start());
//     /* do stuff */
//     QVERIFY(server.stop());
//
// HttpServer owns the HttpReqRep objects. The signal handler should not store
// references to HttpReqRep objects.
//
// Only if a handler calls sendResponse() will a response be actually sent. This
// means that multiple handlers can be connected to the signal, with different
// handlers responsible for different paths.
class HttpServer : public QObject
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);
    HttpServer(const QHostAddress &hostAddress, quint16 port, QObject *parent = nullptr);
    HttpServer(QTcpServer *server, const QString &protocol, const QHostAddress &address,
               quint16 port, QObject *parent = nullptr);

    ~HttpServer() override;

    // Must be called to start listening.
    //
    // Returns true if a TCP port has been successfully bound.
    Q_INVOKABLE Q_REQUIRED_RESULT bool start();

    // Stops listening and performs final error checks.
    Q_INVOKABLE Q_REQUIRED_RESULT bool stop();

    Q_INVOKABLE void setExpectError(bool b);

    // Full URL for given relative path
    Q_INVOKABLE QUrl url(const QString &path = QStringLiteral("/")) const;

    Q_INVOKABLE QString sharedDataDir() const;

    Q_INVOKABLE void setResourceDirs(const QStringList &dirs) { m_dirs = dirs; }

    Q_INVOKABLE void setHostDomain(const QString &host) { m_url.setHost(host); }

    Q_INVOKABLE QTcpServer *getTcpServer() const { return m_tcpServer; }

Q_SIGNALS:
    // Emitted after a HTTP request has been successfully parsed.
    void newRequest(HttpReqRep *reqRep);

private Q_SLOTS:
    void handleNewConnection();

private:
    QTcpServer *m_tcpServer;
    QUrl m_url;
    QStringList m_dirs;
    bool m_error = false;
    bool m_ignoreNewConnection = false;
    bool m_expectingError = false;
    QHostAddress m_hostAddress;
    quint16 m_port;
};

#endif // !HTTPSERVER_H
