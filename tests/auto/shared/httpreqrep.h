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
#ifndef HTTPREQREP_H
#define HTTPREQREP_H

#include <QTcpSocket>

#include <utility>

// Represents an HTTP request-response exchange.
class HttpReqRep : public QObject
{
    Q_OBJECT
public:
    HttpReqRep(QTcpSocket *socket, QObject *parent = nullptr);
    void sendResponse();
    QByteArray requestMethod() const { return m_requestMethod; }
    QByteArray requestPath() const { return m_requestPath; }
    QByteArray requestHeader(const QByteArray &key) const;
    void setResponseStatus(int statusCode)
    {
        m_responseStatusCode = statusCode;
    }
    void setResponseHeader(const QByteArray &key, QByteArray value)
    {
        m_responseHeaders[key.toLower()] = std::move(value);
    }
    void setResponseBody(QByteArray content)
    {
        m_responseHeaders["content-length"] = QByteArray::number(content.size());
        m_responseBody = std::move(content);
    }

Q_SIGNALS:
    void readFinished(bool ok);

private Q_SLOTS:
    void handleReadyRead();

private:
    QTcpSocket *m_socket = nullptr;
    QByteArray m_requestMethod;
    QByteArray m_requestPath;
    std::map<QByteArray, QByteArray> m_requestHeaders;
    int m_responseStatusCode = 200;
    std::map<QByteArray, QByteArray> m_responseHeaders;
    QByteArray m_responseBody;
};

#endif // !HTTPREQREP_H
