// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest/QtTest>

#include <qwebenginepage.h>
#include <qwebengineprofile.h>
#include <qwebenginesettings.h>
#include <qwebengineurlrequestjob.h>
#include <qwebengineurlscheme.h>
#include <qwebengineurlschemehandler.h>
#include <qwebengineview.h>
#include <widgetutil.h>

class tst_Schemes : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void unknownUrlSchemePolicy_data();
    void unknownUrlSchemePolicy();
    void customSchemeFragmentNavigation_data();
    void customSchemeFragmentNavigation();
};

void tst_Schemes::initTestCase()
{
    QWebEngineUrlScheme pathScheme("path");
    pathScheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    QWebEngineUrlScheme::registerScheme(pathScheme);

    QWebEngineUrlScheme hostScheme("host");
    hostScheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);
    QWebEngineUrlScheme::registerScheme(hostScheme);

    QWebEngineUrlScheme hostAndPortScheme("hostandport");
    hostAndPortScheme.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort);
    hostAndPortScheme.setDefaultPort(3000);
    QWebEngineUrlScheme::registerScheme(hostAndPortScheme);

    QWebEngineUrlScheme hostPortUserInfoScheme("hostportuserinfo");
    hostPortUserInfoScheme.setSyntax(QWebEngineUrlScheme::Syntax::HostPortAndUserInformation);
    hostPortUserInfoScheme.setDefaultPort(3000);
    QWebEngineUrlScheme::registerScheme(hostPortUserInfoScheme);
}

class AcceptNavigationRequestHandler : public QWebEnginePage
{
public:
    AcceptNavigationRequestHandler(QObject* parent = nullptr)
        : QWebEnginePage(parent)
    {
    }
    int acceptNavigationRequestCalls = 0;
    bool acceptNavigationRequest(const QUrl &/*url*/, NavigationType type, bool /*isMainFrame*/) override
    {
        if (type == QWebEnginePage::NavigationTypeTyped)
            return true;
        this->acceptNavigationRequestCalls++;
        return false;
    }
};

Q_DECLARE_METATYPE(QWebEngineSettings::UnknownUrlSchemePolicy)

void tst_Schemes::unknownUrlSchemePolicy_data()
{
    QTest::addColumn<QWebEngineSettings::UnknownUrlSchemePolicy>("policy");
    QTest::addColumn<bool>("userAction");
    QTest::newRow("DisallowUnknownUrlSchemes, script") << QWebEngineSettings::DisallowUnknownUrlSchemes << false;
    QTest::newRow("DisallowUnknownUrlSchemes, user")   << QWebEngineSettings::DisallowUnknownUrlSchemes << true;
    QTest::newRow("AllowUnknownUrlSchemesFromUserInteraction, script") << QWebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction << false;
    QTest::newRow("AllowUnknownUrlSchemesFromUserInteraction, user")   << QWebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction << true;
    QTest::newRow("AllowAllUnknownUrlSchemes, script") << QWebEngineSettings::AllowAllUnknownUrlSchemes << false;
    QTest::newRow("AllowAllUnknownUrlSchemes, user")   << QWebEngineSettings::AllowAllUnknownUrlSchemes << true;
    QTest::newRow("default UnknownUrlSchemePolicy, script") << QWebEngineSettings::UnknownUrlSchemePolicy(0) << false;
    QTest::newRow("default UnknownUrlSchemePolicy, user")   << QWebEngineSettings::UnknownUrlSchemePolicy(0) << true;
}

void tst_Schemes::unknownUrlSchemePolicy()
{
    QFETCH(QWebEngineSettings::UnknownUrlSchemePolicy, policy);
    QFETCH(bool, userAction);

    QWebEngineView view;
    AcceptNavigationRequestHandler page;
    QSignalSpy loadFinishedSpy(&page, &QWebEnginePage::loadFinished);
    view.setPage(&page);
    view.resize(400, 40);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QWebEngineSettings *settings = view.page()->profile()->settings();
    settings->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);
    settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);

    if (policy > 0)
        settings->setUnknownUrlSchemePolicy(policy);
    else
        settings->resetUnknownUrlSchemePolicy();
    loadFinishedSpy.clear();
    page.acceptNavigationRequestCalls = 0;
    bool shouldAccept;

    if (!userAction) { // navigation request coming from javascript
        shouldAccept = (policy == QWebEngineSettings::AllowAllUnknownUrlSchemes);
        view.setHtml("<html><script>setTimeout(function(){ window.location.href='nonexistentscheme://somewhere'; }, 10);</script><body>testing...</body></html>");
    } else { // navigation request coming from user interaction
        shouldAccept = (policy != QWebEngineSettings::DisallowUnknownUrlSchemes);
        view.setHtml("<html><body><a id='nonexlink' href='nonexistentscheme://somewhere'>nonexistentscheme://somewhere</a></body></html>");
        QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 15000);
        // focus and trigger the link
        view.page()->runJavaScript("document.getElementById('nonexlink').focus();", [&view](const QVariant &result) {
            Q_UNUSED(result);
            QTest::sendKeyEvent(QTest::Press, view.focusProxy(), Qt::Key_Return, QString("\r"), Qt::NoModifier);
            QTest::sendKeyEvent(QTest::Release, view.focusProxy(), Qt::Key_Return, QString("\r"), Qt::NoModifier);
        });
    }

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 2, 60000);
    QCOMPARE(page.acceptNavigationRequestCalls, shouldAccept ? 1 : 0);
}

class CustomScheme : public QWebEngineUrlSchemeHandler
{
public:
    CustomScheme(const QString &linkUrl) : m_linkUrl(linkUrl) { }

    void requestStarted(QWebEngineUrlRequestJob *requestJob) override
    {
        QString html = QString("<html><body>"
                               "<p style='height: 2000px;'>"
                               "<a href='%1' id='link'>Click link</a>"
                               "</p><p id='anchor'>Anchor</p>"
                               "</body></html>")
                               .arg(m_linkUrl);
        QBuffer *buffer = new QBuffer(requestJob);
        buffer->setData(html.toUtf8());
        requestJob->reply("text/html", buffer);
    }

    QString m_linkUrl;
};

void tst_Schemes::customSchemeFragmentNavigation_data()
{
    QTest::addColumn<QUrl>("baseUrl");
    QTest::addColumn<QString>("linkUrl");
    QTest::addColumn<QUrl>("expectedUrl");

    // Path syntax
    // - Preserves each part of the URL after navigation
    QTest::newRow("Path syntax, path only, relative url")
            << QUrl("path://path") << "#anchor" << QUrl("path://path#anchor");
    QTest::newRow("Path syntax, path only, absolute url")
            << QUrl("path://path") << "path://path#anchor" << QUrl("path://path#anchor");
    QTest::newRow("Path syntax, host/path, relative url")
            << QUrl("path://host/path") << "#anchor" << QUrl("path://host/path#anchor");
    QTest::newRow("Path syntax, host/path, absolute url")
            << QUrl("path://host/path") << "path://host/path#anchor"
            << QUrl("path://host/path#anchor");
    QTest::newRow("Path syntax, host:port, relative url")
            << QUrl("path://host:3000") << "#anchor" << QUrl("path://host:3000#anchor");
    QTest::newRow("Path syntax, host:port, absolute url")
            << QUrl("path://host:3000") << "path://host:3000#anchor"
            << QUrl("path://host:3000#anchor");
    QTest::newRow("Path syntax, userinfo@host:port, relative url")
            << QUrl("path://user:password@host:3000") << "#anchor"
            << QUrl("path://user:password@host:3000#anchor");
    QTest::newRow("Path syntax, userinfo@host:port, absolute url")
            << QUrl("path://user:password@host:3000") << "path://user:password@host:3000#anchor"
            << QUrl("path://user:password@host:3000#anchor");

    // Host syntax
    // - We lose the port and the user info from the authority after navigation
    QTest::newRow("Host syntax, host only, relative url")
            << QUrl("host://host") << "#anchor" << QUrl("host://host/#anchor");
    QTest::newRow("Host syntax, host only, absolute url")
            << QUrl("host://host") << "host://host#anchor" << QUrl("host://host/#anchor");
    QTest::newRow("Host syntax, host/path, relative url")
            << QUrl("host://host/path") << "#anchor" << QUrl("host://host/path#anchor");
    QTest::newRow("Host syntax, host/path, absolute url")
            << QUrl("host://host/path") << "host://host/path#anchor"
            << QUrl("host://host/path#anchor");
    QTest::newRow("Host syntax, host:port, relative url")
            << QUrl("host://host:3000") << "#anchor" << QUrl("host://host/#anchor");
    QTest::newRow("Host syntax, host:port, absolute url")
            << QUrl("host://host:3000") << "host://host:3000#anchor" << QUrl("host://host/#anchor");
    QTest::newRow("Host syntax, userinfo@host:port, relative url")
            << QUrl("host://user:password@host:3000") << "#anchor" << QUrl("host://host/#anchor");
    QTest::newRow("Host syntax, userinfo@host:port, absolute url")
            << QUrl("host://user:password@host:3000") << "host://user:password@host:3000#anchor"
            << QUrl("host://host/#anchor");

    // HostAndPort syntax
    // - We lose the port and the user info from the authority after navigation
    QTest::newRow("HostAndPort syntax, host only, relative url")
            << QUrl("hostandport://host") << "#anchor" << QUrl("hostandport://host/#anchor");
    QTest::newRow("HostAndPort syntax, host only, absolute url")
            << QUrl("hostandport://host") << "hostandport://host#anchor"
            << QUrl("hostandport://host/#anchor");
    QTest::newRow("HostAndPort syntax, host/path, relative url")
            << QUrl("hostandport://host/path") << "#anchor"
            << QUrl("hostandport://host/path#anchor");
    QTest::newRow("HostAndPort syntax, host/path, absolute url")
            << QUrl("hostandport://host/path") << "hostandport://host/path#anchor"
            << QUrl("hostandport://host/path#anchor");
    QTest::newRow("HostAndPort syntax, host:port, relative url")
            << QUrl("hostandport://host:3000") << "#anchor" << QUrl("hostandport://host/#anchor");
    QTest::newRow("HostAndPort syntax, host:port, absolute url")
            << QUrl("hostandport://host:3000") << "hostandport://host:3000#anchor"
            << QUrl("hostandport://host/#anchor");
    QTest::newRow("HostAndPort syntax, userinfo@host:port, relative url")
            << QUrl("hostandport://user:password@host:3000") << "#anchor"
            << QUrl("hostandport://host/#anchor");
    QTest::newRow("HostAndPort syntax, userinfo@host:port, absolute url")
            << QUrl("hostandport://user:password@host:3000")
            << "hostandport://user:password@host:3000#anchor" << QUrl("hostandport://host/#anchor");

    // HostPortAndUserInformation syntax
    // - We lose the port and it preserves the user info in the authority after navigation
    QTest::newRow("HostPortAndUserInformation syntax, host only, relative url")
            << QUrl("hostportuserinfo://host") << "#anchor"
            << QUrl("hostportuserinfo://host/#anchor");
    QTest::newRow("HostPortAndUserInformation syntax, host only, absolute url")
            << QUrl("hostportuserinfo://host") << "hostportuserinfo://host#anchor"
            << QUrl("hostportuserinfo://host/#anchor");
    QTest::newRow("HostPortAndUserInformation syntax, host/path, relative url")
            << QUrl("hostportuserinfo://host/path") << "#anchor"
            << QUrl("hostportuserinfo://host/path#anchor");
    QTest::newRow("HostPortAndUserInformation syntax, host/path, absolute url")
            << QUrl("hostportuserinfo://host/path") << "hostportuserinfo://host/path#anchor"
            << QUrl("hostportuserinfo://host/path#anchor");
    QTest::newRow("HostPortAndUserInformation syntax, host:port, relative url")
            << QUrl("hostportuserinfo://host:3000") << "#anchor"
            << QUrl("hostportuserinfo://host/#anchor");
    QTest::newRow("HostPortAndUserInformation syntax, host:port, absolute url")
            << QUrl("hostportuserinfo://host:3000") << "hostportuserinfo://host:3000#anchor"
            << QUrl("hostportuserinfo://host/#anchor");
    QTest::newRow("HostPortAndUserInformation syntax, userinfo@host:port, relative url")
            << QUrl("hostportuserinfo://user:password@host:3000") << "#anchor"
            << QUrl("hostportuserinfo://user:password@host/#anchor");
    QTest::newRow("HostPortAndUserInformation syntax, userinfo@host:port, absolute url")
            << QUrl("hostportuserinfo://user:password@host:3000")
            << "hostportuserinfo://user:password@host:3000#anchor"
            << QUrl("hostportuserinfo://user:password@host/#anchor");
}

void tst_Schemes::customSchemeFragmentNavigation()
{
    QFETCH(QUrl, baseUrl);
    QFETCH(QUrl, expectedUrl);
    QFETCH(QString, linkUrl);

    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QWebEngineView view;
    view.setPage(&page);
    view.resize(800, 600);
    view.show();
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy urlChangedSpy(&page, SIGNAL(urlChanged(QUrl)));

    CustomScheme *schemeHandler = new CustomScheme(linkUrl);
    page.profile()->installUrlSchemeHandler(baseUrl.scheme().toUtf8(), schemeHandler);

    view.load(baseUrl);
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "window.scrollY").toInt() == 0);

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, elementCenter(&page, "link"));
    QVERIFY(urlChangedSpy.wait());
    QCOMPARE(page.url(), expectedUrl);
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "window.scrollY").toInt() > 0);

    // Same document navigation doesn't emit loadFinished
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
}

QTEST_MAIN(tst_Schemes)
#include "tst_schemes.moc"
