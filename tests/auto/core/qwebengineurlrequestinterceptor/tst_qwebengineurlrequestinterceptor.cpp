// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <util.h>
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineurlrequestinfo.h>
#include <QtWebEngineCore/qwebengineurlrequestinterceptor.h>
#include <QtWebEngineCore/qwebenginesettings.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineCore/qwebenginehttprequest.h>

#include <httpserver.h>
#include <httpreqrep.h>

class tst_QWebEngineUrlRequestInterceptor : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineUrlRequestInterceptor();
    ~tst_QWebEngineUrlRequestInterceptor();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void interceptRequest();
    void ipv6HostEncoding();
    void requestedUrl_data();
    void requestedUrl();
    void setUrlSameUrl_data();
    void setUrlSameUrl();
    void firstPartyUrl();
    void firstPartyUrlNestedIframes_data();
    void firstPartyUrlNestedIframes();
    void requestInterceptorByResourceType_data();
    void requestInterceptorByResourceType();
    void firstPartyUrlHttp();
    void headers();
    void customHeaders();
    void initiator();
    void jsServiceWorker();
    void replaceInterceptor_data();
    void replaceInterceptor();
    void replaceOnIntercept();
    void multipleRedirects();
    void profilePreventsPageInterception_data();
    void profilePreventsPageInterception();
};

tst_QWebEngineUrlRequestInterceptor::tst_QWebEngineUrlRequestInterceptor()
{
}

tst_QWebEngineUrlRequestInterceptor::~tst_QWebEngineUrlRequestInterceptor()
{
}

void tst_QWebEngineUrlRequestInterceptor::init()
{
}

void tst_QWebEngineUrlRequestInterceptor::cleanup()
{
}

void tst_QWebEngineUrlRequestInterceptor::initTestCase()
{
}

void tst_QWebEngineUrlRequestInterceptor::cleanupTestCase()
{
}

struct RequestInfo {
    RequestInfo(QWebEngineUrlRequestInfo &info)
        : requestUrl(info.requestUrl())
        , firstPartyUrl(info.firstPartyUrl())
        , initiator(info.initiator())
        , resourceType(info.resourceType())
        , headers(info.httpHeaders())
    {}

    QUrl requestUrl;
    QUrl firstPartyUrl;
    QUrl initiator;
    int resourceType;
    QHash<QByteArray, QByteArray> headers;
};

static const QUrl kRedirectUrl = QUrl("qrc:///resources/content.html");

Q_LOGGING_CATEGORY(lc, "qt.webengine.tests")

class TestRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    QList<RequestInfo> requestInfos;
    bool shouldRedirect = false;
    QUrl redirectUrl;
    QMap<QUrl, QSet<QUrl>> requestInitiatorUrls;
    QMap<QByteArray, QByteArray> headers;
    std::function<bool (QWebEngineUrlRequestInfo &)> onIntercept;

    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        QVERIFY(QThread::currentThread() == QCoreApplication::instance()->thread());
        qCDebug(lc) << this << "Type:" << info.resourceType() << info.requestMethod() << "Navigation:" << info.navigationType()
                    << info.requestUrl() << "Initiator:" << info.initiator();

        // Since 63 we also intercept some unrelated blob requests..
        if (info.requestUrl().scheme() == QLatin1String("blob"))
            return;

        if (onIntercept && !onIntercept(info))
            return;

        bool block = info.requestMethod() != QByteArrayLiteral("GET");
        bool redirect = shouldRedirect && info.requestUrl() != redirectUrl;

        // set additional headers if any required by test
        for (auto it = headers.begin(); it != headers.end(); ++it) info.setHttpHeader(it.key(), it.value());

        if (block) {
            info.block(true);
        } else if (redirect) {
            info.redirect(redirectUrl);
        }

        requestInitiatorUrls[info.requestUrl()].insert(info.initiator());
        requestInfos.append(info);

        // MEMO avoid unintentionally changing request when it is not needed for test logic
        //      since api behavior depends on 'changed' state of the info object
        Q_ASSERT(info.changed() == (block || redirect));
    }

    bool shouldSkipRequest(const RequestInfo &requestInfo)
    {
        if (requestInfo.resourceType ==  QWebEngineUrlRequestInfo::ResourceTypeMainFrame ||
                requestInfo.resourceType == QWebEngineUrlRequestInfo::ResourceTypeSubFrame)
            return false;

        // Skip import documents and sandboxed documents.
        // See Document::SiteForCookies() in chromium/third_party/blink/renderer/core/dom/document.cc.
        return requestInfo.firstPartyUrl == QUrl("");
    }

    QList<RequestInfo> getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceType type)
    {
        QList<RequestInfo> infos;

        foreach (auto requestInfo, requestInfos) {
            if (shouldSkipRequest(requestInfo))
                continue;

            if (type == requestInfo.resourceType)
                infos.append(requestInfo);
        }

        return infos;
    }

    bool hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceType type)
    {
        foreach (auto requestInfo, requestInfos) {
            if (shouldSkipRequest(requestInfo))
                continue;

            if (type == requestInfo.resourceType)
                return true;
        }

        return false;
    }

    TestRequestInterceptor(bool redirect = false, const QUrl &url = kRedirectUrl)
        : shouldRedirect(redirect), redirectUrl(url)
    {
    }
};

class TestMultipleRedirectsInterceptor : public QWebEngineUrlRequestInterceptor {
public:
    QList<RequestInfo> requestInfos;
    QMap<QUrl, QUrl> redirectPairs;
    int redirectCount = 0;
    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        QVERIFY(QThread::currentThread() == QCoreApplication::instance()->thread());
        qCDebug(lc) << this << "Type:" << info.resourceType() << info.requestMethod() << "Navigation:" << info.navigationType()
                    << info.requestUrl() << "Initiator:" << info.initiator();
        auto redirectUrl = redirectPairs.constFind(info.requestUrl());
        if (redirectUrl != redirectPairs.constEnd()) {
          info.redirect(redirectUrl.value());
          requestInfos.append(info);
          redirectCount++;
        }
    }

    TestMultipleRedirectsInterceptor()
    {
    }
};

class ConsolePage : public QWebEnginePage {
    Q_OBJECT
public:
    ConsolePage(QWebEngineProfile* profile) : QWebEnginePage(profile) {}

    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) override
    {
        levels.append(level);
        messages.append(message);
        lineNumbers.append(lineNumber);
        sourceIDs.append(sourceID);
    }

    QList<int> levels;
    QStringList messages;
    QList<int> lineNumbers;
    QStringList sourceIDs;
};

void tst_QWebEngineUrlRequestInterceptor::interceptRequest()
{
    QWebEngineProfile profile;
    profile.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setUrlRequestInterceptor(&interceptor);
    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.load(QUrl("qrc:///resources/index.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);
    QVariant success = loadSpy.takeFirst().takeFirst();
    QVERIFY(success.toBool());
    loadSpy.clear();
    QVariant ok;

    page.runJavaScript("post();", [&ok](const QVariant result){ ok = result; });
    QTRY_VERIFY(ok.toBool());
    QTRY_COMPARE(loadSpy.size(), 1);
    success = loadSpy.takeFirst().takeFirst();
    // We block non-GET requests, so this should not succeed.
    QVERIFY(!success.toBool());
    loadSpy.clear();

    interceptor.shouldRedirect = true;
    page.load(QUrl("qrc:///resources/__placeholder__"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);
    success = loadSpy.takeFirst().takeFirst();
    // The redirection for __placeholder__ should succeed.
    QVERIFY(success.toBool());
    loadSpy.clear();
    QCOMPARE(interceptor.requestInfos.size(), 4);

    // Make sure that registering an observer does not modify the request.
    TestRequestInterceptor observer(/* intercept */ false);
    profile.setUrlRequestInterceptor(&observer);
    page.load(QUrl("qrc:///resources/__placeholder__"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);
    success = loadSpy.takeFirst().takeFirst();
    // Since we do not intercept, loading an invalid path should not succeed.
    QVERIFY(!success.toBool());
    QCOMPARE(observer.requestInfos.size(), 1);
}

class LocalhostContentProvider : public QWebEngineUrlRequestInterceptor
{
public:
    LocalhostContentProvider() { }

    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        // Since 63 we also intercept the original data requests
        if (info.requestUrl().scheme() == QLatin1String("data"))
            return;
        if (info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeFavicon)
            return;

        requestedUrls.append(info.requestUrl());
        info.redirect(QUrl("data:text/html,<p>hello"));
    }

    QList<QUrl> requestedUrls;
};

void tst_QWebEngineUrlRequestInterceptor::ipv6HostEncoding()
{
    QWebEngineProfile profile;
    LocalhostContentProvider contentProvider;
    profile.setUrlRequestInterceptor(&contentProvider);

    QWebEnginePage page(&profile);
    QSignalSpy spyLoadFinished(&page, SIGNAL(loadFinished(bool)));

    page.setHtml("<p>Hi", QUrl::fromEncoded("http://[::1]/index.html"));
    QTRY_COMPARE(spyLoadFinished.size(), 1);
    QCOMPARE(contentProvider.requestedUrls.size(), 0);

    evaluateJavaScriptSync(&page, "var r = new XMLHttpRequest();"
            "r.open('GET', 'http://[::1]/test.xml', false);"
            "r.send(null);"
            );

    QCOMPARE(contentProvider.requestedUrls.size(), 1);
    QCOMPARE(contentProvider.requestedUrls.at(0), QUrl::fromEncoded("http://[::1]/test.xml"));
}

void tst_QWebEngineUrlRequestInterceptor::requestedUrl_data()
{
    QTest::addColumn<bool>("interceptInPage");
    QTest::newRow("profile intercept") << false;
    QTest::newRow("page intercept") << true;
}

void tst_QWebEngineUrlRequestInterceptor::requestedUrl()
{
    QFETCH(bool, interceptInPage);

    QWebEngineProfile profile;
    profile.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    TestRequestInterceptor interceptor(/* intercept */ true);
    if (!interceptInPage)
        profile.setUrlRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    if (interceptInPage)
        page.setUrlRequestInterceptor(&interceptor);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    page.setUrl(QUrl("qrc:///resources/__placeholder__"));
    QVERIFY(spy.wait());
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 20000);
    QVERIFY(interceptor.requestInfos.size() >= 1);
    QCOMPARE(interceptor.requestInfos.at(0).requestUrl, QUrl("qrc:///resources/content.html"));
    QCOMPARE(page.requestedUrl(), QUrl("qrc:///resources/__placeholder__"));
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));

    interceptor.shouldRedirect = false;

    page.setUrl(QUrl("qrc:/non-existent.html"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 2, 20000);
    QVERIFY(interceptor.requestInfos.size() >= 3);
    QCOMPARE(interceptor.requestInfos.at(2).requestUrl, QUrl("qrc:/non-existent.html"));
    QCOMPARE(page.requestedUrl(), QUrl("qrc:///resources/__placeholder__"));
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));

    page.setUrl(QUrl("http://abcdef.abcdef"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 3, 20000);
    QVERIFY(interceptor.requestInfos.size() >= 4);
    QCOMPARE(interceptor.requestInfos.at(3).requestUrl, QUrl("http://abcdef.abcdef/"));
    QCOMPARE(page.requestedUrl(), QUrl("qrc:///resources/__placeholder__"));
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));
}

void tst_QWebEngineUrlRequestInterceptor::setUrlSameUrl_data()
{
    requestedUrl_data();
}

void tst_QWebEngineUrlRequestInterceptor::setUrlSameUrl()
{
    QFETCH(bool, interceptInPage);

    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ true);
    if (!interceptInPage)
        profile.setUrlRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    if (interceptInPage)
        page.setUrlRequestInterceptor(&interceptor);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    page.setUrl(QUrl("qrc:///resources/__placeholder__"));
    QVERIFY(spy.wait());
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));
    QCOMPARE(spy.size(), 1);

    page.setUrl(QUrl("qrc:///resources/__placeholder__"));
    QVERIFY(spy.wait());
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));
    QCOMPARE(spy.size(), 2);

    // Now a case without redirect.
    page.setUrl(QUrl("qrc:///resources/content.html"));
    QVERIFY(spy.wait());
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));
    QCOMPARE(spy.size(), 3);

    page.setUrl(QUrl("qrc:///resources/__placeholder__"));
    QVERIFY(spy.wait());
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));
    QCOMPARE(spy.size(), 4);
}

void tst_QWebEngineUrlRequestInterceptor::firstPartyUrl()
{
    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setUrlRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    page.setUrl(QUrl("qrc:///resources/firstparty.html"));
    QVERIFY(spy.wait());
    QVERIFY(interceptor.requestInfos.size() >= 2);
    QCOMPARE(interceptor.requestInfos.at(0).requestUrl, QUrl("qrc:///resources/firstparty.html"));
    QCOMPARE(interceptor.requestInfos.at(1).requestUrl, QUrl("qrc:///resources/content.html"));
    QCOMPARE(interceptor.requestInfos.at(0).firstPartyUrl, QUrl("qrc:///resources/firstparty.html"));
    QCOMPARE(interceptor.requestInfos.at(1).firstPartyUrl, QUrl("qrc:///resources/firstparty.html"));
    QCOMPARE(spy.size(), 1);
}

void tst_QWebEngineUrlRequestInterceptor::firstPartyUrlNestedIframes_data()
{
    QUrl url = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                   + QLatin1String("/resources/iframe.html"));
    QTest::addColumn<QUrl>("requestUrl");
    QTest::newRow("ui file") << url;
    QTest::newRow("ui qrc") << QUrl("qrc:///resources/iframe.html");
}

void tst_QWebEngineUrlRequestInterceptor::firstPartyUrlNestedIframes()
{
    QFETCH(QUrl, requestUrl);

    if (requestUrl.scheme() == "file"
        && !QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);

    QString adjustedUrl = requestUrl.adjusted(QUrl::RemoveFilename).toString();

    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setUrlRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    page.setUrl(requestUrl);
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);

    QVERIFY(interceptor.requestInfos.size() >= 1);
    RequestInfo info = interceptor.requestInfos.at(0);
    QCOMPARE(info.requestUrl, requestUrl);
    QCOMPARE(info.firstPartyUrl, requestUrl);
    QCOMPARE(info.resourceType, QWebEngineUrlRequestInfo::ResourceTypeMainFrame);

    QVERIFY(interceptor.requestInfos.size() >= 2);
    info = interceptor.requestInfos.at(1);
    QCOMPARE(info.requestUrl, QUrl(adjustedUrl + "iframe2.html"));
    QCOMPARE(info.firstPartyUrl, requestUrl);
    QCOMPARE(info.resourceType, QWebEngineUrlRequestInfo::ResourceTypeSubFrame);

    QVERIFY(interceptor.requestInfos.size() >= 3);
    info = interceptor.requestInfos.at(2);
    QCOMPARE(info.requestUrl, QUrl(adjustedUrl + "iframe3.html"));
    QCOMPARE(info.firstPartyUrl, requestUrl);
    QCOMPARE(info.resourceType, QWebEngineUrlRequestInfo::ResourceTypeSubFrame);
}

void tst_QWebEngineUrlRequestInterceptor::requestInterceptorByResourceType_data()
{
    QUrl firstPartyUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                             + QLatin1String("/resources/resource_in_iframe.html"));
    QUrl styleRequestUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                               + QLatin1String("/resources/style.css"));
    QUrl scriptRequestUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                                + QLatin1String("/resources/script.js"));
    QUrl fontRequestUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                              + QLatin1String("/resources/fontawesome.woff"));
    QUrl xhrRequestUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                             + QLatin1String("/resources/test"));
    QUrl imageFirstPartyUrl =
            QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                + QLatin1String("/resources/image_in_iframe.html"));
    QUrl imageRequestUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                               + QLatin1String("/resources/icons/favicon.png"));
    QUrl mediaFirstPartyUrl =
            QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                + QLatin1String("/resources/media_in_iframe.html"));
    QUrl mediaRequestUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                               + QLatin1String("/resources/media.mp4"));
    QUrl faviconFirstPartyUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                                    + QLatin1String("/resources/favicon.html"));
    QUrl faviconRequestUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                                 + QLatin1String("/resources/icons/favicon.png"));

    QTest::addColumn<QUrl>("requestUrl");
    QTest::addColumn<QUrl>("firstPartyUrl");
    QTest::addColumn<int>("resourceType");

    QTest::newRow("StyleSheet")
            << styleRequestUrl << firstPartyUrl
            << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeStylesheet);
    QTest::newRow("Script") << scriptRequestUrl << firstPartyUrl
                                                  << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeScript);
    QTest::newRow("Image") << imageRequestUrl << imageFirstPartyUrl
                                                  << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeImage);
    QTest::newRow("FontResource")
            << fontRequestUrl << firstPartyUrl
            << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeFontResource);
    QTest::newRow(qPrintable("Media")) << mediaRequestUrl << mediaFirstPartyUrl
                                                  << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeMedia);
    QTest::newRow("Favicon")
            << faviconRequestUrl << faviconFirstPartyUrl
            << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeFavicon);
    QTest::newRow(qPrintable("Xhr")) << xhrRequestUrl << firstPartyUrl
                                                << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeXhr);
}

void tst_QWebEngineUrlRequestInterceptor::requestInterceptorByResourceType()
{
    if (!QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);
    QFETCH(QUrl, requestUrl);
    QFETCH(QUrl, firstPartyUrl);
    QFETCH(int, resourceType);

    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setUrlRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    page.setUrl(firstPartyUrl);
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);

    QTRY_COMPARE(interceptor.getUrlRequestForType(static_cast<QWebEngineUrlRequestInfo::ResourceType>(resourceType)).size(), 1);
    QList<RequestInfo> infos = interceptor.getUrlRequestForType(static_cast<QWebEngineUrlRequestInfo::ResourceType>(resourceType));
    QVERIFY(infos.size() >= 1);
    QCOMPARE(infos.at(0).requestUrl, requestUrl);
    QCOMPARE(infos.at(0).firstPartyUrl, firstPartyUrl);
    QCOMPARE(infos.at(0).resourceType, resourceType);
}

void tst_QWebEngineUrlRequestInterceptor::firstPartyUrlHttp()
{
    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setUrlRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QUrl firstPartyUrl = QUrl("https://www.w3schools.com/tags/tryit.asp?filename=tryhtml5_video");
    page.setUrl(QUrl(firstPartyUrl));
    if (!loadSpy.wait(15000) || !loadSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    QList<RequestInfo> infos;

    // Stylesheet
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeStylesheet));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeStylesheet);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // Script
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeScript));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeScript);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // Image
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeImage));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeImage);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // FontResource
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFontResource));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFontResource);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // Media
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeMedia));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeMedia);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // Favicon
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFavicon));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFavicon);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // XMLHttpRequest
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeXhr));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeXhr);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);
}

void tst_QWebEngineUrlRequestInterceptor::headers()
{
    HttpServer httpServer;
    httpServer.setResourceDirs({ QDir(QT_TESTCASE_SOURCEDIR).canonicalPath() + "/resources" });
    QVERIFY(httpServer.start());
    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(false);
    profile.setUrlRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    QWebEngineHttpRequest request(httpServer.url("/content.html"));
    request.setHeader("X-HEADERNAME", "HEADERVALUE");
    page.load(request);
    QVERIFY(spy.wait());
    QVERIFY(interceptor.requestInfos.last().headers.contains("X-HEADERNAME"));
    QCOMPARE(interceptor.requestInfos.last().headers.value("X-HEADERNAME"),
             QByteArray("HEADERVALUE"));

    bool jsFinished = false;

    page.runJavaScript(R"(
var request = new XMLHttpRequest();
request.open('GET', 'resource.html', /* async = */ false);
request.setRequestHeader('X-FOO', 'BAR');
request.send();
)",
                       [&](const QVariant &) { jsFinished = true; });
    QTRY_VERIFY(jsFinished);
    QVERIFY(interceptor.requestInfos.last().headers.contains("X-FOO"));
    QCOMPARE(interceptor.requestInfos.last().headers.value("X-FOO"), QByteArray("BAR"));
}

void tst_QWebEngineUrlRequestInterceptor::customHeaders()
{
    // Create HTTP Server to parse the request.
    HttpServer httpServer;
    httpServer.setResourceDirs({ QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                 + "/resources" });
    QVERIFY(httpServer.start());

    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(false);
    profile.setUrlRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    interceptor.headers = {
        { "referer", "http://somereferrer.com/" },
        { "from", "user@example.com" },
        { "user-agent", "mozilla/5.0 (x11; linux x86_64; rv:12.0) gecko/20100101 firefox/12.0" },
    };

    QMap<QByteArray, QByteArray> actual, expected;
    connect(&httpServer, &HttpServer::newRequest, [&] (HttpReqRep *rr) {
        for (auto it = expected.begin(); it != expected.end(); ++it) {
            auto headerValue = rr->requestHeader(it.key());
            actual[it.key()] = headerValue;
            QCOMPARE(headerValue, it.value());
        }
    });

    auto dumpHeaders = [&] () {
        QString s; QDebug d(&s);
        for (auto it = expected.begin(); it != expected.end(); ++it)
            d << "\n\tHeader:" << it.key() << "| actual:" << actual[it.key()] << "expected:" << it.value();
        return s;
    };

    expected = interceptor.headers;
    page.load(httpServer.url("/content.html"));
    QVERIFY(spy.wait());
    QVERIFY2(actual == expected, qPrintable(dumpHeaders()));

    // test that custom headers are also applied on redirect
    interceptor.shouldRedirect = true;
    interceptor.redirectUrl = httpServer.url("/content2.html");
    interceptor.headers = {
        { "referer", "http://somereferrer2.com/" },
        { "from", "user2@example.com" },
        { "user-agent", "mozilla/5.0 (compatible; googlebot/2.1; +http://www.google.com/bot.html)" },
    };

    actual.clear();
    expected = interceptor.headers;
    page.triggerAction(QWebEnginePage::Reload);
    QVERIFY(spy.wait());
    QVERIFY2(actual == expected, qPrintable(dumpHeaders()));

    (void) httpServer.stop();
}

void tst_QWebEngineUrlRequestInterceptor::initiator()
{
    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setUrlRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QUrl url = QUrl("https://www.w3schools.com/tags/tryit.asp?filename=tryhtml5_video");
    page.setUrl(QUrl(url));
    if (!loadSpy.wait(15000) || !loadSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    QList<RequestInfo> infos;

    // Stylesheet
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeStylesheet));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeStylesheet);
    foreach (auto info, infos)
        QVERIFY(interceptor.requestInitiatorUrls[info.requestUrl].contains(info.initiator));

    // Script
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeScript));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeScript);
    foreach (auto info, infos)
        QVERIFY(interceptor.requestInitiatorUrls[info.requestUrl].contains(info.initiator));

    // Image
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeImage));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeImage);
    foreach (auto info, infos)
        QVERIFY(interceptor.requestInitiatorUrls[info.requestUrl].contains(info.initiator));

    // FontResource
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFontResource));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFontResource);
    foreach (auto info, infos)
        QVERIFY(interceptor.requestInitiatorUrls[info.requestUrl].contains(info.initiator));

    // Media
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeMedia));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeMedia);
    foreach (auto info, infos)
        QVERIFY(interceptor.requestInitiatorUrls[info.requestUrl].contains(info.initiator));

    // Favicon
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFavicon));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFavicon);
    foreach (auto info, infos)
        QVERIFY(interceptor.requestInitiatorUrls[info.requestUrl].contains(info.initiator));

    // XMLHttpRequest
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeXhr));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeXhr);
    foreach (auto info, infos)
        QVERIFY(interceptor.requestInitiatorUrls[info.requestUrl].contains(info.initiator));
}

void tst_QWebEngineUrlRequestInterceptor::jsServiceWorker()
{

    HttpServer server;
    server.setResourceDirs({ QDir(QT_TESTCASE_SOURCEDIR).canonicalPath() + "/resources" });
    QVERIFY(server.start());
    QWebEngineProfile profile;
    std::unique_ptr<ConsolePage> page;
    page.reset(new ConsolePage(&profile));
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setUrlRequestInterceptor(&interceptor);
    QVERIFY(loadSync(page.get(), server.url("/sw.html")));

    // We expect only one message here, because logging of services workers is not exposed in our API.
    // Note this is very fragile setup , you need fresh profile otherwise install event might not get triggered
    // and this in turn can lead to incorrect intercepted requests, therefore we should keep this off the record.
    QTRY_COMPARE_WITH_TIMEOUT(page->messages.size(), 5, 20000);

    QCOMPARE(page->levels.at(0), QWebEnginePage::InfoMessageLevel);
    QCOMPARE(page->messages.at(0),QLatin1String("Service worker installing"));
    QCOMPARE(page->messages.at(1),QLatin1String("Service worker installed"));
    QCOMPARE(page->messages.at(2),QLatin1String("Service worker activating"));
    QCOMPARE(page->messages.at(3),QLatin1String("Service worker activated"));
    QCOMPARE(page->messages.at(4),QLatin1String("Service worker done"));
    QUrl firstPartyUrl = QUrl(server.url().toString() + "sw.html");
    QList<RequestInfo> infos;
    // Service Worker
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeServiceWorker));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeServiceWorker);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    QVERIFY(server.stop());
}

void tst_QWebEngineUrlRequestInterceptor::replaceInterceptor_data()
{
    QTest::addColumn<bool>("firstInterceptIsInPage");
    QTest::addColumn<bool>("keepInterceptionPoint");
    QTest::newRow("page")         << true << true;
    QTest::newRow("page-profile") << true << false;
    QTest::newRow("profile")      << false << true;
    QTest::newRow("profile-page") << false << false;
}

void tst_QWebEngineUrlRequestInterceptor::replaceInterceptor()
{
    QFETCH(bool, firstInterceptIsInPage);
    QFETCH(bool, keepInterceptionPoint);

    HttpServer server;
    server.setResourceDirs({ ":/resources" });
    QVERIFY(server.start());

    QWebEngineProfile profile;
    profile.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
    bool fetchFinished = false;

    auto setInterceptor = [&] (QWebEngineUrlRequestInterceptor *interceptor, bool interceptInPage) {
        interceptInPage ? page.setUrlRequestInterceptor(interceptor) : profile.setUrlRequestInterceptor(interceptor);
    };

    std::vector<TestRequestInterceptor> interceptors(3);
    std::vector<int> requestsOnReplace;
    setInterceptor(&interceptors.front(), firstInterceptIsInPage);

    auto sc = connect(&page, &QWebEnginePage::loadFinished, [&] () {
        auto currentInterceptorIndex = requestsOnReplace.size();
        requestsOnReplace.push_back(interceptors[currentInterceptorIndex].requestInfos.size());

        bool isFirstReinstall = currentInterceptorIndex == 0;
        bool interceptInPage = keepInterceptionPoint ? firstInterceptIsInPage : (isFirstReinstall ^ firstInterceptIsInPage);
        setInterceptor(&interceptors[++currentInterceptorIndex], interceptInPage);
        if (!keepInterceptionPoint)
            setInterceptor(nullptr, !interceptInPage);

        if (isFirstReinstall) {
            page.triggerAction(QWebEnginePage::Reload);
        } else {
            page.runJavaScript("fetch('http://nonexistent.invalid').catch(() => {})", [&, interceptInPage] (const QVariant &) {
                requestsOnReplace.push_back(interceptors.back().requestInfos.size());
                setInterceptor(nullptr, interceptInPage);
                fetchFinished = true;
            });
        }
    });

    page.setUrl(server.url("/favicon.html"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 2, 20000);
    QTRY_VERIFY(fetchFinished);

    QString s; QDebug d(&s);
    for (auto i = 0u; i < interceptors.size(); ++i) {
        auto &&interceptor = interceptors[i];
        auto &&requests = interceptor.requestInfos;
        d << "\nInterceptor [" << i << "] with" << requestsOnReplace[i] << "requests on replace and" << requests.size() << "in the end:";
        for (int j = 0; j < requests.size(); ++j) {
            auto &&r = requests[j];
            d << "\n\t" << j << "| url:" << r.requestUrl << "firstPartyUrl:" << r.firstPartyUrl;
        }
        QVERIFY2(!requests.isEmpty(), qPrintable(s));
        QVERIFY2(requests.size() == requestsOnReplace[i], qPrintable(s));
    }
}

void tst_QWebEngineUrlRequestInterceptor::replaceOnIntercept()
{
    HttpServer server;
    server.setResourceDirs({ ":/resources" });
    QVERIFY(server.start());

    QWebEngineProfile profile;
    profile.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    struct Interceptor : QWebEngineUrlRequestInterceptor {
        Interceptor(const std::function<void ()> &a) : action(a) { }
        void interceptRequest(QWebEngineUrlRequestInfo &) override { action(); }
        std::function<void ()> action;
        int interceptRequestReceived = 0;
    };

    TestRequestInterceptor profileInterceptor, pageInterceptor1, pageInterceptor2;
    page.setUrlRequestInterceptor(&pageInterceptor1);
    profile.setUrlRequestInterceptor(&profileInterceptor);
    profileInterceptor.onIntercept = [&] (QWebEngineUrlRequestInfo &) {
        page.setUrlRequestInterceptor(&pageInterceptor2);
        return true;
    };

    page.setUrl(server.url("/favicon.html"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 20000);
    QTRY_COMPARE(profileInterceptor.requestInfos.size(), 2);

    // if interceptor for page was replaced on intercept call in profile then, since request first
    // comes to profile, forward to page's interceptor should land to second one
    QCOMPARE(pageInterceptor1.requestInfos.size(), 0);
    QCOMPARE(profileInterceptor.requestInfos.size(), pageInterceptor2.requestInfos.size());

    page.setUrlRequestInterceptor(&pageInterceptor1);
    bool fetchFinished = false;
    page.runJavaScript("fetch('http://nonexistent.invalid').catch(() => {})", [&] (const QVariant &) {
        page.setUrlRequestInterceptor(&pageInterceptor2);
        fetchFinished = true;
    });

    QTRY_VERIFY(fetchFinished);
    QCOMPARE(profileInterceptor.requestInfos.size(), 3);
    QCOMPARE(pageInterceptor1.requestInfos.size(), 0);
    QCOMPARE(profileInterceptor.requestInfos.size(), pageInterceptor2.requestInfos.size());
}

void tst_QWebEngineUrlRequestInterceptor::multipleRedirects()
{
    HttpServer server;
    server.setResourceDirs({ ":/resources" });
    QVERIFY(server.start());

    TestMultipleRedirectsInterceptor multiInterceptor;
    multiInterceptor.redirectPairs.insert(QUrl(server.url("/content.html")), QUrl(server.url("/content2.html")));
    multiInterceptor.redirectPairs.insert(QUrl(server.url("/content2.html")), QUrl(server.url("/content3.html")));

    QWebEngineProfile profile;
    profile.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    profile.setUrlRequestInterceptor(&multiInterceptor);
    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    page.setUrl(server.url("/content.html"));

    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 20000);
    QTRY_COMPARE(multiInterceptor.redirectCount, 2);
    QTRY_COMPARE(multiInterceptor.requestInfos.size(), 2);
}

class PageOrProfileInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    PageOrProfileInterceptor(const QString &profileAction)
        : profileAction(profileAction)
    {
    }

    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        if (profileAction == "block")
            info.block(true);
        else if (profileAction == "redirect")
            info.redirect(QUrl("data:text/html,<p>redirected"));
        else if (profileAction == "add header")
            info.setHttpHeader("Custom-Header", "Value");
        else
            QVERIFY(info.httpHeaders().contains("Custom-Header"));
        ran = true;
    }

    QString profileAction;
    bool ran = false;
};

void tst_QWebEngineUrlRequestInterceptor::profilePreventsPageInterception_data()
{
    QTest::addColumn<QString>("profileAction");
    QTest::addColumn<bool>("interceptInProfile");
    QTest::addColumn<bool>("interceptInPage");
    QTest::newRow("block") << "block" << true << false;
    QTest::newRow("redirect") << "redirect" << true << false;
    QTest::newRow("add header") << "add header" << true << true;
}

void tst_QWebEngineUrlRequestInterceptor::profilePreventsPageInterception()
{
    QFETCH(QString, profileAction);
    QFETCH(bool, interceptInProfile);
    QFETCH(bool, interceptInPage);

    QWebEngineProfile profile;
    PageOrProfileInterceptor profileInterceptor(profileAction);
    profile.setUrlRequestInterceptor(&profileInterceptor);
    profile.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);

    QWebEnginePage page(&profile);
    PageOrProfileInterceptor pageInterceptor("");
    page.setUrlRequestInterceptor(&pageInterceptor);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.load(QUrl("qrc:///resources/index.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(profileInterceptor.ran, interceptInProfile);
    QCOMPARE(pageInterceptor.ran, interceptInPage);
}

QTEST_MAIN(tst_QWebEngineUrlRequestInterceptor)
#include "tst_qwebengineurlrequestinterceptor.moc"
