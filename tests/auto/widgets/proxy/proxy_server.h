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

public slots:
    void run();

private slots:
    void handleNewConnection();
    void handleReadReady();

signals:
    void authenticationSuccess();
    void cookieMatch();

private:
    QByteArray m_data;
    QTcpServer m_server;
    QByteArray m_auth;
    QByteArray m_cookie;
    bool m_authenticate = false;
};

#endif // PROXY_SERVER_H
