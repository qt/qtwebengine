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

#include "proxy_server.h"
#include <QTest>
#include <QSignalSpy>
#include <QNetworkProxy>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWebEngineUrlRequestInterceptor>


struct Interceptor : public QWebEngineUrlRequestInterceptor
{
   Interceptor(const QByteArray cookie):m_cookie(cookie){};
   void interceptRequest(QWebEngineUrlRequestInfo &info) override {
       info.setHttpHeader(QByteArray("Cookie"), m_cookie);
   };
   QByteArray m_cookie;
};


class tst_Proxy : public QObject {
    Q_OBJECT
public:
    tst_Proxy(){}

private slots:
    void proxyAuthentication();
    void forwardCookie();
};


void tst_Proxy::proxyAuthentication()
{
    QByteArray user(QByteArrayLiteral("test"));
    QByteArray password(QByteArrayLiteral("pass"));
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("localhost");
    proxy.setPort(5555);
    proxy.setUser(user);
    proxy.setPassword(password);
    QNetworkProxy::setApplicationProxy(proxy);
    ProxyServer server;
    server.setCredentials(user,password);
    server.run();
    QTRY_VERIFY2(server.isListening(), "Could not setup authentication server");
    QWebEnginePage page;
    QSignalSpy successSpy(&server, &ProxyServer::authenticationSuccess);
    page.load(QUrl("http://www.qt.io"));
    QTRY_VERIFY2(successSpy.count() > 0, "Could not get authentication token");
}

void tst_Proxy::forwardCookie()
{
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("localhost");
    proxy.setPort(5555);
    QNetworkProxy::setApplicationProxy(proxy);
    ProxyServer server;
    QByteArray cookie("foo=bar; sessionToken=123");
    server.setCookie(cookie);
    server.run();
    QTRY_VERIFY2(server.isListening(), "Could not setup proxy server");
    Interceptor interceptor(cookie);
    QWebEnginePage page;
    page.setUrlRequestInterceptor(&interceptor);
    QSignalSpy cookieSpy(&server, &ProxyServer::cookieMatch);
    page.load(QUrl("http://www.qt.io"));
    QTRY_VERIFY2(cookieSpy.count() > 0, "Could not get cookie");
}

#include "tst_proxy.moc"
QTEST_MAIN(tst_Proxy)

