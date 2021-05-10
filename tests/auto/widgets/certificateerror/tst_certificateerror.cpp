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
#include <httpsserver.h>
#include <util.h>

#include <QWebEngineCertificateError>
#include <QWebEnginePage>
#include <QWebEngineProfile>
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
    PageWithCertificateErrorHandler(bool defer, bool accept, QObject *p = nullptr)
        : QWebEnginePage(p), deferError(defer), acceptCertificate(accept)
        , loadSpy(this, &QWebEnginePage::loadFinished) {
    }

    bool deferError, acceptCertificate;

    QSignalSpy loadSpy;
    QScopedPointer<QWebEngineCertificateError> error;

    bool certificateError(const QWebEngineCertificateError &e) override {
        error.reset(new QWebEngineCertificateError(e));
        if (deferError)
            error->defer();
        return acceptCertificate;
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
    HttpsServer server;
    server.setExpectError(true);
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
        QVERIFY(page.error->deferred());
        QVERIFY(!page.error->answered());
        QCOMPARE(page.loadSpy.count(), 0);
        QCOMPARE(toPlainTextSync(&page), QString());

        if (acceptCertificate)
            page.error->ignoreCertificateError();
        else
            page.error->rejectCertificate();

        QVERIFY(page.error->answered());
        page.error.reset();
    }
    QTRY_COMPARE_WITH_TIMEOUT(page.loadSpy.count(), 1, 30000);
    QCOMPARE(page.loadSpy.takeFirst().value(0).toBool(), acceptCertificate);
    QCOMPARE(toPlainTextSync(&page), expectedContent);
}

void tst_CertificateError::fatalError()
{
    PageWithCertificateErrorHandler page(false, false);
    page.profile()->setUseForGlobalCertificateVerification();
    page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QSignalSpy loadFinishedSpy(&page, &QWebEnginePage::loadFinished);

    page.setUrl(QUrl("https://revoked.badssl.com"));
    if (!loadFinishedSpy.wait(10000))
        QSKIP("Couldn't load page from network, skipping test.");
    QTRY_VERIFY(page.error);
    QVERIFY(!page.error->isOverridable());

    // Fatal certificate errors are implicitly rejected. This should not cause crash.
    page.error->rejectCertificate();
}

QTEST_MAIN(tst_CertificateError)
#include <tst_certificateerror.moc>
