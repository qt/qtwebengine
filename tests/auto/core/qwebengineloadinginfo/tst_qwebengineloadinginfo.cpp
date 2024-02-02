// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineCore/qwebengineloadinginfo.h>
#include <QtWebEngineCore/qwebenginehttprequest.h>
#include <QtWebEngineCore/qwebenginesettings.h>

#include <httpserver.h>
#include <httpreqrep.h>

typedef QMultiMap<QByteArray, QByteArray> Map;

class tst_QWebEngineLoadingInfo : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineLoadingInfo() { }

public slots:
    void loadingInfoChanged(QWebEngineLoadingInfo loadingInfo)
    {
        const auto responseHeaders = loadingInfo.responseHeaders();
        QFETCH(Map, expected);

        if (loadingInfo.status() == QWebEngineLoadingInfo::LoadSucceededStatus
            || loadingInfo.status() == QWebEngineLoadingInfo::LoadFailedStatus) {
            if (!expected.empty())
                QCOMPARE(responseHeaders, expected);
        } else {
            QVERIFY(responseHeaders.size() == 0);
        }
    }

private Q_SLOTS:
    void responseHeaders_data()
    {
        QTest::addColumn<int>("responseCode");
        QTest::addColumn<Map>("input");
        QTest::addColumn<Map>("expected");

        const Map empty;
        const Map input {
            std::make_pair("header1", "value1"),
            std::make_pair("header2", "value2")
        };
        const Map expected {
            std::make_pair("header1", "value1"),
            std::make_pair("header2", "value2"),
            std::make_pair("Connection", "close")
        };


        QTest::newRow("with headers HTTP 200") << 200 << input << expected;
        QTest::newRow("with headers HTTP 500") << 500 << input << expected;
        QTest::newRow("without headers HTTP 200") << 200 << empty << empty;
        QTest::newRow("without headers HTTP 500") << 500 << empty << empty;
    }

    void responseHeaders()
    {
        HttpServer httpServer;

        QFETCH(Map, input);
        QFETCH(int, responseCode);
        QObject::connect(&httpServer, &HttpServer::newRequest, this, [&](HttpReqRep *rr) {
            for (auto it = input.cbegin(); it != input.cend(); ++it)
                rr->setResponseHeader(it.key(), it.value());

            rr->sendResponse(responseCode);
        });
        QVERIFY(httpServer.start());

        QWebEngineProfile profile;
        QWebEnginePage page(&profile);
        page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
        QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
        QObject::connect(&page, &QWebEnginePage::loadingChanged, this, &tst_QWebEngineLoadingInfo::loadingInfoChanged);


        QWebEngineHttpRequest request(httpServer.url("/somepage.html"));
        page.load(request);

        QTRY_VERIFY(spy.count() > 0);
        QVERIFY(httpServer.stop());
    }
};

QTEST_MAIN(tst_QWebEngineLoadingInfo)
#include "tst_qwebengineloadinginfo.moc"
