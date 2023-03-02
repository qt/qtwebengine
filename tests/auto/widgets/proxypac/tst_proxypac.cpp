/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
    QTRY_COMPARE(proxySpy1.count() >= 1, v8_proxy_resolver_enabled);
    QVERIFY(proxySpy2.count() == 0);
    page.load(QUrl("http://test.proxy2.com"));
    QTRY_COMPARE(proxySpy2.count() >= 1, v8_proxy_resolver_enabled);

    // check for crash
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    page.load(QUrl("https://contribute.qt-project.org"));

    QTRY_VERIFY_WITH_TIMEOUT(!spyFinished.isEmpty(), 100000);

}

#include "tst_proxypac.moc"
QTEST_MAIN(tst_ProxyPac)

