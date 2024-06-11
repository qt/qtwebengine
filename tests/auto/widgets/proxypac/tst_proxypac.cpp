// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "proxy_server.h"
#include <QTest>
#include <QSignalSpy>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QNetworkProxy>

class tst_ProxyPac : public QObject {
    Q_OBJECT
public:
    tst_ProxyPac(){}

private slots:
    void proxypac();
};

void tst_ProxyPac::proxypac()
{
    const QString fromEnv = qEnvironmentVariable("QTWEBENGINE_CHROMIUM_FLAGS");
    if (!fromEnv.contains("--proxy-pac-url"))
        qFatal("--proxy-pac-url argument is not passed. Use ctest or set QTWEBENGINE_CHROMIUM_FLAGS");

    ProxyServer proxyServer1;
    QSignalSpy proxySpy1(&proxyServer1, &ProxyServer::requestReceived);
    proxyServer1.setPort(5551);
    proxyServer1.run();

    ProxyServer proxyServer2;
    QSignalSpy proxySpy2(&proxyServer2, &ProxyServer::requestReceived);
    proxyServer2.setPort(5552);
    proxyServer2.run();

    QTRY_VERIFY2(proxyServer1.isListening(), "Could not setup proxy server 1");
    QTRY_VERIFY2(proxyServer2.isListening(), "Could not setup proxy server 2");

    QWebEngineProfile profile;
    QWebEnginePage page(&profile);

    const bool v8_proxy_resolver_enabled = !fromEnv.contains("--single-process");
    page.load(QUrl("http://test.proxy1.com"));
    QTRY_COMPARE(proxySpy1.size() >= 1, v8_proxy_resolver_enabled);
    QVERIFY(proxySpy2.size() == 0);
    page.load(QUrl("http://test.proxy2.com"));
    QTRY_COMPARE(proxySpy2.size() >= 1, v8_proxy_resolver_enabled);

    // check for crash
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    page.load(QUrl("https://contribute.qt-project.org"));

    QTRY_VERIFY_WITH_TIMEOUT(!spyFinished.isEmpty(), 200000);

}

#include "tst_proxypac.moc"
QTEST_MAIN(tst_ProxyPac)

