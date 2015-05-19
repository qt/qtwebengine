/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "../../widgets/util.h"
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebenginecallback.h>
#include <QtWebEngineCore/qwebenginecookiestoreclient.h>
#include <QtWebEngineWidgets/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebengineprofile.h>
#include <QtWebEngineWidgets/qwebengineview.h>

class tst_QWebEngineCookieStoreClient : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineCookieStoreClient();
    ~tst_QWebEngineCookieStoreClient();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cookieSignals();
    void setAndDeleteCookie();
    void batchCookieTasks();
};

tst_QWebEngineCookieStoreClient::tst_QWebEngineCookieStoreClient()
{
}

tst_QWebEngineCookieStoreClient::~tst_QWebEngineCookieStoreClient()
{
}

void tst_QWebEngineCookieStoreClient::init()
{
}

void tst_QWebEngineCookieStoreClient::cleanup()
{
}

void tst_QWebEngineCookieStoreClient::initTestCase()
{
}

void tst_QWebEngineCookieStoreClient::cleanupTestCase()
{
}

void tst_QWebEngineCookieStoreClient::cookieSignals()
{
    QWebEngineView view;
    QWebEngineCookieStoreClient client;

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    QSignalSpy cookieAddedSpy(&client, SIGNAL(cookieAdded(const QNetworkCookie &)));
    QSignalSpy cookieRemovedSpy(&client, SIGNAL(cookieRemoved(const QNetworkCookie &)));

    view.page()->profile()->setCookieStoreClient(&client);

    view.load(QUrl("qrc:///resources/index.html"));

    QTRY_COMPARE(loadSpy.count(), 1);
    QVariant success = loadSpy.takeFirst().takeFirst();
    QVERIFY(success.toBool());
    QTRY_COMPARE(cookieAddedSpy.count(), 2);

    // try whether updating a cookie to be expired results in that cookie being removed.
    QNetworkCookie expiredCookie(QNetworkCookie::parseCookies(QByteArrayLiteral("SessionCookie=delete; expires=Thu, 01-Jan-1970 00:00:00 GMT; path=///resources")).first());
    client.setCookie(expiredCookie, QUrl("qrc:///resources/index.html"));
    QTRY_COMPARE(cookieRemovedSpy.count(), 1);
    cookieRemovedSpy.clear();

    // try removing the other cookie.
    QNetworkCookie nonSessionCookie(QNetworkCookie::parseCookies(QByteArrayLiteral("CookieWithExpiresField=QtWebEngineCookieTest; path=///resources")).first());
    client.deleteCookie(nonSessionCookie, QUrl("qrc:///resources/index.html"));
    QTRY_COMPARE(cookieRemovedSpy.count(), 1);
}

void tst_QWebEngineCookieStoreClient::setAndDeleteCookie()
{
    QWebEngineView view;
    QWebEngineCookieStoreClient client;

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    QSignalSpy cookieAddedSpy(&client, SIGNAL(cookieAdded(const QNetworkCookie &)));
    QSignalSpy cookieRemovedSpy(&client, SIGNAL(cookieRemoved(const QNetworkCookie &)));

    QNetworkCookie cookie1(QNetworkCookie::parseCookies(QByteArrayLiteral("khaos=I9GX8CWI; Domain=.example.com; Path=/docs")).first());
    QNetworkCookie cookie2(QNetworkCookie::parseCookies(QByteArrayLiteral("Test%20Cookie=foobar; domain=example.com; Path=/")).first());
    QNetworkCookie cookie3(QNetworkCookie::parseCookies(QByteArrayLiteral("SessionCookie=QtWebEngineCookieTest; Path=///resources")).first());
    QNetworkCookie expiredCookie3(QNetworkCookie::parseCookies(QByteArrayLiteral("SessionCookie=delete; expires=Thu, 01-Jan-1970 00:00:00 GMT; path=///resources")).first());

    // check if pending cookies are set and removed
    client.setCookieWithCallback(cookie1, [](bool success) { QVERIFY(success); });
    client.setCookieWithCallback(cookie2, [](bool success) { QVERIFY(success); });
    client.deleteCookie(cookie1);

    view.page()->profile()->setCookieStoreClient(&client);
    view.load(QUrl("qrc:///resources/content.html"));

    QTRY_COMPARE(loadSpy.count(), 1);
    QVariant success = loadSpy.takeFirst().takeFirst();
    QVERIFY(success.toBool());
    QTRY_COMPARE(cookieAddedSpy.count(), 2);
    QTRY_COMPARE(cookieRemovedSpy.count(), 1);
    cookieAddedSpy.clear();
    cookieRemovedSpy.clear();

    client.setCookieWithCallback(cookie3, [](bool success) { QVERIFY(success); });
    // updating a cookie with an expired 'expires' field should remove the cookie with the same name
    client.setCookieWithCallback(expiredCookie3, [](bool success) { QVERIFY(success); });
    client.deleteCookie(cookie2);
    QTRY_COMPARE(cookieAddedSpy.count(), 1);
    QTRY_COMPARE(cookieRemovedSpy.count(), 2);
}

void tst_QWebEngineCookieStoreClient::batchCookieTasks()
{
    QWebEngineView view;
    QWebEngineCookieStoreClient client;

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    QSignalSpy cookieAddedSpy(&client, SIGNAL(cookieAdded(const QNetworkCookie &)));
    QSignalSpy cookieRemovedSpy(&client, SIGNAL(cookieRemoved(const QNetworkCookie &)));

    QNetworkCookie cookie1(QNetworkCookie::parseCookies(QByteArrayLiteral("khaos=I9GX8CWI; Domain=.example.com; Path=/docs")).first());
    QNetworkCookie cookie2(QNetworkCookie::parseCookies(QByteArrayLiteral("Test%20Cookie=foobar; domain=example.com; Path=/")).first());

    int capture = 0;

    client.setCookieWithCallback(cookie1, [&capture](bool success) { QVERIFY(success); ++capture; });
    client.setCookieWithCallback(cookie2, [&capture](bool success) { QVERIFY(success); ++capture; });

    view.page()->profile()->setCookieStoreClient(&client);
    view.load(QUrl("qrc:///resources/index.html"));

    QTRY_COMPARE(loadSpy.count(), 1);
    QVariant success = loadSpy.takeFirst().takeFirst();
    QVERIFY(success.toBool());
    QTRY_COMPARE(cookieAddedSpy.count(), 4);
    QTRY_COMPARE(cookieRemovedSpy.count(), 0);
    QTRY_COMPARE(capture, 2);
    capture = 0;

    cookieAddedSpy.clear();
    cookieRemovedSpy.clear();

    client.getAllCookies([&capture](const QByteArray& cookieLine) {
        ++capture;
        QCOMPARE(QNetworkCookie::parseCookies(cookieLine).count(), 4);
    });

    client.deleteSessionCookiesWithCallback([&capture](int numDeleted) {
        ++capture;
        QCOMPARE(numDeleted, 3);
    });

    client.deleteAllCookiesWithCallback([&capture](int numDeleted) {
        ++capture;
        QCOMPARE(numDeleted, 1);
    });

    QTRY_COMPARE(capture, 3);
}

QTEST_MAIN(tst_QWebEngineCookieStoreClient)
#include "tst_qwebenginecookiestoreclient.moc"
