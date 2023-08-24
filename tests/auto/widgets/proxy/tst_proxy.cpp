// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "proxy_server.h"
#include <QTest>
#include <QSignalSpy>
#include <QNetworkProxy>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineLoadingInfo>

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
    void invalidHostName();
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
    QTRY_VERIFY2(successSpy.size() > 0, "Could not get authentication token");
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
    QTRY_VERIFY2(cookieSpy.size() > 0, "Could not get cookie");
}

// Crash test ( https://bugreports.qt.io/browse/QTBUG-113992 )
void tst_Proxy::invalidHostName()
{
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("999.0.0.0");
    QNetworkProxy::setApplicationProxy(proxy);
    QWebEnginePage page;
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl("http://www.qt.io"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);
}

#include "tst_proxy.moc"
QTEST_MAIN(tst_Proxy)

