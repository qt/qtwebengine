/*
    Copyright (C) 2015 The Qt Company Ltd.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtTest/QtTest>

#include <qwebengineview.h>
#include <qwebenginepage.h>
#include <qwebengineprofile.h>
#include <qwebenginesettings.h>

class tst_QWebEngineSettings: public QObject {
    Q_OBJECT

private Q_SLOTS:
    void resetAttributes();
    void defaultFontFamily_data();
    void defaultFontFamily();
    void unknownUrlSchemePolicy();
};

void tst_QWebEngineSettings::resetAttributes()
{
    // QT_TODO_FIXME_ADAPT
    QSKIP("The application deadlocks and hangs without exiting.");
    QWebEngineProfile profile;
    QWebEngineSettings *settings = profile.settings();

    // Attribute
    bool defaultValue = settings->testAttribute(QWebEngineSettings::FullScreenSupportEnabled);
    settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, !defaultValue);
    QCOMPARE(!defaultValue, settings->testAttribute(QWebEngineSettings::FullScreenSupportEnabled));
    settings->resetAttribute(QWebEngineSettings::FullScreenSupportEnabled);
    QCOMPARE(defaultValue, settings->testAttribute(QWebEngineSettings::FullScreenSupportEnabled));

    // Font family
    QString defaultFamily = settings->fontFamily(QWebEngineSettings::StandardFont);
    QString newFontFamily("PugDog");
    settings->setFontFamily(QWebEngineSettings::StandardFont, newFontFamily);
    QCOMPARE(newFontFamily, settings->fontFamily(QWebEngineSettings::StandardFont));
    settings->resetFontFamily(QWebEngineSettings::StandardFont);
    QCOMPARE(defaultFamily, settings->fontFamily(QWebEngineSettings::StandardFont));

    // Font size
    int defaultSize = settings->fontSize(QWebEngineSettings::MinimumFontSize);
    int newSize = defaultSize + 10;
    settings->setFontSize(QWebEngineSettings::MinimumFontSize, newSize);
    QCOMPARE(newSize, settings->fontSize(QWebEngineSettings::MinimumFontSize));
    settings->resetFontSize(QWebEngineSettings::MinimumFontSize);
    QCOMPARE(defaultSize, settings->fontSize(QWebEngineSettings::MinimumFontSize));
}

void tst_QWebEngineSettings::defaultFontFamily_data()
{
    QTest::addColumn<int>("fontFamily");

    QTest::newRow("StandardFont") << static_cast<int>(QWebEngineSettings::StandardFont);
    QTest::newRow("FixedFont") << static_cast<int>(QWebEngineSettings::FixedFont);
    QTest::newRow("SerifFont") << static_cast<int>(QWebEngineSettings::SerifFont);
    QTest::newRow("SansSerifFont") << static_cast<int>(QWebEngineSettings::SansSerifFont);
    QTest::newRow("CursiveFont") << static_cast<int>(QWebEngineSettings::CursiveFont);
    QTest::newRow("FantasyFont") << static_cast<int>(QWebEngineSettings::FantasyFont);
}

void tst_QWebEngineSettings::defaultFontFamily()
{
    // QT_TODO_FIXME_ADAPT
    QSKIP("The application deadlocks and hangs without exiting.");
    QWebEngineProfile profile;
    QWebEngineSettings *settings = profile.settings();

    QFETCH(int, fontFamily);
    QVERIFY(!settings->fontFamily(static_cast<QWebEngineSettings::FontFamily>(fontFamily)).isEmpty());
}


class AcceptNavigationRequestHandler : public QWebEnginePage
{
public:
    AcceptNavigationRequestHandler(QObject* parent = nullptr)
        : QWebEnginePage(parent)
    {
    }
    int acceptNavigationRequestCalls = 0;
    bool acceptNavigationRequest(const QUrl &/*url*/, NavigationType /*type*/, bool /*isMainFrame*/) override
    {
        this->acceptNavigationRequestCalls++;
        return false;
    }
};

void tst_QWebEngineSettings::unknownUrlSchemePolicy()
{
    QWebEngineView view;
    AcceptNavigationRequestHandler page;
    view.setPage(&page);
    view.resize(400, 40);
    view.show();
    QTest::qWaitForWindowExposed(&view);
    QWebEngineSettings *settings = view.page()->profile()->settings();
    settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    QSignalSpy loadFinishedSpy(&view, &QWebEngineView::loadFinished);

    QWebEngineSettings::UnknownUrlSchemePolicy policies[6] = {QWebEngineSettings::DisallowUnknownUrlSchemes,
                                                              QWebEngineSettings::DisallowUnknownUrlSchemes,
                                                              QWebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction,
                                                              QWebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction,
                                                              QWebEngineSettings::AllowAllUnknownUrlSchemes,
                                                              QWebEngineSettings::AllowAllUnknownUrlSchemes};
    // even iterations are for navigation-requests from javascript,
    // odd iterations are for navigations-requests from user-interaction
    for (int i = 0; i < 8; i++) {
        if (i <= 5)
            settings->setUnknownUrlSchemePolicy(policies[i]);
        else
            settings->resetUnknownUrlSchemePolicy();
        loadFinishedSpy.clear();
        page.acceptNavigationRequestCalls = 0;
        bool shouldAccept;

        if (i % 2 == 0) { // navigation request coming from javascript
            shouldAccept = (4 <= i && i <= 5); // only case AllowAllUnknownUrlSchemes
            view.setHtml("<html><script>setTimeout(function(){ window.location.href='nonexistentscheme://somewhere'; }, 10);</script><body>testing...</body></html>");
        } else { // navigation request coming from user interaction
            shouldAccept = (2 <= i); // all cases except DisallowUnknownUrlSchemes
            view.setHtml("<html><body><a id='nonexlink' href='nonexistentscheme://somewhere'>nonexistentscheme://somewhere</a></body></html>");
            QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
            // focus and trigger the link
            view.page()->runJavaScript("document.getElementById('nonexlink').focus();", [&view](const QVariant &result) {
                Q_UNUSED(result);
                QTest::sendKeyEvent(QTest::Press, view.focusProxy(), Qt::Key_Return, QString("\r"), Qt::NoModifier);
                QTest::sendKeyEvent(QTest::Release, view.focusProxy(), Qt::Key_Return, QString("\r"), Qt::NoModifier);
            });
        }

        bool errorPageEnabled = settings->testAttribute(QWebEngineSettings::ErrorPageEnabled);
        QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 2 + (errorPageEnabled ? 1 : 0), 30000);
        QCOMPARE(page.acceptNavigationRequestCalls, shouldAccept ? 1 : 0);
    }
}

QTEST_MAIN(tst_QWebEngineSettings)

#include "tst_qwebenginesettings.moc"
