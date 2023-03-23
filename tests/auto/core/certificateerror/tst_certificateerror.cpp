// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include <httpsserver.h>
#include <util.h>

#include <QWebEngineCertificateError>
#include <QWebEnginePage>
#include <QWebEngineSettings>

#include <QtTest/QtTest>

class tst_CertificateError : public QObject
{
    Q_OBJECT
public:
    tst_CertificateError() { }

private Q_SLOTS:
    void handleError_data();
    void handleError();
    void fatalError();
};

struct PageWithCertificateErrorHandler : QWebEnginePage
{
    Q_OBJECT

public:
    PageWithCertificateErrorHandler(bool defer, bool accept, QObject *p = nullptr)
        : QWebEnginePage(p), deferError(defer), acceptCertificate(accept)
        , loadSpy(this, &QWebEnginePage::loadFinished)
    {
        connect(this, &PageWithCertificateErrorHandler::certificateError,
                this, &PageWithCertificateErrorHandler::onCertificateError);
    }

    bool deferError, acceptCertificate;

    QSignalSpy loadSpy;
    QScopedPointer<QWebEngineCertificateError> error;

public Q_SLOTS:
    void onCertificateError(QWebEngineCertificateError e)
    {
        error.reset(new QWebEngineCertificateError(e));
        if (deferError) {
            error->defer();
            return;
        }

        if (acceptCertificate)
            error->acceptCertificate();
        else
            error->rejectCertificate();
    }
};

void tst_CertificateError::handleError_data()
{
    QTest::addColumn<bool>("deferError");
    QTest::addColumn<bool>("acceptCertificate");
    QTest::addColumn<QString>("expectedContent");
    QTest::addRow("Reject") << false << false << QString();
    QTest::addRow("DeferReject") << true << false << QString();
    QTest::addRow("DeferAccept") << true << true << "TEST";
}

void tst_CertificateError::handleError()
{
    HttpsServer server(":/resources/server.pem", ":/resources/server.key", "");
    server.setExpectError(false);
    QVERIFY(server.start());

    connect(&server, &HttpsServer::newRequest, [&] (HttpReqRep *rr) {
        rr->setResponseBody(QByteArrayLiteral("<html><body>TEST</body></html>"));
        rr->sendResponse();
    });

    QFETCH(bool, deferError);
    QFETCH(bool, acceptCertificate);
    QFETCH(QString, expectedContent);

    PageWithCertificateErrorHandler page(deferError, acceptCertificate);
    page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);

    page.setUrl(server.url());
    QTRY_VERIFY(page.error);
    QVERIFY(page.error->isOverridable());
    auto chain = page.error->certificateChain();
    QCOMPARE(chain.size(), 2);
    QCOMPARE(chain[0].serialNumber(), "15:91:08:23:37:91:ee:51:00:d7:4a:db:d7:8c:3b:31:f8:4f:f3:b3");
    QCOMPARE(chain[1].serialNumber(), "3c:16:83:83:59:c4:2a:65:8f:7a:b2:07:10:14:4e:2d:70:9a:3e:23");

    if (deferError) {
        QCOMPARE(page.loadSpy.size(), 0);
        QCOMPARE(toPlainTextSync(&page), QString());

        if (acceptCertificate)
            page.error->acceptCertificate();
        else
            page.error->rejectCertificate();

        page.error.reset();
    }
    QTRY_COMPARE_WITH_TIMEOUT(page.loadSpy.size(), 1, 30000);
    QCOMPARE(page.loadSpy.takeFirst().value(0).toBool(), acceptCertificate);
    QCOMPARE(toPlainTextSync(&page), expectedContent);
    QVERIFY(server.stop());
}

void tst_CertificateError::fatalError()
{
    PageWithCertificateErrorHandler page(false, false);
    page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QSignalSpy loadFinishedSpy(&page, &QWebEnginePage::loadFinished);

    page.setUrl(QUrl("https://revoked.badssl.com"));
    if (!loadFinishedSpy.wait(10000)) {
        QVERIFY2(!page.error, "There shouldn't be any certificate error if not loaded due to missing internet access!");
        QSKIP("Couldn't load page from network, skipping test.");
    }

    // revoked certificate might not be reported as invalid by chromium and the load will silently succeed
    bool failed = !loadFinishedSpy.first().first().toBool(), hasError = bool(page.error);
    QCOMPARE(failed, hasError);
    if (hasError) {
        QVERIFY(!page.error->isOverridable());
        // Fatal certificate errors are implicitly rejected. But second call should not cause crash.
        page.error->rejectCertificate();
    }
}

QTEST_MAIN(tst_CertificateError)
#include <tst_certificateerror.moc>
