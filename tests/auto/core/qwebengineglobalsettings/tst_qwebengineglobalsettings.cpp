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
    QTest::addColumn<QWebEngineGlobalSettings::SecureDnsMode>("dnsMode");
    QTest::addColumn<QString>("uriTemplate");
    QTest::addColumn<bool>("isMockDnsServerCalledExpected");
    QTest::addColumn<bool>("isDnsResolutionSuccessExpected");
    QTest::addColumn<bool>("isConfigurationSuccessExpected");
    QTest::newRow("DnsMode::SystemOnly (no DoH server)")
            << QWebEngineGlobalSettings::SecureDnsMode::SystemOnly << QStringLiteral("") << false
            << true << true;
    QTest::newRow("DnsMode::SecureOnly (mock DoH server)")
            << QWebEngineGlobalSettings::SecureDnsMode::SecureOnly
            << QStringLiteral("https://127.0.0.1:3000/dns-query{?dns}") << true << false << true;
    QTest::newRow("DnsMode::SecureOnly (real DoH server)")
            << QWebEngineGlobalSettings::SecureDnsMode::SecureOnly
            << QStringLiteral("https://dns.google/dns-query{?dns}") << false << true << true;
    QTest::newRow("DnsMode::SecureOnly (Empty URI Templates)")
            << QWebEngineGlobalSettings::SecureDnsMode::SecureOnly << QStringLiteral("") << false
            << false << false;
    // Note: In the following test, we can't verify that the DoH server is called first and
    // afterwards insecure DNS is tried, because for the DoH server to ever be used when the DNS
    // mode is set to DnsMode::WithFallback, Chromium starts an asynchronous DoH server DnsProbe and
    // requires that the connection is successful. That is, we'd have to implement a correct
    // DNS response, which in turn requires that certificate errors aren't ignored and
    // non-self-signed certificates are used for correct encryption. Instead of implementing
    // all of that, this test verifies that Chromium tries probing the configured DoH server only.
    QTest::newRow("DnsMode::SecureWithFallback (mock DoH server)")
            << QWebEngineGlobalSettings::SecureDnsMode::SecureWithFallback
            << QStringLiteral("https://127.0.0.1:3000/dns-query{?dns}") << true << true << true;
    QTest::newRow("DnsMode::SecureWithFallback (Empty URI Templates)")
            << QWebEngineGlobalSettings::SecureDnsMode::SecureWithFallback << QStringLiteral("")
            << false << false << false;
}

void tst_QWebEngineGlobalSettings::dnsOverHttps()
{
    QFETCH(QWebEngineGlobalSettings::SecureDnsMode, dnsMode);
    QFETCH(QString, uriTemplate);
    QFETCH(bool, isMockDnsServerCalledExpected);
    QFETCH(bool, isDnsResolutionSuccessExpected);
    QFETCH(bool, isConfigurationSuccessExpected);
    bool isMockDnsServerCalled = false;
    bool isLoadSuccessful = false;

    bool configurationSuccess =
            QWebEngineGlobalSettings::setDnsMode({ dnsMode, QStringList{ uriTemplate } });
    QCOMPARE(configurationSuccess, isConfigurationSuccessExpected);

    if (!configurationSuccess) {
        // In this case, DNS has invalid configuration, so the DNS change transaction is not
        // triggered and the result of the DNS resolution depends on the current DNS mode, which is
        // set by the previous run of this function.
        return;
    }
    HttpsServer httpsServer(":/cert/localhost.crt", ":/cert/localhost.key", ":/cert/RootCA.pem",
                            3000, this);
    QObject::connect(&httpsServer, &HttpsServer::newRequest, this,
                     [&isMockDnsServerCalled](HttpReqRep *rr) {
                         QVERIFY(rr->requestPath().contains(QByteArrayLiteral("/dns-query?dns=")));
                         isMockDnsServerCalled = true;
                         rr->close();
                     });
    QVERIFY(httpsServer.start());
    httpsServer.setExpectError(isMockDnsServerCalledExpected);
    httpsServer.setVerifyMode(QSslSocket::PeerVerifyMode::VerifyNone);

    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    connect(&page, &QWebEnginePage::loadFinished, this,
            [&isLoadSuccessful](bool ok) { isLoadSuccessful = ok; });

    page.load(QUrl("https://google.com/"));
    if (!loadSpy.wait(20000)) {
        QSKIP("Couldn't load page from network, skipping test.");
    }

    QTRY_COMPARE(isMockDnsServerCalled, isMockDnsServerCalledExpected);
    QCOMPARE(isLoadSuccessful, isDnsResolutionSuccessExpected);
    QVERIFY(httpsServer.stop());
}

static QByteArrayList params = QByteArrayList() << "--ignore-certificate-errors";

W_QTEST_MAIN(tst_QWebEngineGlobalSettings, params)
#include "tst_qwebengineglobalsettings.moc"
