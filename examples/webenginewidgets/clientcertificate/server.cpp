// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/qcoreapplication.h>
#include <QtCore/qfile.h>
#include <QtCore/qpointer.h>
#include <QtCore/qtimer.h>
#include <QtNetwork/qsslconfiguration.h>
#include <QtNetwork/qsslkey.h>
#include <QtNetwork/qsslserver.h>

struct Request : public QObject
{
    QByteArray m_data;
};

static const QByteArray http_ok(QByteArrayLiteral(
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"));
static const QByteArray html_start(QByteArrayLiteral("<html><style>"
                                                     "div {"
                                                     "height: 400px;"
                                                     "width: 200px;"
                                                     "position: fixed;"
                                                     "top: 50%;"
                                                     "left: 50%;"
                                                     "margin-top: -100px;"
                                                     "margin-left: -200px;"
                                                     "}</style><body><div>"));
static const QByteArray html_end(QByteArrayLiteral("</div></body></html>"));

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QCoreApplication app(argc, argv);

    QSslServer server;
    QSslConfiguration configuration(QSslConfiguration::defaultConfiguration());
    configuration.setPeerVerifyMode(QSslSocket::VerifyPeer);

    QFile keyFile(":/resources/server.key");
    keyFile.open(QIODevice::ReadOnly);

    QSslKey key(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
    configuration.setPrivateKey(key);

    QList<QSslCertificate> localCerts = QSslCertificate::fromPath(":/resources/server.pem");
    configuration.setLocalCertificateChain(localCerts);

    QList<QSslCertificate> caCerts = QSslCertificate::fromPath(":resources/ca.pem");
    configuration.addCaCertificates(caCerts);

    server.setSslConfiguration(configuration);

    if (!server.listen(QHostAddress::LocalHost, 5555))
        qFatal("Could not start server on localhost:5555");
    else
        qInfo("Server started on localhost:5555");

    QObject::connect(&server, &QTcpServer::pendingConnectionAvailable, [&server]() {
        QTcpSocket *socket = server.nextPendingConnection();
        Q_ASSERT(socket);

        QPointer<Request> request(new Request);

        QObject::connect(socket, &QAbstractSocket::disconnected, socket,
                         [socket, request]() mutable {
                             delete request;
                             socket->deleteLater();
                         });

        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket, request]() mutable {
            request->m_data.append(socket->readAll());

            if (!request->m_data.endsWith("\r\n\r\n"))
                return;

            socket->write(http_ok);
            socket->write(html_start);

            if (request->m_data.startsWith("GET / ")) {
                socket->write("<p>ACCESS GRANTED !</p>");
                socket->write("<p>You reached the place, where no one has gone before.</p>");
                socket->write("<button onclick=\"window.location.href='/exit'\">Exit</button>");
            } else if (request->m_data.startsWith("GET /exit ")) {
                socket->write("<p>BYE !</p>");
                socket->write("<p>Have good day ...</p>");
                QTimer::singleShot(0, &QCoreApplication::quit);
            } else {
                socket->write("<p>There is nothing to see here.</p>");
            }

            socket->write(html_end);
            delete request;
            socket->disconnectFromHost();
        });
    });

    return app.exec();
}
