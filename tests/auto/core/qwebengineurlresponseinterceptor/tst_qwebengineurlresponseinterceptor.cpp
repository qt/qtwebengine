// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest/QtTest>
#include <QtWebEngineCore/QWebEngineUrlResponseInterceptor>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWebEngineCore/QWebEnginePage>
#include <QtWebEngineCore/QWebEngineLoadingInfo>
#include <QtWebEngineCore/QWebEngineUrlResponseInfo>

#include <httpserver.h>
#include <httpreqrep.h>

class tst_QWebEngineUrlResponseInterceptor : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineUrlResponseInterceptor() { }
    ~tst_QWebEngineUrlResponseInterceptor() { }

public Q_SLOTS:
    void init() { }
    void cleanup() { }

private Q_SLOTS:
    void initTestCase() { }
    void cleanupTestCase() { }
    void interceptRequest_data();
    void interceptRequest();
};

Q_LOGGING_CATEGORY(lc, "qt.webengine.tests")

class Interceptor : public QWebEngineUrlResponseInterceptor
{
    Q_OBJECT

    QUrl m_receivedRequestUrl;
    QMultiHash<QByteArray, QByteArray> m_receivedRequestHeaders;

public:
    void interceptResponseHeaders(QWebEngineUrlResponseInfo &info) override
    {
        m_receivedRequestUrl = info.requestUrl();
        m_receivedRequestHeaders = info.requestHeaders();
        QHash<QByteArray, QByteArray> responseHeaders = info.responseHeaders();

        responseHeaders.insert(QByteArrayLiteral("ADDEDHEADER"), QByteArrayLiteral("ADDEDVALUE"));
        *(responseHeaders.find(QByteArrayLiteral("content-length"))) = QByteArrayLiteral("57");

        info.setResponseHeaders(responseHeaders);
    }

    void getReceivedRequest(QUrl *receivedRequestUrl,
                            QMultiHash<QByteArray, QByteArray> *receivedRequestHeaders)
    {
        *receivedRequestUrl = m_receivedRequestUrl;
        *receivedRequestHeaders = m_receivedRequestHeaders;
    }
};

void tst_QWebEngineUrlResponseInterceptor::interceptRequest_data()
{
    QTest::addColumn<bool>("withProfileInterceptor");
    QTest::newRow("with profile interceptor") << true;
    QTest::newRow("with page interceptor") << false;
}

void tst_QWebEngineUrlResponseInterceptor::interceptRequest()
{
    QFETCH(bool, withProfileInterceptor);

    HttpServer httpServer;
    QObject::connect(&httpServer, &HttpServer::newRequest, this, [&](HttpReqRep *rr) {
        if (rr->requestPath() == QByteArrayLiteral("/okay.html")) {
            rr->setResponseBody(QByteArrayLiteral(
                    "<html><script>console.log('hello world js!');</script></html>"));
            rr->sendResponse();
        }
    });
    QVERIFY(httpServer.start());

    const QUrl requestUrl = httpServer.url(QStringLiteral("/okay.html"));

    Interceptor interceptor;
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    if (withProfileInterceptor)
        profile.setUrlResponseInterceptor(&interceptor);
    else
        page.setUrlResponseInterceptor(&interceptor);

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    bool headersWereChanged = false;
    QObject::connect(
            &page, &QWebEnginePage::loadingChanged, this, [&](QWebEngineLoadingInfo loadingInfo) {
                const QHash<QByteArray, QByteArray> responseHeaders = loadingInfo.responseHeaders();
                bool contentLengthSizeChanged = false;
                bool additionalHeaderAdded = false;
                for (auto it = responseHeaders.constBegin(); it != responseHeaders.constEnd();
                     ++it) {
                    if (it.key() == QByteArrayLiteral("content-length")
                        && it.value() == QByteArrayLiteral("57"))
                        contentLengthSizeChanged = true;
                    if (it.key() == QByteArrayLiteral("ADDEDHEADER")
                        && it.value() == QByteArrayLiteral("ADDEDVALUE"))
                        additionalHeaderAdded = true;
                }

                if (contentLengthSizeChanged && additionalHeaderAdded)
                    headersWereChanged = true;
            });

    page.load(requestUrl);

    QVERIFY(loadSpy.wait());

    QUrl receivedRequestUrl;
    QMultiHash<QByteArray, QByteArray> receivedRequestHeaders;
    interceptor.getReceivedRequest(&receivedRequestUrl, &receivedRequestHeaders);

    bool receivedRequestHeadersContainsQtWebEngine = false;
    for (auto it = receivedRequestHeaders.cbegin(); it != receivedRequestHeaders.cend(); ++it) {
        if (it.value().contains("QtWebEngine/")) {
            receivedRequestHeadersContainsQtWebEngine = true;
            break;
        }
    }

    QVERIFY(headersWereChanged);
    QCOMPARE_EQ(receivedRequestUrl, requestUrl);
    QVERIFY(receivedRequestHeaders.size() != 0);
    QVERIFY(receivedRequestHeadersContainsQtWebEngine);
    QVERIFY(httpServer.stop());
}

QTEST_MAIN(tst_QWebEngineUrlResponseInterceptor)
#include "tst_qwebengineurlresponseinterceptor.moc"
