// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <util.h>
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebenginecookiestore.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginepage.h>

#include "httpserver.h"
#include "httpreqrep.h"

// locally overwrite the default timeout of QTY_(COMPARE|VERIFY)
#define QWE_TRY_COMPARE(x, y) QTRY_COMPARE_WITH_TIMEOUT(x, y, 30000)
#define QWE_TRY_VERIFY(x) QTRY_VERIFY_WITH_TIMEOUT(x, 30000)

class tst_QWebEngineCookieStore : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineCookieStore();
    ~tst_QWebEngineCookieStore();

public Q_SLOTS:
    void init();
    void cleanup();

    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    // MEMO should be the first test of a testcase
    // as it checks storage manipulation without navigation
    void setAndDeleteCookie();

    void cookieSignals();
    void batchCookieTasks();
    void basicFilter();
    void basicFilterOverHTTP();
    void html5featureFilter();

private:
    QWebEngineProfile *m_profile;
};

tst_QWebEngineCookieStore::tst_QWebEngineCookieStore()
{
}

tst_QWebEngineCookieStore::~tst_QWebEngineCookieStore()
{
}

void tst_QWebEngineCookieStore::init()
{
}

void tst_QWebEngineCookieStore::cleanup()
{
    m_profile->cookieStore()->deleteAllCookies();
}

void tst_QWebEngineCookieStore::initTestCase()
{
    m_profile = new QWebEngineProfile;
}

void tst_QWebEngineCookieStore::cleanupTestCase()
{
    delete m_profile;
}

void tst_QWebEngineCookieStore::cookieSignals()
{
    QWebEnginePage page(m_profile);

    QWebEngineCookieStore *client = m_profile->cookieStore();

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy cookieAddedSpy(client, SIGNAL(cookieAdded(const QNetworkCookie &)));
    QSignalSpy cookieRemovedSpy(client, SIGNAL(cookieRemoved(const QNetworkCookie &)));

    page.load(QUrl("qrc:///resources/index.html"));

    QWE_TRY_COMPARE(loadSpy.size(), 1);
    QVariant success = loadSpy.takeFirst().takeFirst();
    QVERIFY(success.toBool());
    QWE_TRY_COMPARE(cookieAddedSpy.size(), 2);

    // try whether updating a cookie to be expired results in that cookie being removed.
    QNetworkCookie expiredCookie(QNetworkCookie::parseCookies(QByteArrayLiteral("SessionCookie=delete; expires=Thu, 01-Jan-1970 00:00:00 GMT; path=///resources")).first());
    client->setCookie(expiredCookie, QUrl("qrc:///resources/index.html"));

    QWE_TRY_COMPARE(cookieRemovedSpy.size(), 1);
    cookieRemovedSpy.clear();

    // try removing the other cookie.
    QNetworkCookie nonSessionCookie(QNetworkCookie::parseCookies(QByteArrayLiteral("CookieWithExpiresField=QtWebEngineCookieTest; path=///resources")).first());
    client->deleteCookie(nonSessionCookie, QUrl("qrc:///resources/index.html"));
    QWE_TRY_COMPARE(cookieRemovedSpy.size(), 1);
}

void tst_QWebEngineCookieStore::setAndDeleteCookie()
{
    QWebEnginePage page(m_profile);
    QWebEngineCookieStore *client = m_profile->cookieStore();

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy cookieAddedSpy(client, SIGNAL(cookieAdded(const QNetworkCookie &)));
    QSignalSpy cookieRemovedSpy(client, SIGNAL(cookieRemoved(const QNetworkCookie &)));

    QNetworkCookie cookie1(QNetworkCookie::parseCookies(QByteArrayLiteral("khaos=I9GX8CWI; Domain=.example.com; Path=/docs")).first());
    QNetworkCookie cookie2(QNetworkCookie::parseCookies(QByteArrayLiteral("Test%20Cookie=foobar; domain=example.com; Path=/")).first());
    QNetworkCookie cookie3(QNetworkCookie::parseCookies(QByteArrayLiteral("SessionCookie=QtWebEngineCookieTest; Path=///resources")).first());
    QNetworkCookie expiredCookie3(QNetworkCookie::parseCookies(QByteArrayLiteral("SessionCookie=delete; expires=Thu, 01-Jan-1970 00:00:00 GMT; path=///resources")).first());

    // force to init storage as it's done lazily upon first navigation
    client->loadAllCookies();
    // /* FIXME remove 'blank' navigation once loadAllCookies api is fixed
    page.load(QUrl("about:blank"));
    QWE_TRY_COMPARE(loadSpy.size(), 1);
    // */

    // check if pending cookies are set and removed
    client->setCookie(cookie1);
    client->setCookie(cookie2);
    QWE_TRY_COMPARE(cookieAddedSpy.size(), 2);
    client->deleteCookie(cookie1);
    QWE_TRY_COMPARE(cookieRemovedSpy.size(), 1);

    page.load(QUrl("qrc:///resources/content.html"));

    QWE_TRY_COMPARE(loadSpy.size(), 2);
    QVariant success = loadSpy.takeFirst().takeFirst();
    QVERIFY(success.toBool());
    QWE_TRY_COMPARE(cookieAddedSpy.size(), 2);
    QWE_TRY_COMPARE(cookieRemovedSpy.size(), 1);
    cookieAddedSpy.clear();
    cookieRemovedSpy.clear();

    client->setCookie(cookie3);
    QWE_TRY_COMPARE(cookieAddedSpy.size(), 1);
    // updating a cookie with an expired 'expires' field should remove the cookie with the same name
    client->setCookie(expiredCookie3);
    client->deleteCookie(cookie2);
    QWE_TRY_COMPARE(cookieAddedSpy.size(), 1);
    QWE_TRY_COMPARE(cookieRemovedSpy.size(), 2);
}

void tst_QWebEngineCookieStore::batchCookieTasks()
{
    QWebEnginePage page(m_profile);
    QWebEngineCookieStore *client = m_profile->cookieStore();

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy cookieAddedSpy(client, SIGNAL(cookieAdded(const QNetworkCookie &)));
    QSignalSpy cookieRemovedSpy(client, SIGNAL(cookieRemoved(const QNetworkCookie &)));

    QNetworkCookie cookie1(QNetworkCookie::parseCookies(QByteArrayLiteral("khaos=I9GX8CWI; Domain=.example.com; Path=/docs")).first());
    QNetworkCookie cookie2(QNetworkCookie::parseCookies(QByteArrayLiteral("Test%20Cookie=foobar; domain=example.com; Path=/")).first());

    // force to init storage as it's done lazily upon first navigation
    client->loadAllCookies();
    // /* FIXME remove 'blank' navigation once loadAllCookies api is fixed
    page.load(QUrl("about:blank"));
    QWE_TRY_COMPARE(loadSpy.size(), 1);
    // */

    client->setCookie(cookie1);
    client->setCookie(cookie2);
    QWE_TRY_COMPARE(cookieAddedSpy.size(), 2);

    page.load(QUrl("qrc:///resources/index.html"));

    QWE_TRY_COMPARE(loadSpy.size(), 2);
    QVariant success = loadSpy.takeFirst().takeFirst();
    QVERIFY(success.toBool());
    QWE_TRY_COMPARE(cookieAddedSpy.size(), 4);
    QWE_TRY_COMPARE(cookieRemovedSpy.size(), 0);

    cookieAddedSpy.clear();
    cookieRemovedSpy.clear();

    client->deleteSessionCookies();
    QWE_TRY_COMPARE(cookieRemovedSpy.size(), 3);

    client->deleteAllCookies();
    QWE_TRY_COMPARE(cookieRemovedSpy.size(), 4);
}

void tst_QWebEngineCookieStore::basicFilter()
{
    QWebEnginePage page(m_profile);
    QWebEngineCookieStore *client = m_profile->cookieStore();

    QAtomicInt accessTested = 0;
    client->setCookieFilter([&](const QWebEngineCookieStore::FilterRequest &){ ++accessTested; return true;});

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy cookieAddedSpy(client, SIGNAL(cookieAdded(const QNetworkCookie &)));
    QSignalSpy cookieRemovedSpy(client, SIGNAL(cookieRemoved(const QNetworkCookie &)));

    page.load(QUrl("qrc:///resources/index.html"));

    QWE_TRY_COMPARE(loadSpy.size(), 1);
    QVERIFY(loadSpy.takeFirst().takeFirst().toBool());
    QWE_TRY_COMPARE(cookieAddedSpy.size(), 2);
    QWE_TRY_COMPARE(accessTested.loadAcquire(), 2); // FIXME?

    client->deleteAllCookies();
    QWE_TRY_COMPARE(cookieRemovedSpy.size(), 2);

    client->setCookieFilter([&](const QWebEngineCookieStore::FilterRequest &){ ++accessTested; return false; });
    page.triggerAction(QWebEnginePage::ReloadAndBypassCache);
    QWE_TRY_COMPARE(loadSpy.size(), 1);
    QVERIFY(loadSpy.takeFirst().takeFirst().toBool());
    QWE_TRY_COMPARE(accessTested.loadAcquire(), 4); // FIXME?
    // Test cookies are NOT added:
    QTest::qWait(100);
    QCOMPARE(cookieAddedSpy.size(), 2);
}

void tst_QWebEngineCookieStore::basicFilterOverHTTP()
{
    QWebEnginePage page(m_profile);
    QWebEngineCookieStore *client = m_profile->cookieStore();

    QAtomicInt accessTested = 0;
    QList<QPair<QUrl, QUrl>> resourceFirstParty;
    client->setCookieFilter([&](const QWebEngineCookieStore::FilterRequest &request) {
        resourceFirstParty.append(qMakePair(request.origin, request.firstPartyUrl));
        ++accessTested;
        return true;
    });

    HttpServer httpServer;
    httpServer.setHostDomain(QString("sub.test.localhost"));
    QVERIFY(httpServer.start());

    QByteArray cookieRequestHeader;
    connect(&httpServer, &HttpServer::newRequest, [&cookieRequestHeader](HttpReqRep *rr) {
        if (rr->requestMethod() == "GET" && rr->requestPath() == "/test.html") {
            cookieRequestHeader = rr->requestHeader(QByteArrayLiteral("Cookie"));
            if (cookieRequestHeader.isEmpty())
                rr->setResponseHeader(QByteArrayLiteral("Set-Cookie"), QByteArrayLiteral("Test=test"));
            rr->setResponseBody("<head><link rel='icon' type='image/png' href='resources/Fav.png'/>"
                                "<title>Page with a favicon and an icon</title></head>"
                                "<body><img src='resources/Img.ico'></body>");
            rr->sendResponse();
        }
    });

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy cookieAddedSpy(client, SIGNAL(cookieAdded(const QNetworkCookie &)));
    QSignalSpy cookieRemovedSpy(client, SIGNAL(cookieRemoved(const QNetworkCookie &)));
    QSignalSpy serverSpy(&httpServer, SIGNAL(newRequest(HttpReqRep *)));

    QUrl firstPartyUrl = httpServer.url("/test.html");
    page.load(firstPartyUrl);

    QWE_TRY_COMPARE(loadSpy.size(), 1);
    QVERIFY(loadSpy.takeFirst().takeFirst().toBool());
    QWE_TRY_COMPARE(cookieAddedSpy.size(), 1);
    QWE_TRY_COMPARE(accessTested.loadAcquire(), 4);
    QVERIFY(cookieRequestHeader.isEmpty());

    QWE_TRY_COMPARE(serverSpy.size(), 3);

    page.triggerAction(QWebEnginePage::Reload);
    QWE_TRY_COMPARE(loadSpy.size(), 1);
    QVERIFY(loadSpy.takeFirst().takeFirst().toBool());
    QVERIFY(!cookieRequestHeader.isEmpty());
    QWE_TRY_COMPARE(cookieAddedSpy.size(), 1);
    QWE_TRY_COMPARE(accessTested.loadAcquire(), 6);

    QWE_TRY_COMPARE(serverSpy.size(), 5);

    client->deleteAllCookies();
    QWE_TRY_COMPARE(cookieRemovedSpy.size(), 1);

    client->setCookieFilter([&](const QWebEngineCookieStore::FilterRequest &request) {
        resourceFirstParty.append(qMakePair(request.origin, request.firstPartyUrl));
        ++accessTested;
        return false;
    });
    page.triggerAction(QWebEnginePage::ReloadAndBypassCache);
    QWE_TRY_COMPARE(loadSpy.size(), 1);
    QVERIFY(loadSpy.takeFirst().takeFirst().toBool());
    QVERIFY(cookieRequestHeader.isEmpty());
    // Test cookies are NOT added:
    QTest::qWait(100);
    QCOMPARE(cookieAddedSpy.size(), 1);
    QWE_TRY_COMPARE(accessTested.loadAcquire(), 9);

    QWE_TRY_COMPARE(serverSpy.size(), 7);

    page.triggerAction(QWebEnginePage::Reload);
    QWE_TRY_COMPARE(loadSpy.size(), 1);
    QVERIFY(loadSpy.takeFirst().takeFirst().toBool());
    QVERIFY(cookieRequestHeader.isEmpty());
    QCOMPARE(cookieAddedSpy.size(), 1);

    // Wait for last GET /favicon.ico
    QWE_TRY_COMPARE(serverSpy.size(), 9);
    (void) httpServer.stop();

    QCOMPARE(resourceFirstParty.size(), accessTested.loadAcquire());
    for (auto &&p : std::as_const(resourceFirstParty))
        QVERIFY2(p.second == firstPartyUrl,
                 qPrintable(QString("Resource [%1] has wrong firstPartyUrl: %2").arg(p.first.toString(), p.second.toString())));
}

void tst_QWebEngineCookieStore::html5featureFilter()
{
    QWebEnginePage page(m_profile);
    QWebEngineCookieStore *client = m_profile->cookieStore();

    QAtomicInt accessTested = 0;
    client->setCookieFilter([&](const QWebEngineCookieStore::FilterRequest &){ ++accessTested; return false;});

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.load(QUrl("qrc:///resources/content.html"));

    QWE_TRY_COMPARE(loadSpy.size(), 1);
    QVERIFY(loadSpy.takeFirst().takeFirst().toBool());
    QCOMPARE(accessTested.loadAcquire(), 0); // FIXME?
    QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(".*Uncaught SecurityError.*sessionStorage.*"));
    page.runJavaScript("sessionStorage.test = 5;");
    QWE_TRY_COMPARE(accessTested.loadAcquire(), 1);

    QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(".*Uncaught SecurityError.*sessionStorage.*"));
    QAtomicInt callbackTriggered = 0;
    page.runJavaScript("sessionStorage.test", [&](const QVariant &v) { QVERIFY(!v.isValid()); callbackTriggered = 1; });
    QWE_TRY_VERIFY(callbackTriggered);
}

QTEST_MAIN(tst_QWebEngineCookieStore)
#include "tst_qwebenginecookiestore.moc"
