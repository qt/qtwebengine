// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <util.h>

#include <QtWebEngineCore/qwebenginewebauthuxrequest.h>
#include <QtWebEngineCore/qwebenginepage.h>

static const QString kWebAuthRequestHtml = R"(
<html>
<body>
    <script type="text/javascript">
        options = { "challenge": new ArrayBuffer("challenge") }
        navigator.credentials.get({ "publicKey": options })
    </script>
</body>
</html>
)";

class tst_QWebEngineWebAuthUxRequest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void basic();
};

void tst_QWebEngineWebAuthUxRequest::basic() {
#ifdef Q_OS_WIN
    // FIXME
    QSKIP("This test currently hangs indefinitely on Windows");
#endif
    QWebEnginePage page;
    QSignalSpy spy(&page, &QWebEnginePage::webAuthUxRequested);
    QUrl origin("http://localhost");
    page.setHtml(kWebAuthRequestHtml, origin);
    QVERIFY(spy.wait());
    auto *request = spy[0][0].value<QWebEngineWebAuthUxRequest *>();
    QVERIFY(request->userNames().empty());
    QCOMPARE(request->state(), QWebEngineWebAuthUxRequest::WebAuthUxState::Discovery);
    QCOMPARE(request->relyingPartyId(), "localhost");
    QCOMPARE(request->requestFailureReason(),
             QWebEngineWebAuthUxRequest::RequestFailureReason::Timeout);
    auto pinRequest = request->pinRequest();
    QCOMPARE(pinRequest.reason, QWebEngineWebAuthUxRequest::PinEntryReason::Set);
    QCOMPARE(pinRequest.error, QWebEngineWebAuthUxRequest::PinEntryError::NoError);
    QCOMPARE(pinRequest.minPinLength, 0);
    QCOMPARE(pinRequest.remainingAttempts, 0);
}

QTEST_MAIN(tst_QWebEngineWebAuthUxRequest)
#include "tst_qwebenginewebauthuxrequest.moc"
