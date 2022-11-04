// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest/QtTest>

#include <qwebengineview.h>
#include <qwebenginepage.h>
#include <qwebengineprofile.h>
#include <qwebenginesettings.h>

class tst_Schemes : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void unknownUrlSchemePolicy_data();
    void unknownUrlSchemePolicy();
};

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

QTEST_MAIN(tst_Schemes)
#include "tst_schemes.moc"
