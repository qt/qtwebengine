// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <httpsserver.h>
#include <util.h>
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineclientcertificatestore.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginecertificateerror.h>
#include <QtWebEngineCore/qwebenginesettings.h>

class tst_QWebEngineClientCertificateStore : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineClientCertificateStore();
    ~tst_QWebEngineClientCertificateStore();

private Q_SLOTS:
    void init();
    void cleanup();
    void addAndListCertificates();
    void removeAndClearCertificates();
    void clientAuthentication_data();
    void clientAuthentication();
};

tst_QWebEngineClientCertificateStore::tst_QWebEngineClientCertificateStore()
{
}

tst_QWebEngineClientCertificateStore::~tst_QWebEngineClientCertificateStore()
{
}

void tst_QWebEngineClientCertificateStore::init()
{
    QCOMPARE(0,
             QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates().size());
}

void tst_QWebEngineClientCertificateStore::cleanup()
{
    QWebEngineProfile::defaultProfile()->clientCertificateStore()->clear();
    QCOMPARE(0,
             QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates().size());
}

void tst_QWebEngineClientCertificateStore::addAndListCertificates()
{
    // Load QSslCertificate
    QFile certFile(":/resources/certificate.crt");
    certFile.open(QIODevice::ReadOnly);
    const QSslCertificate cert(certFile.readAll(), QSsl::Pem);

    // Load QSslKey
    QFile keyFile(":/resources/privatekey.key");
    keyFile.open(QIODevice::ReadOnly);
    const QSslKey sslKey(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "");

    // Load second QSslCertificate
    QFile certFileSecond(":/resources/certificate1.crt");
    certFileSecond.open(QIODevice::ReadOnly);
    const QSslCertificate certSecond(certFileSecond.readAll(), QSsl::Pem);

    // Load second QSslKey
    QFile keyFileSecond(":/resources/privatekey1.key");
    keyFileSecond.open(QIODevice::ReadOnly);
    const QSslKey sslKeySecond(keyFileSecond.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "");

    // Add certificates to in-memory store
    QWebEngineProfile::defaultProfile()->clientCertificateStore()->add(cert, sslKey);
    QWebEngineProfile::defaultProfile()->clientCertificateStore()->add(certSecond, sslKeySecond);

    QCOMPARE(2, QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates().size());
}

void tst_QWebEngineClientCertificateStore::removeAndClearCertificates()
{
    addAndListCertificates();
    QCOMPARE(2, QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates().size());

    // Remove one certificate from in-memory store
    auto list = QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates();
    QWebEngineProfile::defaultProfile()->clientCertificateStore()->remove(list[0]);
    QCOMPARE(1, QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates().size());

    // Remove all certificates in-memory store
    QWebEngineProfile::defaultProfile()->clientCertificateStore()->clear();
    QCOMPARE(0, QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates().size());
}

void tst_QWebEngineClientCertificateStore::clientAuthentication_data()
{
    QTest::addColumn<QString>("client_certificate");
    QTest::addColumn<QString>("client_key");
    QTest::addColumn<bool>("in_memory");
    QTest::addColumn<bool>("add_more_in_memory_certificates");
    QTest::newRow("in_memory") << ":/resources/client.pem"
                               << ":/resources/client.key" << true << false;
#if defined(TEST_NSS)
    QTest::newRow("nss") << ":/resources/client2.pem"
                         << ":/resources/client2.key" << false << false;
    QTest::newRow("in_memory + nss") << ":/resources/client2.pem"
                                     << ":/resources/client2.key" << false << true;
#endif
}

void tst_QWebEngineClientCertificateStore::clientAuthentication()
{
    QFETCH(QString, client_certificate);
    QFETCH(QString, client_key);
    QFETCH(bool, in_memory);
    QFETCH(bool, add_more_in_memory_certificates);

    HttpsServer server(":/resources/server.pem", ":/resources/server.key", ":resources/ca.pem");
    server.setExpectError(false);
    QVERIFY(server.start());

    connect(&server, &HttpsServer::newRequest, [&](HttpReqRep *rr) {
        rr->setResponseBody(QByteArrayLiteral("<html><body>TEST</body></html>"));
        rr->sendResponse();
    });

    QFile certFile(client_certificate);
    certFile.open(QIODevice::ReadOnly);
    const QSslCertificate cert(certFile.readAll(), QSsl::Pem);

    QFile keyFile(client_key);
    keyFile.open(QIODevice::ReadOnly);
    const QSslKey sslKey(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "");

    if (in_memory)
        QWebEngineProfile::defaultProfile()->clientCertificateStore()->add(cert, sslKey);

    if (add_more_in_memory_certificates)
        addAndListCertificates();

    QWebEnginePage page;
    connect(&page, &QWebEnginePage::certificateError, [](QWebEngineCertificateError e) {
        // ca is self signed in this test simply accept the certificate error
        e.acceptCertificate();
    });
    connect(&page, &QWebEnginePage::selectClientCertificate, &page,
            [&cert](QWebEngineClientCertificateSelection selection) {
                QVERIFY(!selection.certificates().isEmpty());
                for (const QSslCertificate &sCert : selection.certificates()) {
                    if (cert == sCert) {
                        selection.select(sCert);
                        return;
                    }
                }
                QFAIL("No certificate found.");
            });
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    page.setUrl(server.url());
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size() > 0, true, 20000);
    QCOMPARE(loadFinishedSpy.takeFirst().at(0).toBool(), true);
    QCOMPARE(toPlainTextSync(&page), QStringLiteral("TEST"));
    QVERIFY(server.stop());
}

QTEST_MAIN(tst_QWebEngineClientCertificateStore)
#include "tst_qwebengineclientcertificatestore.moc"
