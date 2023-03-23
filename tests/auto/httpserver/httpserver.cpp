// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "httpserver.h"

#include <QFile>
#include <QLoggingCategory>
#include <QMimeDatabase>

Q_LOGGING_CATEGORY(gHttpServerLog, "HttpServer")

HttpServer::HttpServer(QObject *parent)
    : HttpServer(new QTcpServer, "http", QHostAddress::LocalHost, 0, parent)
{
}

HttpServer::HttpServer(const QHostAddress &hostAddress, quint16 port, QObject *parent)
    : HttpServer(new QTcpServer, "http", hostAddress, port, parent)
{
}

HttpServer::HttpServer(QTcpServer *tcpServer, const QString &protocol,
                       const QHostAddress &hostAddress, quint16 port, QObject *parent)
    : QObject(parent), m_tcpServer(tcpServer), m_hostAddress(hostAddress), m_port(port)
{
    m_url.setHost(hostAddress.toString());
    m_url.setScheme(protocol);
    connect(tcpServer, &QTcpServer::pendingConnectionAvailable, this,
            &HttpServer::handleNewConnection);
}

HttpServer::~HttpServer()
{
    delete m_tcpServer;
}

bool HttpServer::start()
{
    m_error = false;
    m_expectingError = false;
    m_ignoreNewConnection = false;

    if (!m_tcpServer->listen(m_hostAddress, m_port)) {
        qCWarning(gHttpServerLog).noquote() << m_tcpServer->errorString();
        return false;
    }

    m_url.setPort(m_tcpServer->serverPort());
    return true;
}

bool HttpServer::stop()
{
    m_tcpServer->close();
    return m_error == m_expectingError;
}

void HttpServer::setExpectError(bool b)
{
    m_expectingError = b;
}

QUrl HttpServer::url(const QString &path) const
{
    auto copy = m_url;
    copy.setPath(path);
    return copy;
}

void HttpServer::handleNewConnection()
{
    if (m_ignoreNewConnection)
        return;

    auto rr = new HttpReqRep(m_tcpServer->nextPendingConnection(), this);
    connect(rr, &HttpReqRep::requestReceived, [this, rr]() {
        Q_EMIT newRequest(rr);
        if (rr->isClosed()) // was explicitly answered
            return;

        // if request wasn't handled or purposely ignored for default behavior
        // then try to serve htmls from resources dirs if set
        if (rr->requestMethod() == "GET") {
            for (auto &&dir : std::as_const(m_dirs)) {
                QFile f(dir + rr->requestPath());
                if (f.exists()) {
                    if (f.open(QFile::ReadOnly)) {
                        QMimeType mime = QMimeDatabase().mimeTypeForFileNameAndData(f.fileName(), &f);
                        rr->setResponseHeader(QByteArrayLiteral("Content-Type"), mime.name().toUtf8());
                        rr->setResponseHeader(QByteArrayLiteral("Access-Control-Allow-Origin"), QByteArrayLiteral("*"));
                        rr->setResponseBody(f.readAll());
                        rr->sendResponse();
                    } else {
                        qWarning() << "Can't open resource" << f.fileName() << ": " << f.errorString();
                        rr->sendResponse(500); // internal server error
                    }
                    break;
                } else {
                    qWarning() << "Can't open resource" << dir + rr->requestPath();
                }
            }
        }

        if (!rr->isClosed())
            rr->sendResponse(404);
    });
    connect(rr, &HttpReqRep::responseSent, [rr]() {
        qCInfo(gHttpServerLog).noquote() << rr->requestMethod() << rr->requestPath()
                                         << rr->responseStatus() << rr->responseBody().size();
    });
    connect(rr, &HttpReqRep::error, [this, rr](const QString &error) {
        qCWarning(gHttpServerLog).noquote() << rr->requestMethod() << rr->requestPath()
                                            << error;
        m_error = true;
    });

    if (!m_tcpServer->isListening()) {
        m_ignoreNewConnection = true;
        connect(rr, &HttpReqRep::closed, rr, &QObject::deleteLater);
    }
}

QString HttpServer::sharedDataDir() const
{
    return SERVER_SOURCE_DIR + QLatin1String("/data");
}

#include "moc_httpserver.cpp"
