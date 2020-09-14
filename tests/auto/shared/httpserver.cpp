/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
#include "httpserver.h"

#include <QFile>
#include <QLoggingCategory>
#include <QMimeDatabase>

Q_LOGGING_CATEGORY(gHttpServerLog, "HttpServer")

HttpServer::HttpServer(QObject *parent) : HttpServer(new QTcpServer, "http", parent)
{
}

HttpServer::HttpServer(QTcpServer *tcpServer, const QString &protocol, QObject *parent)
    : QObject(parent), m_tcpServer(tcpServer)
{
    m_url.setHost(QStringLiteral("127.0.0.1"));
    m_url.setScheme(protocol);
    connect(tcpServer, &QTcpServer::newConnection, this, &HttpServer::handleNewConnection);
}

HttpServer::~HttpServer()
{
    delete m_tcpServer;
}

bool HttpServer::start()
{
    m_error = false;
    m_expectingError = false;

    if (!m_tcpServer->listen()) {
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
    auto rr = new HttpReqRep(m_tcpServer->nextPendingConnection(), this);
    connect(rr, &HttpReqRep::requestReceived, [this, rr]() {
        Q_EMIT newRequest(rr);
        if (rr->isClosed()) // was explicitly answered
            return;

        // if request wasn't handled or purposely ignored for default behavior
        // then try to serve htmls from resources dirs if set
        if (rr->requestMethod() == "GET") {
            for (auto &&dir : qAsConst(m_dirs)) {
                QFile f(dir + rr->requestPath());
                if (f.exists()) {
                    if (f.open(QFile::ReadOnly)) {
                        QMimeType mime = QMimeDatabase().mimeTypeForFileNameAndData(f.fileName(), &f);
                        rr->setResponseHeader(QByteArrayLiteral("Content-Type"), mime.name().toUtf8());
                        rr->setResponseBody(f.readAll());
                        rr->sendResponse();
                    } else {
                        qWarning() << "Can't open resource" << f.fileName() << ": " << f.errorString();
                        rr->sendResponse(500); // internal server error
                    }
                    break;
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
    connect(rr, &HttpReqRep::closed, rr, &QObject::deleteLater);
}
