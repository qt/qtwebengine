// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "httpreqrep.h"

HttpReqRep::HttpReqRep(QTcpSocket *socket, QObject *parent)
    : QObject(parent), m_socket(socket)
{
    m_socket->setParent(this);
    connect(m_socket, &QTcpSocket::readyRead, this, &HttpReqRep::handleReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &HttpReqRep::handleDisconnected);
}

void HttpReqRep::sendResponse(int statusCode)
{
    if (m_state != State::REQUEST_RECEIVED)
        return;
    m_responseStatusCode = statusCode;
    m_socket->write("HTTP/1.1 ");
    m_socket->write(QByteArray::number(m_responseStatusCode));
    m_socket->write(" OK?\r\n");
    for (const auto & kv : m_responseHeaders) {
        m_socket->write(kv.first);
        m_socket->write(": ");
        m_socket->write(kv.second);
        m_socket->write("\r\n");
    }
    m_socket->write("Connection: close\r\n");
    m_socket->write("\r\n");
    m_socket->write(m_responseBody);
    m_state = State::DISCONNECTING;
    m_socket->disconnectFromHost();
    Q_EMIT responseSent();
}

void HttpReqRep::sendResponse(const QByteArray &response)
{
    m_socket->write(response);
    m_state = State::DISCONNECTING;
    m_socket->disconnectFromHost();
    Q_EMIT responseSent();
}

void HttpReqRep::close()
{
    if (m_state != State::REQUEST_RECEIVED)
        return;
    m_state = State::DISCONNECTING;
    m_socket->disconnectFromHost();
    Q_EMIT error(QStringLiteral("missing response"));
}

QByteArray HttpReqRep::requestHeader(const QByteArray &key) const
{
    auto it = m_requestHeaders.find(key.toLower());
    if (it != m_requestHeaders.end())
        return it->second;
    return {};
}

void HttpReqRep::handleReadyRead()
{
    while (m_socket->canReadLine()) {
        switch (m_state) {
        case State::RECEIVING_REQUEST: {
            const auto requestLine = m_socket->readLine();
            const auto requestLineParts = requestLine.split(' ');
            if (requestLineParts.size() != 3 || !requestLineParts[2].toUpper().startsWith("HTTP/")) {
                m_state = State::DISCONNECTING;
                m_socket->disconnectFromHost();
                Q_EMIT error(QStringLiteral("invalid request line"));
                return;
            }
            m_requestMethod = requestLineParts[0];
            m_requestPath = requestLineParts[1];
            m_state = State::RECEIVING_HEADERS;
            break;
        }
        case State::RECEIVING_HEADERS: {
            const auto headerLine = m_socket->readLine();
            if (headerLine == QByteArrayLiteral("\r\n")) {
                m_state = State::REQUEST_RECEIVED;
                Q_EMIT requestReceived();
                return;
            }
            int colonIndex = headerLine.indexOf(':');
            if (colonIndex < 0) {
                m_state = State::DISCONNECTING;
                m_socket->disconnectFromHost();
                Q_EMIT error(QStringLiteral("invalid header line"));
                return;
            }
            auto headerKey = headerLine.left(colonIndex).trimmed().toLower();
            auto headerValue = headerLine.mid(colonIndex + 1).trimmed().toLower();
            m_requestHeaders.emplace(headerKey, headerValue);
            break;
        }
        default:
            return;
        }
    }
}

void HttpReqRep::handleDisconnected()
{
    switch (m_state) {
    case State::RECEIVING_REQUEST:
    case State::RECEIVING_HEADERS:
    case State::REQUEST_RECEIVED:
        Q_EMIT error(QStringLiteral("unexpected disconnect"));
        break;
    case State::DISCONNECTING:
        break;
    case State::DISCONNECTED:
        Q_UNREACHABLE();
    }
    m_state = State::DISCONNECTED;
    Q_EMIT closed();
}

#include "moc_httpreqrep.cpp"
