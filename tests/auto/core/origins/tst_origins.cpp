// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <util.h>
#include "httpserver.h"

#include <QtCore/qfile.h>
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineurlrequestinterceptor.h>
#include <QtWebEngineCore/qwebengineurlrequestjob.h>
#include <QtWebEngineCore/qwebengineurlscheme.h>
#include <QtWebEngineCore/qwebengineurlschemehandler.h>
#include <QtWebEngineCore/qwebenginesettings.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebengineview.h>

#if defined(WEBSOCKETS)
#include <QtWebSockets/qwebsocket.h>
#include <QtWebSockets/qwebsocketserver.h>
#include <QtWebChannel/qwebchannel.h>
#endif
#include <qaction.h>

#define QSL QStringLiteral
#define QBAL QByteArrayLiteral

Q_LOGGING_CATEGORY(lc, "qt.webengine.tests")

void registerSchemes()
{
    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax"));
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-Secure"));
        scheme.setFlags(QWebEngineUrlScheme::SecureScheme);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-Secure-ServiceWorkersAllowed"));
        scheme.setFlags(QWebEngineUrlScheme::SecureScheme | QWebEngineUrlScheme::ServiceWorkersAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-Local"));
        scheme.setFlags(QWebEngineUrlScheme::LocalScheme);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-LocalAccessAllowed"));
        scheme.setFlags(QWebEngineUrlScheme::LocalAccessAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-NoAccessAllowed"));
        scheme.setFlags(QWebEngineUrlScheme::NoAccessAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-ServiceWorkersAllowed"));
        scheme.setFlags(QWebEngineUrlScheme::ServiceWorkersAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-ViewSourceAllowed"));
        scheme.setFlags(QWebEngineUrlScheme::ViewSourceAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("HostSyntax"));
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("HostSyntax-ContentSecurityPolicyIgnored"));
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);
        scheme.setFlags(QWebEngineUrlScheme::ContentSecurityPolicyIgnored);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("HostAndPortSyntax"));
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort);
        scheme.setDefaultPort(42);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("HostPortAndUserInformationSyntax"));
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::HostPortAndUserInformation);
        scheme.setDefaultPort(42);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("redirect"));
        scheme.setFlags(QWebEngineUrlScheme::CorsEnabled);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("redirect-secure"));
        scheme.setFlags(QWebEngineUrlScheme::SecureScheme);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("redirect-local"));
        scheme.setFlags(QWebEngineUrlScheme::LocalScheme | QWebEngineUrlScheme::LocalAccessAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("cors"));
        scheme.setFlags(QWebEngineUrlScheme::CorsEnabled);
        QWebEngineUrlScheme::registerScheme(scheme);
    }
    {
        QWebEngineUrlScheme scheme(QBAL("secure-cors"));
        scheme.setFlags(QWebEngineUrlScheme::SecureScheme | QWebEngineUrlScheme::CorsEnabled);
        QWebEngineUrlScheme::registerScheme(scheme);
    }
    {
        QWebEngineUrlScheme scheme(QBAL("localaccess"));
        scheme.setFlags(QWebEngineUrlScheme::LocalAccessAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }
    {
        QWebEngineUrlScheme scheme(QBAL("local"));
        scheme.setFlags(QWebEngineUrlScheme::LocalScheme);
        QWebEngineUrlScheme::registerScheme(scheme);
    }
    {
        QWebEngineUrlScheme scheme(QBAL("local-localaccess"));
        scheme.setFlags(QWebEngineUrlScheme::LocalScheme | QWebEngineUrlScheme::LocalAccessAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }
    {
        QWebEngineUrlScheme scheme(QBAL("local-cors"));
        scheme.setFlags(QWebEngineUrlScheme::LocalScheme | QWebEngineUrlScheme::CorsEnabled);
        QWebEngineUrlScheme::registerScheme(scheme);
    }
    {
        QWebEngineUrlScheme scheme("fetchapi-allowed");
        scheme.setFlags(QWebEngineUrlScheme::CorsEnabled | QWebEngineUrlScheme::FetchApiAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }
    {
        QWebEngineUrlScheme scheme("fetchapi-not-allowed");
        QWebEngineUrlScheme::registerScheme(scheme);
    }
}
Q_CONSTRUCTOR_FUNCTION(registerSchemes)

class TstUrlSchemeHandler final : public QWebEngineUrlSchemeHandler {
    Q_OBJECT

public:
    TstUrlSchemeHandler(QWebEngineProfile *profile)
    {
        profile->installUrlSchemeHandler(QBAL("tst"), this);

        profile->installUrlSchemeHandler(QBAL("PathSyntax"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-Secure"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-Secure-ServiceWorkersAllowed"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-Local"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-LocalAccessAllowed"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-NoAccessAllowed"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-ServiceWorkersAllowed"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-ViewSourceAllowed"), this);
        profile->installUrlSchemeHandler(QBAL("HostSyntax"), this);
        profile->installUrlSchemeHandler(QBAL("HostSyntax-ContentSecurityPolicyIgnored"), this);
        profile->installUrlSchemeHandler(QBAL("HostAndPortSyntax"), this);
        profile->installUrlSchemeHandler(QBAL("HostPortAndUserInformationSyntax"), this);
        profile->installUrlSchemeHandler(QBAL("redirect"), this);
        profile->installUrlSchemeHandler(QBAL("redirect-secure"), this);
        profile->installUrlSchemeHandler(QBAL("redirect-local"), this);
        profile->installUrlSchemeHandler(QBAL("cors"), this);
        profile->installUrlSchemeHandler(QBAL("secure-cors"), this);
        profile->installUrlSchemeHandler(QBAL("localaccess"), this);
        profile->installUrlSchemeHandler(QBAL("local"), this);
        profile->installUrlSchemeHandler(QBAL("local-localaccess"), this);
        profile->installUrlSchemeHandler(QBAL("local-cors"), this);
    }

    QList<QUrl> &requests() { return m_requests; }

private:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QUrl url = job->requestUrl();
        m_requests << url;

        if (url.scheme().startsWith("redirect")) {
            QString path = url.path();
            int idx = path.indexOf(QChar('/'));
            if (idx > 0) {
                url.setScheme(path.first(idx));
                url.setPath(path.mid(idx, -1));
                job->redirect(url);
                return;
            }
        }

        QString pathPrefix = QDir(QT_TESTCASE_SOURCEDIR).canonicalPath();
        if (url.path().startsWith("/qtwebchannel/"))
            pathPrefix = QSL(":");
        QString pathSuffix = url.path();
        auto file = std::make_unique<QFile>(pathPrefix + pathSuffix, job);
        if (!file->open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to read data for:" << url << file->errorString();
            job->fail(QWebEngineUrlRequestJob::RequestFailed);
            return;
        }
        QByteArray mimeType = QBAL("text/html");
        if (pathSuffix.endsWith(QSL(".js")))
            mimeType = QBAL("application/javascript");
        else if (pathSuffix.endsWith(QSL(".css")))
            mimeType = QBAL("text/css");
        job->reply(mimeType, file.release());
    }

    QList<QUrl> m_requests;
};

class TestRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    TestRequestInterceptor() = default;
    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        qCDebug(lc) << this << "Type:" << info.resourceType() << info.requestMethod() << "Navigation:" << info.navigationType()
                    << info.requestUrl() << "Initiator:" << info.initiator();

        QUrl url = info.requestUrl();
        requests << url;
        if (url.scheme().startsWith("redirect")) {
            QString path = url.path();
            int idx = path.indexOf(QChar('/'));
            if (idx > 0) {
                url.setScheme(path.first(idx));
                url.setPath(path.mid(idx, -1));
                info.redirect(url);
            }
        }
    }
    QList<QUrl> requests;
};

class TestPage : public QWebEnginePage
{
public:
    TestPage(QWebEngineProfile *profile) : QWebEnginePage(profile, nullptr)
    {
    }
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel,
                                  const QString &message, int,
                                  const QString &) override
    {
        messages << message;
        qCDebug(lc) << message;
    }

    bool logContainsDoneMarker() const { return messages.contains("TEST:done"); }

    QString findResultInLog() const
    {
        // make sure we do not have some extra logs from blink
        for (auto message : messages) {
            QStringList s = message.split(':');
            if (s.size() > 1 && s[0] == "TEST")
                return s[1];
        }
        return QString();
    }

    void clearLog() { messages.clear(); }

private:
    QStringList messages;
};

class tst_Origins final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void jsUrlCanon();
    void jsUrlRelative();
    void jsUrlOrigin();
    void subdirWithAccess();
    void subdirWithoutAccess();
    void fileAccessRemoteUrl_data();
    void fileAccessRemoteUrl();
    void fileAccessLocalUrl_data();
    void fileAccessLocalUrl();
    void mixedSchemes_data();
    void mixedSchemes();
    void mixedSchemesWithCsp();
    void mixedXHR_data();
    void mixedXHR();
    void mixedContent_data();
    void mixedContent();
    void localMediaBlock_data();
    void localMediaBlock();
#if defined(WEBSOCKETS)
    void webSocket();
#endif
    void dedicatedWorker();
    void sharedWorker();
    void serviceWorker();
    void viewSource();
    void createObjectURL();
    void redirectScheme();
    void redirectSchemeLocal();
    void redirectSchemeSecure();
    void redirectInterceptor();
    void redirectInterceptorLocal();
    void redirectInterceptorSecure();
    void redirectInterceptorFile();
    void redirectInterceptorHttp();
    void fetchApiCustomUrl_data();
    void fetchApiCustomUrl();
    void fetchApiHttpUrl();

private:
    bool verifyLoad(const QUrl &url)
    {
        QSignalSpy spy(m_page, &QWebEnginePage::loadFinished);
        m_page->load(url);
        [&spy]() { QTRY_VERIFY_WITH_TIMEOUT(!spy.isEmpty(), 90000); }();
        return !spy.isEmpty() && spy.front().value(0).toBool();
    }

    QVariant eval(const QString &code)
    {
        return evaluateJavaScriptSync(m_page, code);
    }

    QWebEngineProfile m_profile;
    TestPage *m_page = nullptr;
    TstUrlSchemeHandler *m_handler = nullptr;
};

void tst_Origins::initTestCase()
{
    QTest::ignoreMessage(
            QtWarningMsg,
            QRegularExpression("Please register the custom scheme 'tst'.*"));

    m_handler = new TstUrlSchemeHandler(&m_profile);
}

void tst_Origins::cleanupTestCase()
{
    QVERIFY(!m_page);
    delete m_handler;
}

void tst_Origins::init()
{
    m_page = new TestPage(&m_profile);
}

void tst_Origins::cleanup()
{
    delete m_page;
    m_page = nullptr;
    m_handler->requests().clear();
}

// Test URL parsing and canonicalization in Blink. The implementation of this
// part is mostly shared between Blink and Chromium proper.
void tst_Origins::jsUrlCanon()
{
    QVERIFY(verifyLoad(QSL("about:blank")));

    // Standard schemes are biased towards the authority part.
    QCOMPARE(eval(QSL("new URL(\"http:foo/bar\").href")),    QVariant(QSL("http://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"http:/foo/bar\").href")),   QVariant(QSL("http://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"http://foo/bar\").href")),  QVariant(QSL("http://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"http:///foo/bar\").href")), QVariant(QSL("http://foo/bar")));

    // The file scheme is however a (particularly) special case.
#ifdef Q_OS_WIN
    QCOMPARE(eval(QSL("new URL(\"file:foo/bar\").href")),    QVariant(QSL("file://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file:/foo/bar\").href")),   QVariant(QSL("file://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file://foo/bar\").href")),  QVariant(QSL("file://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file:///foo/bar\").href")), QVariant(QSL("file:///foo/bar")));
#else
    QCOMPARE(eval(QSL("new URL(\"file:foo/bar\").href")),    QVariant(QSL("file:///foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file:/foo/bar\").href")),   QVariant(QSL("file:///foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file://foo/bar\").href")),  QVariant(QSL("file://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file:///foo/bar\").href")), QVariant(QSL("file:///foo/bar")));
#endif

    // The qrc scheme is a PathSyntax scheme, having only a path and nothing else.
    QCOMPARE(eval(QSL("new URL(\"qrc:foo/bar\").href")),    QVariant(QSL("qrc:foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"qrc:/foo/bar\").href")),   QVariant(QSL("qrc:/foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"qrc://foo/bar\").href")),  QVariant(QSL("qrc://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"qrc:///foo/bar\").href")), QVariant(QSL("qrc:///foo/bar")));

    // Same for unregistered schemes.
    QCOMPARE(eval(QSL("new URL(\"tst:foo/bar\").href")),    QVariant(QSL("tst:foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"tst:/foo/bar\").href")),   QVariant(QSL("tst:/foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"tst://foo/bar\").href")),  QVariant(QSL("tst://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"tst:///foo/bar\").href")), QVariant(QSL("tst:///foo/bar")));

    // A HostSyntax scheme is like http without the port & user information.
    QCOMPARE(eval(QSL("new URL(\"HostSyntax:foo/bar\").href")),     QVariant(QSL("hostsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostSyntax:foo:42/bar\").href")),  QVariant(QSL("hostsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostSyntax:a:b@foo/bar\").href")), QVariant(QSL("hostsyntax://foo/bar")));

    // A HostAndPortSyntax scheme is like http without the user information.
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:foo/bar\").href")),
             QVariant(QSL("hostandportsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:foo:41/bar\").href")),
             QVariant(QSL("hostandportsyntax://foo:41/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:foo:42/bar\").href")),
             QVariant(QSL("hostandportsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:a:b@foo/bar\").href")),
             QVariant(QSL("hostandportsyntax://foo/bar")));

    // A HostPortAndUserInformationSyntax scheme is exactly like http.
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:foo/bar\").href")),
             QVariant(QSL("hostportanduserinformationsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:foo:41/bar\").href")),
             QVariant(QSL("hostportanduserinformationsyntax://foo:41/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:foo:42/bar\").href")),
             QVariant(QSL("hostportanduserinformationsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:a:b@foo/bar\").href")),
             QVariant(QSL("hostportanduserinformationsyntax://a:b@foo/bar")));
}

// Test relative URL resolution.
void tst_Origins::jsUrlRelative()
{
    QVERIFY(verifyLoad(QSL("about:blank")));

    // Schemes with hosts, like http, work as expected.
    QCOMPARE(eval(QSL("new URL('bar', 'http://foo').href")), QVariant(QSL("http://foo/bar")));
    QCOMPARE(eval(QSL("new URL('baz', 'http://foo/bar').href")), QVariant(QSL("http://foo/baz")));
    QCOMPARE(eval(QSL("new URL('baz', 'http://foo/bar/').href")), QVariant(QSL("http://foo/bar/baz")));
    QCOMPARE(eval(QSL("new URL('/baz', 'http://foo/bar/').href")), QVariant(QSL("http://foo/baz")));
    QCOMPARE(eval(QSL("new URL('./baz', 'http://foo/bar/').href")), QVariant(QSL("http://foo/bar/baz")));
    QCOMPARE(eval(QSL("new URL('../baz', 'http://foo/bar/').href")), QVariant(QSL("http://foo/baz")));
    QCOMPARE(eval(QSL("new URL('../../baz', 'http://foo/bar/').href")), QVariant(QSL("http://foo/baz")));
    QCOMPARE(eval(QSL("new URL('//baz', 'http://foo/bar/').href")), QVariant(QSL("http://baz/")));

    // In the case of schemes without hosts, relative URLs only work if the URL
    // starts with a single slash -- and canonicalization does not guarantee
    // this. The following cases all fail with TypeErrors.
    QCOMPARE(eval(QSL("new URL('bar', 'tst:foo').href")), QVariant());
    QCOMPARE(eval(QSL("new URL('baz', 'tst:foo/bar').href")), QVariant());
    QCOMPARE(eval(QSL("new URL('bar', 'tst://foo').href")), QVariant());
    QCOMPARE(eval(QSL("new URL('bar', 'tst:///foo').href")), QVariant());

    // However, registered custom schemes have been patched to allow relative
    // URLs even without an initial slash.
    QCOMPARE(eval(QSL("new URL('bar', 'qrc:foo').href")), QVariant(QSL("qrc:bar")));
    QCOMPARE(eval(QSL("new URL('baz', 'qrc:foo/bar').href")), QVariant(QSL("qrc:foo/baz")));
    QCOMPARE(eval(QSL("new URL('bar', 'qrc://foo').href")), QVariant(QSL("qrc://bar")));
    QCOMPARE(eval(QSL("new URL('bar', 'qrc:///foo').href")), QVariant(QSL("qrc:///bar")));

    // With a slash it works the same as http except 'foo' is part of the path and not the host.
    QCOMPARE(eval(QSL("new URL('bar', 'qrc:/foo').href")), QVariant(QSL("qrc:/bar")));
    QCOMPARE(eval(QSL("new URL('bar', 'qrc:/foo/').href")), QVariant(QSL("qrc:/foo/bar")));
    QCOMPARE(eval(QSL("new URL('baz', 'qrc:/foo/bar').href")), QVariant(QSL("qrc:/foo/baz")));
    QCOMPARE(eval(QSL("new URL('baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/foo/bar/baz")));
    QCOMPARE(eval(QSL("new URL('/baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/baz")));
    QCOMPARE(eval(QSL("new URL('./baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/foo/bar/baz")));
    QCOMPARE(eval(QSL("new URL('../baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/foo/baz")));
    QCOMPARE(eval(QSL("new URL('../../baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/baz")));
    QCOMPARE(eval(QSL("new URL('../../../baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/baz")));

    // If the relative URL begins with >= 2 slashes, then the scheme is treated
    // not as a Syntax::Path scheme but as a Syntax::HostPortAndUserInformation
    // scheme.
    QCOMPARE(eval(QSL("new URL('//baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc://baz/")));
    QCOMPARE(eval(QSL("new URL('///baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc://baz/")));
}

// Test origin serialization in Blink, implemented by blink::KURL and
// blink::SecurityOrigin as opposed to GURL and url::Origin.
void tst_Origins::jsUrlOrigin()
{
    QVERIFY(verifyLoad(QSL("about:blank")));

    // For network protocols the origin string must include the domain and port.
    QCOMPARE(eval(QSL("new URL(\"http://foo.com/page.html\").origin")), QVariant(QSL("http://foo.com")));
    QCOMPARE(eval(QSL("new URL(\"https://foo.com/page.html\").origin")), QVariant(QSL("https://foo.com")));

    // Even though file URL can also have domains, these are not included in the
    // origin string by Chromium. The standard does not specify a value here,
    // but suggests 'null' (https://url.spec.whatwg.org/#origin).
    QCOMPARE(eval(QSL("new URL(\"file:/etc/passwd\").origin")), QVariant(QSL("file://")));
    QCOMPARE(eval(QSL("new URL(\"file://foo.com/etc/passwd\").origin")), QVariant(QSL("file://")));

    // Unregistered schemes behave like file.
    QCOMPARE(eval(QSL("new URL(\"tst:/banana\").origin")), QVariant(QSL("tst://")));
    QCOMPARE(eval(QSL("new URL(\"tst://foo.com/banana\").origin")), QVariant(QSL("tst://")));

    // The non-PathSyntax schemes should have hosts and potentially ports.
    QCOMPARE(eval(QSL("new URL(\"HostSyntax:foo:41/bar\").origin")),
             QVariant(QSL("hostsyntax://foo")));
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:foo:41/bar\").origin")),
             QVariant(QSL("hostandportsyntax://foo:41")));
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:foo:42/bar\").origin")),
             QVariant(QSL("hostandportsyntax://foo")));
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:foo:41/bar\").origin")),
             QVariant(QSL("hostportanduserinformationsyntax://foo:41")));
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:foo:42/bar\").origin")),
             QVariant(QSL("hostportanduserinformationsyntax://foo")));

    // A PathSyntax scheme should have a 'universal' origin.
    QCOMPARE(eval(QSL("new URL(\"PathSyntax:foo\").origin")), QVariant(QSL("pathsyntax:")));
    QCOMPARE(eval(QSL("new URL(\"qrc:/crysis.css\").origin")), QVariant(QSL("qrc:")));
    QCOMPARE(eval(QSL("new URL(\"qrc://foo.com/crysis.css\").origin")), QVariant(QSL("qrc:")));

    // The NoAccessAllowed flag forces opaque origins.
    QCOMPARE(eval(QSL("new URL(\"PathSyntax-NoAccessAllowed:foo\").origin")),
             QVariant(QSL("null")));
}

class ScopedAttribute {
public:
    ScopedAttribute(QWebEngineSettings *settings, QWebEngineSettings::WebAttribute attribute, bool newValue)
        : m_settings(settings)
        , m_attribute(attribute)
        , m_oldValue(m_settings->testAttribute(m_attribute))
    {
        m_settings->setAttribute(m_attribute, newValue);
    }
    ~ScopedAttribute()
    {
        m_settings->setAttribute(m_attribute, m_oldValue);
    }
private:
    QWebEngineSettings *m_settings;
    QWebEngineSettings::WebAttribute m_attribute;
    bool m_oldValue;
};

// Test same-origin policy of file, qrc and custom schemes.
//
// Note the test case involves the main page trying to load an iframe from a
// file that resides in a parent directory. This is just a small detail to
// demonstrate the difference with Firefox where such access is not allowed.
void tst_Origins::subdirWithAccess()
{
    ScopedAttribute sa(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    QVERIFY(verifyLoad("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                       + "/resources/subdir/index.html"));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));

    QVERIFY(verifyLoad(QSL("qrc:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));

    QVERIFY(verifyLoad(QSL("tst:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));
}

// In this variation the LocalContentCanAccessFileUrls attribute is disabled. As
// a result all file URLs will be considered to have unique/opaque origins, that
// is, they are not the 'same origin as' any other origin.
//
// Note that this applies only to file URLs and not qrc or custom schemes.
//
// See also (in Blink):
//   - the allow_file_access_from_file_urls option and
//   - the blink::SecurityOrigin::BlockLocalAccessFromLocalOrigin() method.
void tst_Origins::subdirWithoutAccess()
{
    ScopedAttribute sa(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, false);

    QVERIFY(verifyLoad("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                       + "/resources/subdir/index.html"));
    QCOMPARE(eval(QSL("msg[0]")), QVariant());
    QCOMPARE(eval(QSL("msg[1]")), QVariant());

    QVERIFY(verifyLoad(QSL("qrc:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));

    QVERIFY(verifyLoad(QSL("tst:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));
}

void tst_Origins::fileAccessRemoteUrl_data()
{
    QTest::addColumn<bool>("EnableAccess");
    QTest::addColumn<bool>("UserGesture");
    QTest::addRow("enabled, XHR") << true << false;
    QTest::addRow("enabled, link click") << true << true;
    QTest::addRow("disabled, XHR") << false << false;
    QTest::addRow("disabled, link click") << false << true;
}

void tst_Origins::fileAccessRemoteUrl()
{
    QFETCH(bool, EnableAccess);
    QFETCH(bool, UserGesture);

    QWebEngineView view;
    view.setPage(m_page);
    view.resize(800, 600);
    view.show();

    HttpServer server;
    server.setResourceDirs({ QDir(QT_TESTCASE_SOURCEDIR).canonicalPath() + "/resources" });
    QVERIFY(server.start());

    ScopedAttribute sa1(m_page->settings(), QWebEngineSettings::LocalContentCanAccessRemoteUrls, EnableAccess);
    ScopedAttribute sa2(m_page->settings(), QWebEngineSettings::ErrorPageEnabled, false);

    if (UserGesture) {
        QString remoteUrl(server.url("/link.html").toString());
#ifdef Q_OS_WIN
        QString localUrl("file:///" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/link.html?linkLocation=" + remoteUrl);
#else
        QString localUrl("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/link.html?linkLocation=" + remoteUrl);
#endif

        QVERIFY(verifyLoad(localUrl));

        QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, elementCenter(m_page, "link"));
        // Succeed independently of EnableAccess == false
        QTRY_COMPARE(m_page->url(), remoteUrl);

        // Back/forward navigation is also allowed, however they are not user gesture
        m_page->triggerAction(QWebEnginePage::Back);
        QTRY_COMPARE(m_page->url(), localUrl);
        m_page->triggerAction(QWebEnginePage::Forward);
        QTRY_COMPARE(m_page->url(), remoteUrl);
    } else {
        QVERIFY(verifyLoad("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/mixedXHR.html"));
        eval("sendXHR('" + server.url("/mixedXHR.txt").toString() + "')");
        QTRY_COMPARE(eval("result"), (EnableAccess ? QString("ok") : QString("error")));
    }
}

void tst_Origins::fileAccessLocalUrl_data()
{
    QTest::addColumn<bool>("EnableAccess");
    QTest::addColumn<bool>("UserGesture");
    QTest::addRow("enabled, XHR") << true << false;
    QTest::addRow("enabled, link click") << true << true;
    QTest::addRow("disabled, XHR") << false << false;
    QTest::addRow("disabled, link click") << false << true;
}

void tst_Origins::fileAccessLocalUrl()
{
    QFETCH(bool, EnableAccess);
    QFETCH(bool, UserGesture);

    QWebEngineView view;
    view.setPage(m_page);
    view.resize(800, 600);
    view.show();

    ScopedAttribute sa1(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, EnableAccess);
    ScopedAttribute sa2(m_page->settings(), QWebEngineSettings::ErrorPageEnabled, false);

    if (UserGesture) {
#ifdef Q_OS_WIN
        QString localUrl1("file:///" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/link.html?linkLocation=link.html");
        QString localUrl2("file:///" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/link.html");
#else
        QString localUrl1("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/link.html?linkLocation=link.html");
        QString localUrl2("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/link.html");
#endif

        QVERIFY(verifyLoad(localUrl1));
        QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, elementCenter(m_page, "link"));
        // Succeed independently of EnableAccess == false
        QTRY_COMPARE(m_page->url(), localUrl2);

        // Back/forward navigation is also allowed, however they are not user gesture
        m_page->triggerAction(QWebEnginePage::Back);
        QTRY_COMPARE(m_page->url(), localUrl1);
        m_page->triggerAction(QWebEnginePage::Forward);
        QTRY_COMPARE(m_page->url(), localUrl2);
    } else {
        QVERIFY(verifyLoad("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/mixedXHR.html"));
        eval("sendXHR('file:" +  QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/mixedXHR.txt" + "')");
        QTRY_COMPARE(eval("result"), (EnableAccess ? QString("ok") : QString("error")));
    }
}

// Load the main page over one scheme with an iframe over another scheme.
//
// For file and qrc schemes, the iframe should load but it should not be
// possible for scripts in different frames to interact.
//
// Additionally for unregistered custom schemes and custom schemes without
// LocalAccessAllowed it should not be possible to load an iframe over the
// file: scheme.
void tst_Origins::mixedSchemes_data()
{
    QTest::addColumn<QString>("schemeFrom");
    QTest::addColumn<QVariantMap>("testPairs");

    QVariant SLF = QVariant(QSL("canLoadAndAccess")), OK = QVariant(QSL("canLoadButNotAccess")),
             ERR = QVariant(QSL("cannotLoad"));
    std::vector<std::pair<const char *, std::vector<std::pair<const char *, QVariant>>>> data = {
        { "file",
          {
                  { "file", SLF },
                  { "qrc", OK },
                  { "tst", ERR },
          } },
        { "qrc",
          {
                  { "file", ERR },
                  { "qrc", SLF },
                  { "tst", OK },
          } },
        { "tst",
          {
                  { "file", ERR },
                  { "qrc", OK },
                  { "tst", SLF },
          } },
        { "PathSyntax",
          {
                  { "PathSyntax", SLF },
                  { "PathSyntax-Local", ERR },
                  { "PathSyntax-LocalAccessAllowed", OK },
                  { "PathSyntax-NoAccessAllowed", OK },
          } },
        { "PathSyntax-LocalAccessAllowed",
          {
                  { "PathSyntax", OK },
                  { "PathSyntax-Local", OK },
                  { "PathSyntax-LocalAccessAllowed", SLF },
                  { "PathSyntax-NoAccessAllowed", OK },
          } },
        { "PathSyntax-NoAccessAllowed",
          {
                  { "PathSyntax", OK },
                  { "PathSyntax-Local", ERR },
                  { "PathSyntax-LocalAccessAllowed", OK },
                  { "PathSyntax-NoAccessAllowed", OK },
          } },
        { "HostSyntax://a",
          {
                  { "HostSyntax://a", SLF },
                  { "HostSyntax://b", OK },
          } },
        { "local-localaccess",
          {
                  { "local-cors", OK },
                  { "local-localaccess", SLF },
                  { "local", OK },
          } },
        { "local-cors",
          {
                  { "local", OK },
                  { "local-cors", SLF },
          } },
    };

    for (auto &&d : data) {
        auto schemeFrom = d.first;
        QVariantMap testPairs;
        for (auto &&destSchemes : d.second) {
            auto &&destScheme = destSchemes.first;
            testPairs[destScheme] = destSchemes.second;
        }
        QTest::addRow("%s", schemeFrom) << schemeFrom << testPairs;
    }
}

static QStringList protocolAndHost(const QString scheme)
{
    static QString srcDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath());
    QStringList result;
    if (scheme == QSL("file")) {
        return QStringList{ scheme, srcDir };
    }
    if (scheme.contains(QSL("HostSyntax:"))) {
        const QStringList &res = scheme.split(':');
        Q_ASSERT(res.size() == 2);
        return res;
    }
    return QStringList{ scheme, "" };
}

void tst_Origins::mixedSchemes()
{
    QFETCH(QString, schemeFrom);
    QFETCH(QVariantMap, testPairs);

    ScopedAttribute sa(m_page->settings(), QWebEngineSettings::ErrorPageEnabled, false);
    QString srcDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath());
    QString host;
    auto pah = protocolAndHost(schemeFrom);
    auto loadUrl = QString("%1:%2/resources/mixedSchemes.html").arg(pah[0]).arg(pah[1]);
    QVERIFY(verifyLoad(loadUrl));

    QStringList schemesTo, expected, results;
    for (auto it = testPairs.begin(), end = testPairs.end(); it != end; ++it) {

        auto schemeTo = it.key();
        auto pah = protocolAndHost(schemeTo);
        auto expectedResult = it.value().toString();
        auto frameUrl = QString("%1:%2/resources/mixedSchemes_frame.html").arg(pah[0]).arg(pah[1]);
        auto imgUrl = QString("%1:%2/resources/red.png").arg(pah[0]).arg(pah[1]);

        eval(QString("setIFrameUrl('%1','%2')").arg(frameUrl).arg(imgUrl));

        // wait for token in the log
        QTRY_VERIFY(m_page->logContainsDoneMarker());
        const QString result = m_page->findResultInLog();
        m_page->clearLog();
        schemesTo.append(schemeTo.rightJustified(20));
        results.append(result.rightJustified(20));
        expected.append(expectedResult.rightJustified(20));
    }

    QVERIFY2(results == expected,
             qPrintable(QString("\nFrom '%1' to:\n\tScheme: %2\n\tActual: %3\n\tExpect: %4")
                                .arg(schemeFrom)
                                .arg(schemesTo.join(' '))
                                .arg(results.join(' '))
                                .arg(expected.join(' '))));
}

// Like mixedSchemes but adds a Content-Security-Policy: frame-src 'none' header.
void tst_Origins::mixedSchemesWithCsp()
{
    QVERIFY(verifyLoad(QSL("HostSyntax://a/resources/mixedSchemesWithCsp.html")));
    eval(QSL("setIFrameUrl('HostSyntax://a/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
    eval(QSL("setIFrameUrl('HostSyntax://b/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));

    QVERIFY(verifyLoad(QSL("HostSyntax-ContentSecurityPolicyIgnored://a/resources/mixedSchemesWithCsp.html")));
    eval(QSL("setIFrameUrl('HostSyntax-ContentSecurityPolicyIgnored://a/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadAndAccess")));
    eval(QSL("setIFrameUrl('HostSyntax-ContentSecurityPolicyIgnored://b/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
}

// Load the main page over one scheme, then make an XMLHttpRequest to a
// different scheme.
//
// Cross-origin XMLHttpRequests can only be made to CORS-enabled schemes. These
// include the builtin schemes http, https, data, and chrome, as well as custom
// schemes with the CorsEnabled flag.
void tst_Origins::mixedXHR_data()
{
    QTest::addColumn<QString>("schemeFrom");
    QTest::addColumn<bool>("canAccessFileUrls");
    QTest::addColumn<bool>("canAccessRemoteUrl");
    QTest::addColumn<QVariantMap>("testPairs");

    bool defaultFileAccess = QWebEnginePage().settings()->testAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls);
    bool defaultRemoteAccess = QWebEnginePage().settings()->testAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls);
    Q_ASSERT(defaultFileAccess);
    Q_ASSERT(!defaultRemoteAccess);
    std::vector<std::pair<bool, bool>> settingCombinations = {
        { defaultFileAccess, defaultRemoteAccess },  // tag: *schemeFrom*_local_noremote
        { defaultFileAccess, !defaultRemoteAccess }, // tag: *schemeFrom*_local_remote
        { !defaultFileAccess, defaultRemoteAccess }, // tag: *schemeFrom*_nolocal_noremote
        { !defaultFileAccess, !defaultRemoteAccess } // tag: *schemeFrom*_nolocal_remote
    };

    QVariant OK = QString("ok"), ERR = QString("error");
    std::vector<
        std::pair<const char *, std::vector<
            std::pair<const char *, std::vector<QVariant>>>>> data = {
        { "file", {
            { "file",               {  OK,  OK, ERR, ERR } },
            { "qrc",                { ERR, ERR, ERR, ERR } },
            { "tst",                { ERR, ERR, ERR, ERR } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               { ERR,  OK, ERR,  OK } },
            { "local-localaccess",  {  OK,  OK, ERR, ERR } },
            { "local-cors",         {  OK,  OK, ERR, ERR } }, } },

        { "qrc",  {
            { "file",               { ERR, ERR, ERR, ERR } },
            { "qrc",                {  OK,  OK,  OK,  OK } },
            { "tst",                { ERR, ERR, ERR, ERR } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               {  OK,  OK,  OK,  OK } },
            { "local-localaccess",  { ERR, ERR, ERR, ERR } },
            { "local-cors",         { ERR, ERR, ERR, ERR } }, } },

        { "tst",  {
            { "file",               { ERR, ERR, ERR, ERR } },
            { "qrc",                { ERR, ERR, ERR, ERR } },
            { "tst",                {  OK,  OK,  OK,  OK } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               {  OK,  OK,  OK,  OK } },
            { "local-localaccess",  { ERR, ERR, ERR, ERR } },
            { "local-cors",         { ERR, ERR, ERR, ERR } }, } },

        { "cors", {                 // -local +cors -local-access
            { "file",               { ERR, ERR, ERR, ERR } },
            { "qrc",                { ERR, ERR, ERR, ERR } },
            { "tst",                { ERR, ERR, ERR, ERR } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               {  OK,  OK,  OK,  OK } },
            { "local-localaccess",  { ERR, ERR, ERR, ERR } },
            { "local-cors",         { ERR, ERR, ERR, ERR } }, } },

        { "local", {                // +local -cors -local-access
            { "file",               {  OK,  OK, ERR, ERR } },
            { "qrc",                { ERR, ERR, ERR, ERR } },
            { "tst",                { ERR, ERR, ERR, ERR } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               { ERR,  OK, ERR,  OK } },
            { "local-localaccess",  {  OK,  OK, ERR, ERR } },
            { "local-cors",         {  OK,  OK, ERR, ERR } }, } },

        { "local-cors", {           // +local +cors -local-access
            { "file",               {  OK,  OK, ERR, ERR } },
            { "qrc",                { ERR, ERR, ERR, ERR } },
            { "tst",                { ERR, ERR, ERR, ERR } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               { ERR,  OK, ERR,  OK } },
            { "local-localaccess",  {  OK,  OK, ERR, ERR } },
            { "local-cors",         {  OK,  OK, ERR, ERR } }, } },

        { "local-localaccess", {    // +local -cors +local-access
            { "file",               {  OK,  OK,  OK,  OK } },
            { "qrc",                { ERR, ERR, ERR, ERR } },
            { "tst",                { ERR, ERR, ERR, ERR } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               { ERR,  OK, ERR,  OK } },
            { "local-localaccess",  {  OK,  OK,  OK,  OK } },
            { "local-cors",         {  OK,  OK,  OK,  OK } }, } },

        { "localaccess", {          // -local -cors +local-access
            { "file",               {  OK,  OK,  OK,  OK } },
            { "qrc",                { ERR, ERR, ERR, ERR } },
            { "tst",                { ERR, ERR, ERR, ERR } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               {  OK,  OK,  OK,  OK } },
            { "local-localaccess",  {  OK,  OK,  OK,  OK } },
            { "local-cors",         {  OK,  OK,  OK,  OK } }, } },
    };

    for (auto &&d : data) {
        auto schemeFrom = d.first;

        for (int i = 0; i < 4; ++i) {
            const auto &it = settingCombinations[i];
            bool canAccessFileUrls = it.first, canAccessRemoteUrl = it.second;

            QVariantMap testPairs;
            for (auto &&destSchemes : d.second) {
                auto &&destScheme = destSchemes.first;
                auto &&expectedResults = destSchemes.second;
                testPairs[destScheme] = expectedResults[i];
            }

            QTest::addRow("%s_%s_%s", schemeFrom, (canAccessFileUrls ? "local" : "nolocal"), (canAccessRemoteUrl ? "remote" : "noremote"))
                << schemeFrom << canAccessFileUrls << canAccessRemoteUrl << testPairs;
        }
    }
}

void tst_Origins::mixedXHR()
{
    QFETCH(QString, schemeFrom);
    QFETCH(bool, canAccessFileUrls);
    QFETCH(bool, canAccessRemoteUrl);
    QFETCH(QVariantMap, testPairs);

    QString srcDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath());
    auto loadUrl = QString("%1:%2/resources/mixedXHR.html").arg(schemeFrom).arg(schemeFrom == "file" ? srcDir : "");
    auto sendXHR = [&] (const QString &scheme) {
        if (scheme == "data")
            return QString("sendXHR('data:,ok')");
        return QString("sendXHR('%1:%2/resources/mixedXHR.txt')").arg(scheme).arg(scheme == "file" ? srcDir : "");
    };

    QCOMPARE(testPairs.size(), 7);
    ScopedAttribute sa0(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, canAccessFileUrls);
    ScopedAttribute sa1(m_page->settings(), QWebEngineSettings::LocalContentCanAccessRemoteUrls, canAccessRemoteUrl);
    QVERIFY(verifyLoad(loadUrl));

    QStringList schemesTo, expected, results;
    for (auto it = testPairs.begin(), end = testPairs.end(); it != end; ++it) {
        auto schemeTo = it.key();
        auto expectedResult = it.value().toString();
        auto command = sendXHR(schemeTo);

        eval(command);

        QTRY_COMPARE(eval(QSL("result !== undefined")), QVariant(true));
        auto result = eval(QSL("result")).toString();

        schemesTo.append(schemeTo.rightJustified(10));
        results.append(result.rightJustified(10));
        expected.append(expectedResult.rightJustified(10));
    }
    QVERIFY2(results == expected,
        qPrintable(QString("From '%1' to:\n\tScheme: %2\n\tActual: %3\n\tExpect: %4")
            .arg(schemeFrom).arg(schemesTo.join(' ')).arg(results.join(' ')).arg(expected.join(' '))));
}

// Load the main page over one scheme, then load an iframe over a different scheme. This load is not considered CORS.
void tst_Origins::mixedContent_data()
{
    QTest::addColumn<QString>("schemeFrom");
    QTest::addColumn<bool>("canAccessFileUrls");
    QTest::addColumn<bool>("canAccessRemoteUrl");
    QTest::addColumn<QVariantMap>("testPairs");

    bool defaultFileAccess = true;
    bool defaultRemoteAccess = false;
    std::vector<std::pair<bool, bool>> settingCombinations = {
        { defaultFileAccess, defaultRemoteAccess },  // tag: *schemeFrom*_local_noremote
        { defaultFileAccess, !defaultRemoteAccess }, // tag: *schemeFrom*_local_remote
        { !defaultFileAccess, defaultRemoteAccess }, // tag: *schemeFrom*_nolocal_noremote
        { !defaultFileAccess, !defaultRemoteAccess } // tag: *schemeFrom*_nolocal_remote
    };

    QVariant SLF = QVariant(QSL("canLoadAndAccess")), OK = QVariant(QSL("canLoadButNotAccess")), ERR = QVariant(QSL("cannotLoad"));
    std::vector<
        std::pair<const char *, std::vector<
            std::pair<const char *, std::vector<QVariant>>>>> data = {
        { "file", {
            { "file",               { SLF, SLF, ERR, ERR } },
            { "qrc",                {  OK,  OK,  OK,  OK } },
            { "tst",                { ERR,  OK, ERR,  OK } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               { ERR,  OK, ERR,  OK } },
            { "local-localaccess",  {  OK,  OK, ERR, ERR } },
            { "local-cors",         {  OK,  OK, ERR, ERR } },
        } },

        { "qrc",  {
            { "file",               { ERR, ERR, ERR, ERR } },
            { "qrc",                { SLF, SLF, SLF, SLF } },
            { "tst",                {  OK,  OK,  OK,  OK } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               {  OK,  OK,  OK,  OK } },
            { "local-localaccess",  { ERR, ERR, ERR, ERR } },
            { "local-cors",         { ERR, ERR, ERR, ERR } }, } },

        { "tst",  {
            { "file",               { ERR, ERR, ERR, ERR } },
            { "qrc",                {  OK,  OK,  OK,  OK } },
            { "tst",                { SLF, SLF, SLF, SLF } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               {  OK,  OK,  OK,  OK } },
            { "local-localaccess",  { ERR, ERR, ERR, ERR } },
            { "local-cors",         { ERR, ERR, ERR, ERR } }, } },

        { "cors", {                 // -local +cors -local-access
            { "file",               { ERR, ERR, ERR, ERR } },
            { "qrc",                {  OK,  OK,  OK,  OK } },
            { "tst",                {  OK,  OK,  OK,  OK } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               { SLF, SLF, SLF, SLF } },
            { "local-localaccess",  { ERR, ERR, ERR, ERR } },
            { "local-cors",         { ERR, ERR, ERR, ERR } }, } },

        { "local", {                // +local -cors -local-access
            { "file",               {  OK,  OK, ERR, ERR } },
            { "qrc",                {  OK,  OK,  OK,  OK } },
            { "tst",                { ERR,  OK, ERR,  OK } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               { ERR,  OK, ERR,  OK } },
            { "local-localaccess",  {  OK,  OK, ERR, ERR } },
            { "local-cors",         {  OK,  OK, ERR, ERR } },
        } },

        { "local-cors", {           // +local +cors -local-access
            { "file",               {  OK,  OK, ERR, ERR } },
            { "qrc",                {  OK,  OK,  OK,  OK } },
            { "tst",                { ERR,  OK, ERR,  OK } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               { ERR,  OK, ERR,  OK } },
            { "local-localaccess",  {  OK,  OK, ERR, ERR } },
            { "local-cors",         { SLF, SLF, ERR, ERR } },
        } },

        { "local-localaccess", {    // +local -cors + OK-access
            { "file",               {  OK,  OK,  OK,  OK } },
            { "qrc",                {  OK,  OK,  OK,  OK } },
            { "tst",                { ERR,  OK, ERR,  OK } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               { ERR,  OK, ERR,  OK } },
            { "local-localaccess",  { SLF, SLF,  OK,  OK } }, // ### should probably be: SLF, SLF, SLF, SLF
            { "local-cors",         {  OK,  OK,  OK,  OK } },
        } },

        { "localaccess", {          // -local -cors +local-access
            { "file",               {  OK,  OK,  OK,  OK } },
            { "qrc",                {  OK,  OK,  OK,  OK } },
            { "tst",                {  OK,  OK,  OK,  OK } },
            { "data",               {  OK,  OK,  OK,  OK } },
            { "cors",               {  OK,  OK,  OK,  OK } },
            { "local-localaccess",  {  OK,  OK,  OK,  OK } },
            { "local-cors",         {  OK,  OK,  OK,  OK } }, } },
    };

    for (auto &&d : data) {
        auto schemeFrom = d.first;

        for (int i = 0; i < 4; ++i) {
            const auto &it = settingCombinations[i];
            bool canAccessFileUrls = it.first, canAccessRemoteUrl = it.second;

            QVariantMap testPairs;
            for (auto &&destSchemes : d.second) {
                auto &&destScheme = destSchemes.first;
                auto &&expectedResults = destSchemes.second;
                testPairs[destScheme] = expectedResults[i];
            }

            QTest::addRow("%s_%s_%s", schemeFrom, (canAccessFileUrls ? "local" : "nolocal"), (canAccessRemoteUrl ? "remote" : "noremote"))
                << schemeFrom << canAccessFileUrls << canAccessRemoteUrl << testPairs;
        }
    }
}

void tst_Origins::mixedContent()
{
    QFETCH(QString, schemeFrom);
    QFETCH(bool, canAccessFileUrls);
    QFETCH(bool, canAccessRemoteUrl);
    QFETCH(QVariantMap, testPairs);

    QString srcDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath());
    auto loadUrl = QString("%1:%2/resources/mixedSchemes.html").arg(schemeFrom).arg(schemeFrom == "file" ? srcDir : "");

    QCOMPARE(testPairs.size(), 7);
    ScopedAttribute sa2(m_page->settings(), QWebEngineSettings::ErrorPageEnabled, false);
    ScopedAttribute sa0(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, canAccessFileUrls);
    ScopedAttribute sa1(m_page->settings(), QWebEngineSettings::LocalContentCanAccessRemoteUrls, canAccessRemoteUrl);
    QVERIFY(verifyLoad(loadUrl));

    auto setIFrameUrl = [&] (const QString &scheme) {
        if (scheme == "data")
            return QString("setIFrameUrl('data:,<script>var canary = true; parent.canary = "
                           "true</script>','data:image/png;base64, iVBORw0KGgoAAAANSUhEUgAAAAUA"
                           "AAAFCAYAAACNbyblAAAAHElEQVQI12P4//8/"
                           "w38GIAXDIBKE0DHxgljNBAAO9TXL0Y4OHwAAAABJRU5ErkJggg==')");
        auto frameUrl = QString("%1:%2/resources/mixedSchemes_frame.html").arg(scheme).arg(scheme == "file" ? srcDir : "");
        auto imgUrl =
                QString("%1:%2/resources/red.png").arg(scheme).arg(scheme == "file" ? srcDir : "");
        return QString("setIFrameUrl('%1','%2')").arg(frameUrl).arg(imgUrl);
    };

    m_page->clearLog();
    QStringList schemesTo, expected, results;
    for (auto it = testPairs.begin(), end = testPairs.end(); it != end; ++it) {

        auto schemeTo = it.key();
        auto expectedResult = it.value().toString();

        eval(setIFrameUrl(schemeTo));

        // wait for token in the log
        QTRY_VERIFY(m_page->logContainsDoneMarker());
        const QString result = m_page->findResultInLog();
        m_page->clearLog();
        schemesTo.append(schemeTo.rightJustified(20));
        results.append(result.rightJustified(20));
        expected.append(expectedResult.rightJustified(20));
    }
    QVERIFY2(results == expected,
        qPrintable(QString("\nFrom '%1' to:\n\tScheme: %2\n\tActual: %3\n\tExpect: %4")
            .arg(schemeFrom).arg(schemesTo.join(' ')).arg(results.join(' ')).arg(expected.join(' '))));
}

#if defined(WEBSOCKETS)
class EchoServer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)
public:
    EchoServer() : webSocketServer(QSL("EchoServer"), QWebSocketServer::NonSecureMode)
    {
        connect(&webSocketServer, &QWebSocketServer::newConnection, this, &EchoServer::onNewConnection);
    }

    bool listen()
    {
        if (webSocketServer.listen(QHostAddress::Any)) {
            Q_EMIT urlChanged();
            return true;
        }
        return false;
    }

    QUrl url() const
    {
        return webSocketServer.serverUrl();
    }

Q_SIGNALS:
    void urlChanged();

private:
    void onNewConnection()
    {
        QWebSocket *socket = webSocketServer.nextPendingConnection();
        connect(socket, &QWebSocket::textMessageReceived, this, &EchoServer::onTextMessageReceived);
        connect(socket, &QWebSocket::disconnected, socket, &QObject::deleteLater);
    }

    void onTextMessageReceived(const QString &message)
    {
        QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
        socket->sendTextMessage(message);
    }

    QWebSocketServer webSocketServer;
};

// Try opening a WebSocket from pages loaded over various URL schemes.
void tst_Origins::webSocket()
{
    EchoServer echoServer;
    QWebChannel channel;
    channel.registerObject(QSL("echoServer"), &echoServer);
    m_page->setWebChannel(&channel);
    QVERIFY(echoServer.listen());

    QVERIFY(verifyLoad("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                       + "/resources/websocket.html"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));

    QVERIFY(verifyLoad(QSL("qrc:/resources/websocket.html")));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));

    // Unregistered schemes can also open WebSockets (since Chromium 71)
    QVERIFY(verifyLoad(QSL("tst:/resources/websocket2.html")));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));

    // Even an insecure registered scheme can open WebSockets.
    QVERIFY(verifyLoad(QSL("PathSyntax:/resources/websocket2.html")));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));
}
#endif
// Create a (Dedicated)Worker. Since dedicated workers can only be accessed from
// one page, there is not much need for security restrictions.
void tst_Origins::dedicatedWorker()
{
    QVERIFY(verifyLoad("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                       + "/resources/dedicatedWorker.html"));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    QVERIFY(verifyLoad(QSL("qrc:/resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    // Unregistered schemes can also create Workers (since Chromium 71)
    QVERIFY(verifyLoad(QSL("tst:/resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    // Even an insecure registered scheme can create Workers.
    QVERIFY(verifyLoad(QSL("PathSyntax:/resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    // But not if the NoAccessAllowed flag is set.
    QVERIFY(verifyLoad(QSL("PathSyntax-NoAccessAllowed:/resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("cannot be accessed from origin 'null'")));
}

// Create a SharedWorker. Shared workers can be accessed from multiple pages,
// and therefore the same-origin policy applies.
void tst_Origins::sharedWorker()
{
    {
        ScopedAttribute sa(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, false);
        QVERIFY(verifyLoad("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/sharedWorker.html"));
        QTRY_VERIFY_WITH_TIMEOUT(eval(QSL("done")).toBool(), 10000);
        QVERIFY(eval(QSL("error")).toString()
                .contains(QSL("cannot be accessed from origin 'null'")));
    }

    {
        ScopedAttribute sa(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, true);
        QVERIFY(verifyLoad("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                           + "/resources/sharedWorker.html"));
        QTRY_VERIFY_WITH_TIMEOUT(eval(QSL("done")).toBool(), 10000);
        QCOMPARE(eval(QSL("result")), QVariant(42));
    }

    QVERIFY(verifyLoad(QSL("qrc:/resources/sharedWorker.html")));
    QTRY_VERIFY_WITH_TIMEOUT(eval(QSL("done")).toBool(), 10000);
    QCOMPARE(eval(QSL("result")), QVariant(42));

    // Unregistered schemes should not create SharedWorkers.

    QVERIFY(verifyLoad(QSL("PathSyntax:/resources/sharedWorker.html")));
    QTRY_VERIFY_WITH_TIMEOUT(eval(QSL("done")).toBool(), 10000);
    QCOMPARE(eval(QSL("result")), QVariant(42));

    QVERIFY(verifyLoad(QSL("PathSyntax-NoAccessAllowed:/resources/sharedWorker.html")));
    QTRY_VERIFY_WITH_TIMEOUT(eval(QSL("done")).toBool(), 10000);
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("denied to origin 'null'")));
}

// Service workers have to be explicitly enabled for a scheme.
void tst_Origins::serviceWorker()
{
    QVERIFY(verifyLoad("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                       + "/resources/serviceWorker.html"));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("The URL protocol of the current origin ('file://') is not supported.")));

    QVERIFY(verifyLoad(QSL("qrc:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("The URL protocol of the current origin ('qrc:') is not supported.")));

    QVERIFY(verifyLoad(QSL("tst:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
             .contains(QSL("Cannot read properties of undefined")));

    QVERIFY(verifyLoad(QSL("PathSyntax:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("Cannot read properties of undefined")));

    QVERIFY(verifyLoad(QSL("PathSyntax-Secure:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("The URL protocol of the current origin ('pathsyntax-secure:') is not supported.")));

    QVERIFY(verifyLoad(QSL("PathSyntax-ServiceWorkersAllowed:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("Cannot read properties of undefined")));

    QVERIFY(verifyLoad(QSL("PathSyntax-Secure-ServiceWorkersAllowed:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("error")), QVariant());

    QVERIFY(verifyLoad(QSL("PathSyntax-NoAccessAllowed:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("Cannot read properties of undefined")));
}

// Support for view-source must be enabled explicitly.
void tst_Origins::viewSource()
{
    QVERIFY(verifyLoad("view-source:file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                       + "/resources/viewSource.html"));
#ifdef Q_OS_WIN
    QCOMPARE(m_page->requestedUrl().toString(),
             "file:///" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                     + "/resources/viewSource.html");
#else
    QCOMPARE(m_page->requestedUrl().toString(),
             "file://" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                     + "/resources/viewSource.html");
#endif

    QVERIFY(verifyLoad(QSL("view-source:qrc:/resources/viewSource.html")));
    QCOMPARE(m_page->requestedUrl().toString(), QSL("qrc:/resources/viewSource.html"));

    QVERIFY(verifyLoad(QSL("view-source:tst:/resources/viewSource.html")));
    QCOMPARE(m_page->requestedUrl().toString(), QSL("about:blank"));

    QVERIFY(verifyLoad(QSL("view-source:PathSyntax:/resources/viewSource.html")));
    QCOMPARE(m_page->requestedUrl().toString(), QSL("about:blank"));

    QVERIFY(verifyLoad(QSL("view-source:PathSyntax-ViewSourceAllowed:/resources/viewSource.html")));
    QCOMPARE(m_page->requestedUrl().toString(), QSL("pathsyntax-viewsourceallowed:/resources/viewSource.html"));
}

void tst_Origins::createObjectURL()
{
    // Legal for registered custom schemes.
    QVERIFY(verifyLoad(QSL("qrc:/resources/createObjectURL.html")));
    QVERIFY(eval(QSL("result")).toString().startsWith(QSL("blob:qrc:")));

    // Also legal for unregistered schemes (since Chromium 71)
    QVERIFY(verifyLoad(QSL("tst:/resources/createObjectURL.html")));
    QVERIFY(eval(QSL("result")).toString().startsWith(QSL("blob:tst:")));
}

void tst_Origins::redirectScheme()
{
    QVERIFY(verifyLoad(QSL("redirect:cors/resources/redirect.html")));
    eval("addStylesheetLink('redirect:cors/resources/redirect.css')");
    QTRY_COMPARE(m_handler->requests().size(), 4);
    QCOMPARE(m_handler->requests()[0], QUrl(QStringLiteral("redirect:cors/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[1], QUrl(QStringLiteral("cors:/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[2], QUrl(QStringLiteral("redirect:cors/resources/redirect.css")));
    QCOMPARE(m_handler->requests()[3], QUrl(QStringLiteral("cors:/resources/redirect.css")));

    QVERIFY(!verifyLoad(QSL("redirect:file/resources/redirect.html")));
    QVERIFY(!verifyLoad(QSL("redirect:local/resources/redirect.html")));
    QVERIFY(!verifyLoad(QSL("redirect:local-cors/resources/redirect.html")));
}

void tst_Origins::redirectSchemeLocal()
{
    QVERIFY(verifyLoad(QSL("redirect-local:local/resources/redirect.html")));
    eval("addStylesheetLink('redirect-local:local/resources/redirect.css')");
    QTRY_COMPARE(m_handler->requests().size(), 4);
    QCOMPARE(m_handler->requests()[0], QUrl(QStringLiteral("redirect-local:local/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[1], QUrl(QStringLiteral("local:/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[2], QUrl(QStringLiteral("redirect-local:local/resources/redirect.css")));
    QCOMPARE(m_handler->requests()[3], QUrl(QStringLiteral("local:/resources/redirect.css")));
}

void tst_Origins::redirectSchemeSecure()
{
    QVERIFY(verifyLoad(QSL("redirect-secure:secure-cors/resources/redirect.html")));
    eval("addStylesheetLink('redirect-secure:secure-cors/resources/redirect.css')");
    QTRY_COMPARE(m_handler->requests().size(), 4);
    QCOMPARE(m_handler->requests()[0], QUrl(QStringLiteral("redirect-secure:secure-cors/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[1], QUrl(QStringLiteral("secure-cors:/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[2], QUrl(QStringLiteral("redirect-secure:secure-cors/resources/redirect.css")));
    QCOMPARE(m_handler->requests()[3], QUrl(QStringLiteral("secure-cors:/resources/redirect.css")));
}

void tst_Origins::redirectInterceptor()
{
    TestRequestInterceptor interceptor;
    m_profile.setUrlRequestInterceptor(&interceptor);

    QVERIFY(verifyLoad(QSL("redirect:cors/resources/redirect.html")));
    eval("addStylesheetLink('redirect:cors/resources/redirect.css')");

    QTRY_COMPARE(interceptor.requests.size(), 4);
    QTRY_COMPARE(m_handler->requests().size(), 2);
    QCOMPARE(m_handler->requests()[0], QUrl(QStringLiteral("cors:/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[1], QUrl(QStringLiteral("cors:/resources/redirect.css")));

    QCOMPARE(interceptor.requests[0], QUrl(QStringLiteral("redirect:cors/resources/redirect.html")));
    QCOMPARE(interceptor.requests[1], QUrl(QStringLiteral("cors:/resources/redirect.html")));
    QCOMPARE(interceptor.requests[2], QUrl(QStringLiteral("redirect:cors/resources/redirect.css")));
    QCOMPARE(interceptor.requests[3], QUrl(QStringLiteral("cors:/resources/redirect.css")));

    QVERIFY(!verifyLoad(QSL("redirect:file/resources/redirect.html")));
    QVERIFY(!verifyLoad(QSL("redirect:local/resources/redirect.html")));
    QVERIFY(!verifyLoad(QSL("redirect:local-cors/resources/redirect.html")));
}

void tst_Origins::redirectInterceptorLocal()
{
    TestRequestInterceptor interceptor;
    m_profile.setUrlRequestInterceptor(&interceptor);

    QVERIFY(verifyLoad(QSL("redirect-local:local/resources/redirect.html")));
    eval("addStylesheetLink('redirect-local:local/resources/redirect.css')");

    QTRY_COMPARE(interceptor.requests.size(), 4);
    QTRY_COMPARE(m_handler->requests().size(), 2);
    QCOMPARE(m_handler->requests()[0], QUrl(QStringLiteral("local:/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[1], QUrl(QStringLiteral("local:/resources/redirect.css")));

    QCOMPARE(interceptor.requests[0], QUrl(QStringLiteral("redirect-local:local/resources/redirect.html")));
    QCOMPARE(interceptor.requests[1], QUrl(QStringLiteral("local:/resources/redirect.html")));
    QCOMPARE(interceptor.requests[2], QUrl(QStringLiteral("redirect-local:local/resources/redirect.css")));
    QCOMPARE(interceptor.requests[3], QUrl(QStringLiteral("local:/resources/redirect.css")));
}

void tst_Origins::redirectInterceptorSecure()
{
    TestRequestInterceptor interceptor;
    m_profile.setUrlRequestInterceptor(&interceptor);

    QVERIFY(verifyLoad(QSL("redirect-secure:secure-cors/resources/redirect.html")));
    eval("addStylesheetLink('redirect-secure:secure-cors/resources/redirect.css')");

    QTRY_COMPARE(interceptor.requests.size(), 4);
    QTRY_COMPARE(m_handler->requests().size(), 2);
    QCOMPARE(m_handler->requests()[0], QUrl(QStringLiteral("secure-cors:/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[1], QUrl(QStringLiteral("secure-cors:/resources/redirect.css")));

    QCOMPARE(interceptor.requests[0], QUrl(QStringLiteral("redirect-secure:secure-cors/resources/redirect.html")));
    QCOMPARE(interceptor.requests[1], QUrl(QStringLiteral("secure-cors:/resources/redirect.html")));
    QCOMPARE(interceptor.requests[2], QUrl(QStringLiteral("redirect-secure:secure-cors/resources/redirect.css")));
    QCOMPARE(interceptor.requests[3], QUrl(QStringLiteral("secure-cors:/resources/redirect.css")));
}

class TestRedirectInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    TestRedirectInterceptor() = default;
    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        qCDebug(lc) << this << "Type:" << info.resourceType() << info.requestMethod() << "Navigation:" << info.navigationType()
                    << info.requestUrl() << "Initiator:" << info.initiator();

        QUrl url = info.requestUrl();
        requests << url;
        if (url.path().startsWith("/redirect")) {
            QString path = url.path();
            int idx = path.indexOf(QChar('/'), 10);
            if (idx > 0) {
                url.setScheme(path.mid(10, idx - 10));
                url.setPath(path.mid(idx, -1));
                url.setHost({});
                info.redirect(url);
            }
        }
    }
    QList<QUrl> requests;
};

void tst_Origins::redirectInterceptorFile()
{
    TestRedirectInterceptor interceptor;
    m_profile.setUrlRequestInterceptor(&interceptor);

    QVERIFY(verifyLoad(QSL("file:///redirect/local-cors/resources/redirect.html")));
    eval("addStylesheetLink('file:///redirect/local-cors/resources/redirect.css')");

    QTRY_COMPARE(interceptor.requests.size(), 4);
    QTRY_COMPARE(m_handler->requests().size(), 2);
    QCOMPARE(m_handler->requests()[0], QUrl(QStringLiteral("local-cors:/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[1], QUrl(QStringLiteral("local-cors:/resources/redirect.css")));

    QCOMPARE(interceptor.requests[0], QUrl(QStringLiteral("file:///redirect/local-cors/resources/redirect.html")));
    QCOMPARE(interceptor.requests[1], QUrl(QStringLiteral("local-cors:/resources/redirect.html")));
    QCOMPARE(interceptor.requests[2], QUrl(QStringLiteral("file:///redirect/local-cors/resources/redirect.css")));
    QCOMPARE(interceptor.requests[3], QUrl(QStringLiteral("local-cors:/resources/redirect.css")));
}

void tst_Origins::redirectInterceptorHttp()
{
    TestRedirectInterceptor interceptor;
    m_profile.setUrlRequestInterceptor(&interceptor);

    QVERIFY(verifyLoad(QSL("http://hallo/redirect/cors/resources/redirect.html")));
    eval("addStylesheetLink('http://hallo/redirect/cors/resources/redirect.css')");

    QTRY_COMPARE(interceptor.requests.size(), 4);
    QTRY_COMPARE(m_handler->requests().size(), 2);
    QCOMPARE(m_handler->requests()[0], QUrl(QStringLiteral("cors:/resources/redirect.html")));
    QCOMPARE(m_handler->requests()[1], QUrl(QStringLiteral("cors:/resources/redirect.css")));

    QCOMPARE(interceptor.requests[0], QUrl(QStringLiteral("http://hallo/redirect/cors/resources/redirect.html")));
    QCOMPARE(interceptor.requests[1], QUrl(QStringLiteral("cors:/resources/redirect.html")));
    QCOMPARE(interceptor.requests[2], QUrl(QStringLiteral("http://hallo/redirect/cors/resources/redirect.css")));
    QCOMPARE(interceptor.requests[3], QUrl(QStringLiteral("cors:/resources/redirect.css")));
}

void tst_Origins::localMediaBlock_data()
{
    QTest::addColumn<bool>("enableAccess");
    QTest::addRow("enabled") << true;
    QTest::addRow("disabled") << false;
}

void tst_Origins::localMediaBlock()
{
    QFETCH(bool, enableAccess);

    std::atomic<bool> accessed = false;
    HttpServer server;
    server.setResourceDirs({ QDir(QT_TESTCASE_SOURCEDIR).canonicalPath() + "/resources" });
    connect(&server, &HttpServer::newRequest, [&](HttpReqRep *) { accessed.store(true); });
    QVERIFY(server.start());

    ScopedAttribute sa1(m_page->settings(), QWebEngineSettings::LocalContentCanAccessRemoteUrls, enableAccess);

    QVERIFY(verifyLoad("file:" + QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                       + "/resources/media.html"));
    eval("addAudio('" + server.url("/mixedXHR.txt").toString() + "')");

    // Give it a chance to avoid a false positive on the default value of accessed.
    if (!enableAccess)
        QTest::qSleep(500);
    QTRY_COMPARE(accessed.load(), enableAccess);

}

class FetchApiHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    FetchApiHandler(QByteArray schemeName, QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent), m_schemeName(schemeName)
    {
    }

    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QCOMPARE(job->requestUrl(), QUrl(m_schemeName + ":about"));
        fetchWasAllowed = true;
    }

    bool fetchWasAllowed = false;

private:
    QByteArray m_schemeName;
};

class FetchApiPage : public QWebEnginePage
{
    Q_OBJECT

signals:
    void jsCalled();

public:
    FetchApiPage(QWebEngineProfile *profile, QObject *parent = nullptr)
        : QWebEnginePage(profile, parent)
    {
    }

protected:
    void javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level,
                                  const QString &message, int lineNumber,
                                  const QString &sourceID) override
    {
        qCritical() << "js:" << message;
        emit jsCalled();
    }
};

void tst_Origins::fetchApiCustomUrl_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QByteArray>("fetchApiScheme");
    QTest::addColumn<bool>("expectedFetchWasAllowed");

    QTest::newRow("custom url with fetch allowed flag")
            << QUrl("qrc:///resources/fetchApi.html?printRes=false&url=fetchapi-allowed:about")
            << QBAL("fetchapi-allowed") << true;
    QTest::newRow("custom url without fetch allowed flag")
            << QUrl("qrc:///resources/fetchApi.html?printRes=false&url=fetchapi-not-allowed:about")
            << QBAL("fetchapi-not-allowed") << false;
}

void tst_Origins::fetchApiCustomUrl()
{
    QFETCH(QUrl, url);
    QFETCH(QByteArray, fetchApiScheme);
    QFETCH(bool, expectedFetchWasAllowed);

    QWebEngineProfile profile;
    FetchApiHandler handler(fetchApiScheme);

    profile.installUrlSchemeHandler(fetchApiScheme, &handler);

    FetchApiPage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy jsSpy(&page, SIGNAL(jsCalled()));

    if (fetchApiScheme == "fetchapi-not-allowed") {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression("Failed to fetch"));
        QTest::ignoreMessage(
                QtCriticalMsg,
                QRegularExpression("Fetch API cannot load fetchapi-not-allowed:about."));
    }

    page.load(url);
    QTRY_VERIFY(loadSpy.count() > 0);
    QTRY_COMPARE(handler.fetchWasAllowed, expectedFetchWasAllowed);

    if (fetchApiScheme == "fetchapi-not-allowed") {
        QTRY_VERIFY(jsSpy.count() > 0);
    }
}

void tst_Origins::fetchApiHttpUrl()
{
    HttpServer httpServer;
    QObject::connect(&httpServer, &HttpServer::newRequest, this, [](HttpReqRep *rr) {
        rr->setResponseBody(QBAL("Fetch Was Allowed"));
        rr->setResponseHeader(QBAL("Access-Control-Allow-Origin"), QBAL("*"));
        rr->sendResponse();
    });
    QVERIFY(httpServer.start());

    QWebEngineProfile profile;
    FetchApiPage page(&profile);

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy jsSpy(&page, SIGNAL(jsCalled()));

    QTest::ignoreMessage(QtCriticalMsg, QRegularExpression("Fetch Was Allowed"));

    const QByteArray fullUrl = QByteArray("qrc:///resources/fetchApi.html?printRes=true&url=")
            + httpServer.url("/somepage.html").toEncoded();
    page.load(QUrl(fullUrl));

    QTRY_VERIFY(loadSpy.count() > 0);
    QTRY_VERIFY(jsSpy.count() > 0);
    QVERIFY(httpServer.stop());
}

QTEST_MAIN(tst_Origins)
#include "tst_origins.moc"
