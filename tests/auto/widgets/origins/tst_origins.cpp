/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "../util.h"

#include <QtCore/qfile.h>
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineurlrequestjob.h>
#include <QtWebEngineCore/qwebengineurlschemehandler.h>
#include <QtWebEngineWidgets/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebengineprofile.h>
#include <QtWebEngineWidgets/qwebenginesettings.h>

#define QSL QStringLiteral
#define QBAL QByteArrayLiteral
#define THIS_DIR TESTS_SOURCE_DIR "origins/"

class TstUrlSchemeHandler final : public QWebEngineUrlSchemeHandler {
    Q_OBJECT

public:
    TstUrlSchemeHandler(QWebEngineProfile *profile)
    {
        profile->installUrlSchemeHandler(QBAL("tst"), this);
    }

private:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QString pathPrefix = QSL(THIS_DIR);
        QString pathSuffix = job->requestUrl().path();
        QFile *file = new QFile(pathPrefix + pathSuffix, job);
        if (!file->open(QIODevice::ReadOnly)) {
            job->fail(QWebEngineUrlRequestJob::RequestFailed);
            return;
        }
        job->reply(QBAL("text/html"), file);
    }
};

class tst_Origins final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void jsUrlCanon();
    void jsUrlOrigin();
    void subdirWithAccess();
    void subdirWithoutAccess();
    void mixedSchemes();
    void webSocket();
    void dedicatedWorker();
    void sharedWorker();
    void serviceWorker();

private:
    bool load(const QUrl &url)
    {
        QSignalSpy spy(&m_page, &QWebEnginePage::loadFinished);
        m_page.load(url);
        return (!spy.empty() || spy.wait())
            && spy.front().value(0).toBool();
    }

    QVariant eval(const QString &code)
    {
        return evaluateJavaScriptSync(&m_page, code);
    }

    QWebEngineProfile m_profile;
    QWebEnginePage m_page{&m_profile};
    TstUrlSchemeHandler m_handler{&m_profile};
};

// Test URL parsing and canonicalization in Blink. The implementation of this
// part is mostly shared between Blink and Chromium proper.
void tst_Origins::jsUrlCanon()
{
    QVERIFY(load(QSL("about:blank")));

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

    // The qrc scheme is a 'dumb' URL, having only a path and nothing else.
    QCOMPARE(eval(QSL("new URL(\"qrc:foo/bar\").href")),    QVariant(QSL("qrc:foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"qrc:/foo/bar\").href")),   QVariant(QSL("qrc:/foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"qrc://foo/bar\").href")),  QVariant(QSL("qrc://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"qrc:///foo/bar\").href")), QVariant(QSL("qrc:///foo/bar")));

    // Same for custom schemes.
    QCOMPARE(eval(QSL("new URL(\"tst:foo/bar\").href")),    QVariant(QSL("tst:foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"tst:/foo/bar\").href")),   QVariant(QSL("tst:/foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"tst://foo/bar\").href")),  QVariant(QSL("tst://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"tst:///foo/bar\").href")), QVariant(QSL("tst:///foo/bar")));
}

// Test origin serialization in Blink, implemented by blink::KURL and
// blink::SecurityOrigin as opposed to GURL and url::Origin.
void tst_Origins::jsUrlOrigin()
{
    QVERIFY(load(QSL("about:blank")));

    // For network protocols the origin string must include the domain and port.
    QCOMPARE(eval(QSL("new URL(\"http://foo.com/page.html\").origin")), QVariant(QSL("http://foo.com")));
    QCOMPARE(eval(QSL("new URL(\"https://foo.com/page.html\").origin")), QVariant(QSL("https://foo.com")));

    // Even though file URL can also have domains, these are not included in the
    // origin string by Chromium. The standard does not specify a value here,
    // but suggests 'null' (https://url.spec.whatwg.org/#origin).
    QCOMPARE(eval(QSL("new URL(\"file:/etc/passwd\").origin")), QVariant(QSL("file://")));
    QCOMPARE(eval(QSL("new URL(\"file://foo.com/etc/passwd\").origin")), QVariant(QSL("file://")));

    // The qrc scheme should behave like file.
    QCOMPARE(eval(QSL("new URL(\"qrc:/crysis.css\").origin")), QVariant(QSL("qrc://")));
    QCOMPARE(eval(QSL("new URL(\"qrc://foo.com/crysis.css\").origin")), QVariant(QSL("qrc://")));

    // Same with custom schemes.
    QCOMPARE(eval(QSL("new URL(\"tst:/banana\").origin")), QVariant(QSL("tst://")));
    QCOMPARE(eval(QSL("new URL(\"tst://foo.com/banana\").origin")), QVariant(QSL("tst://")));
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
    ScopedAttribute sa(m_page.settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    QVERIFY(load(QSL("file:" THIS_DIR "resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));

    QVERIFY(load(QSL("qrc:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));

    QVERIFY(load(QSL("tst:/resources/subdir/index.html")));
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
    ScopedAttribute sa(m_page.settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, false);

    QVERIFY(load(QSL("file:" THIS_DIR "resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant());
    QCOMPARE(eval(QSL("msg[1]")), QVariant());

    QVERIFY(load(QSL("qrc:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));

    QVERIFY(load(QSL("tst:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));
}

// Try to mix schemes, for example by loading the main page over file with an
// iframe over qrc. This should be forbidden.
void tst_Origins::mixedSchemes()
{
    QVERIFY(load(QSL("file:" THIS_DIR "resources/mixed_qrc.html")));
    QCOMPARE(eval(QSL("msg")), QVariant());
    QVERIFY(load(QSL("file:" THIS_DIR "resources/mixed_tst.html")));
    QCOMPARE(eval(QSL("msg")), QVariant());

    QVERIFY(load(QSL("qrc:/resources/mixed_qrc.html")));
    QCOMPARE(eval(QSL("msg")), QVariant(QSL("mixed")));
    QVERIFY(load(QSL("qrc:/resources/mixed_tst.html")));
    QCOMPARE(eval(QSL("msg")), QVariant());

    QVERIFY(load(QSL("tst:/resources/mixed_qrc.html")));
    QCOMPARE(eval(QSL("msg")), QVariant());
    QVERIFY(load(QSL("tst:/resources/mixed_tst.html")));
    QCOMPARE(eval(QSL("msg")), QVariant(QSL("mixed")));
}

// Try opening a WebSocket from pages loaded over various URL schemes.
void tst_Origins::webSocket()
{
    // 1006 indicates 'Abnormal Closure'.
    //
    // The example page is passing a URL with a non-existent domain to the
    // WebSocket constructor, so we expect the connection to fail. This is
    // enough though to trigger the origin checks.
    const int expected = 1006;

    QVERIFY(load(QSL("file:" THIS_DIR "resources/websocket.html")));
    QTRY_VERIFY(eval(QSL("err")) == QVariant(expected));

    QVERIFY(load(QSL("qrc:/resources/websocket.html")));
    QTRY_VERIFY(eval(QSL("err")) == QVariant(expected));

    QVERIFY(load(QSL("tst:/resources/websocket.html")));
    QTRY_VERIFY(eval(QSL("err")) == QVariant(expected));
}

// Create a (Dedicated)Worker. Since dedicated workers can only be accessed from
// one page, there is not much need for security restrictions.
void tst_Origins::dedicatedWorker()
{
    QVERIFY(load(QSL("file:" THIS_DIR "resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    QVERIFY(load(QSL("qrc:/resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    // FIXME(juvaldma): QTBUG-62536
    QVERIFY(load(QSL("tst:/resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("Access to dedicated workers is denied to origin 'tst://'")));
}

// Create a SharedWorker. Shared workers can be accessed from multiple pages,
// and therefore the same-origin policy applies.
void tst_Origins::sharedWorker()
{
    {
        ScopedAttribute sa(m_page.settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, false);
        QVERIFY(load(QSL("file:" THIS_DIR "resources/sharedWorker.html")));
        QTRY_VERIFY(eval(QSL("done")).toBool());
        QVERIFY(eval(QSL("error")).toString()
                .contains(QSL("cannot be accessed from origin 'null'")));
    }

    {
        ScopedAttribute sa(m_page.settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, true);
        QVERIFY(load(QSL("file:" THIS_DIR "resources/sharedWorker.html")));
        QTRY_VERIFY(eval(QSL("done")).toBool());
        QCOMPARE(eval(QSL("result")), QVariant(42));
    }

    QVERIFY(load(QSL("qrc:/resources/sharedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    QVERIFY(load(QSL("tst:/resources/sharedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));
}

// Service workers don't work.
void tst_Origins::serviceWorker()
{
    QVERIFY(load(QSL("file:" THIS_DIR "resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("The URL protocol of the current origin ('file://') is not supported.")));

    QVERIFY(load(QSL("qrc:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("The URL protocol of the current origin ('qrc://') is not supported.")));

    QVERIFY(load(QSL("tst:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("Only secure origins are allowed")));
}

QTEST_MAIN(tst_Origins)
#include "tst_origins.moc"
