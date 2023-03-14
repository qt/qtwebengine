// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest>
#include <widgetutil.h>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QWebEngineGlobalSettings>
#include <QWebEngineLoadingInfo>

#include "httpsserver.h"
#include "httpreqrep.h"

class tst_QWebEngineGlobalSettings : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineGlobalSettings() { }
    ~tst_QWebEngineGlobalSettings() { }

public Q_SLOTS:
    void init() { }
    void cleanup() { }

private Q_SLOTS:
    void initTestCase() { }
    void cleanupTestCase() { }
    void dnsOverHttps_data();
    void dnsOverHttps();
};

Q_LOGGING_CATEGORY(lc, "qt.webengine.tests")

void tst_QWebEngineGlobalSettings::dnsOverHttps_data()
{
    QTest::addColumn<QWebEngineGlobalSettings::DnsMode>("dnsMode");
    QTest::addColumn<QString>("uriTemplate");
    QTest::addColumn<bool>("isWithCustomDnsServer");
    QTest::addColumn<bool>("isDnsResolutionSuccessExpected");
    QTest::newRow("DnsMode::Secure (mock DNS)")
            << QWebEngineGlobalSettings::DnsMode::Secure
            << QStringLiteral("https://127.0.0.1:3000/dns-query{?dns}") << true << false;
    QTest::newRow("DnsMode::Secure (real DNS)")
            << QWebEngineGlobalSettings::DnsMode::Secure
            << QStringLiteral("https://dns.google/dns-query{?dns}") << false << true;

    // Note: In the following test, we can't verify that the DoH server is called first and
    // afterwards insecure DNS is tried, because for the DoH server to ever be used when the DNS
    // mode is set to DnsMode::WithFallback, Chromium starts an asynchronous DoH server DnsProbe and
    // requires that the connection is successful. That is, we'd have to implement a correct
    // DNS response, which in turn requires that certificate errors aren't ignored and
    // non-self-signed certificates are used for correct encryption. Instead of implementing
    // all of that, this test verifies that Chromium tries probing the configured DoH server only.
    QTest::newRow("DnsMode::WithFallback (mock DNS)")
            << QWebEngineGlobalSettings::DnsMode::WithFallback
            << QStringLiteral("https://127.0.0.1:3000/dns-query{?dns}") << true << true;
}

void tst_QWebEngineGlobalSettings::dnsOverHttps()
{
    QFETCH(QWebEngineGlobalSettings::DnsMode, dnsMode);
    QFETCH(QString, uriTemplate);
    QFETCH(bool, isWithCustomDnsServer);
    QFETCH(bool, isDnsResolutionSuccessExpected);
    bool isDnsServerCalled = false;
    bool isLoadSuccessful = false;

    HttpsServer *httpsServer;
    if (isWithCustomDnsServer) {
        httpsServer = new HttpsServer(":/cert/localhost.crt", ":/cert/localhost.key",
                                      ":/cert/RootCA.pem", 3000, this);
        QObject::connect(
                httpsServer, &HttpsServer::newRequest, this, [&isDnsServerCalled](HttpReqRep *rr) {
                    QVERIFY(rr->requestPath().contains(QByteArrayLiteral("/dns-query?dns=")));
                    isDnsServerCalled = true;
                    rr->close();
                });
        QVERIFY(httpsServer->start());
        httpsServer->setExpectError(true);
        httpsServer->setVerifyMode(QSslSocket::PeerVerifyMode::VerifyNone);
    }

    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    connect(&page, &QWebEnginePage::loadFinished, this,
            [&isLoadSuccessful](bool ok) { isLoadSuccessful = ok; });

    QWebEngineGlobalSettings *globalSettings = QWebEngineGlobalSettings::GetInstance();
    globalSettings->configureDnsOverHttps(dnsMode, uriTemplate);

    page.load(QUrl("https://google.com/"));
    QVERIFY(loadSpy.wait());

    QTRY_COMPARE(isDnsServerCalled, isWithCustomDnsServer);
    QCOMPARE(isLoadSuccessful, isDnsResolutionSuccessExpected);

    if (isWithCustomDnsServer)
        QVERIFY(httpsServer->stop());
}

static QByteArrayList params = QByteArrayList() << "--ignore-certificate-errors";

W_QTEST_MAIN(tst_QWebEngineGlobalSettings, params)
#include "tst_qwebengineglobalsettings.moc"
