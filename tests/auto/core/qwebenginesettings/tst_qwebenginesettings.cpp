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

#include <util.h>

#include <QtTest/QtTest>

#include <qwebenginepage.h>
#include <qwebengineprofile.h>
#include <qwebenginesettings.h>

#include <QtGui/qclipboard.h>
#include <QtGui/qguiapplication.h>
#include <QtWebEngineWidgets/qwebengineview.h>

class tst_QWebEngineSettings: public QObject {
    Q_OBJECT

private Q_SLOTS:
    void resetAttributes();
    void defaultFontFamily_data();
    void defaultFontFamily();
    void javascriptClipboard_data();
    void javascriptClipboard();
    void setInAcceptNavigationRequest();
    void disableReadingFromCanvas_data();
    void disableReadingFromCanvas();
    void forceDarkMode();
    void forceDarkModeMultiView();
};

void tst_QWebEngineSettings::resetAttributes()
{
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
    QWebEngineProfile profile;
    QWebEngineSettings *settings = profile.settings();

    QFETCH(int, fontFamily);
    QVERIFY(!settings->fontFamily(static_cast<QWebEngineSettings::FontFamily>(fontFamily)).isEmpty());
}

void tst_QWebEngineSettings::javascriptClipboard_data()
{
    QTest::addColumn<bool>("javascriptCanAccessClipboard");
    QTest::addColumn<bool>("javascriptCanPaste");
    QTest::addColumn<bool>("copyResult");
    QTest::addColumn<bool>("pasteResult");

    QTest::newRow("default") << false << false << false << false;
    QTest::newRow("canCopy") << true << false << true << false;
    // paste command requires both permissions
    QTest::newRow("canPaste") << false << true << false << false;
    QTest::newRow("canCopyAndPaste") << true << true << true << true;
}

void tst_QWebEngineSettings::javascriptClipboard()
{
    QFETCH(bool, javascriptCanAccessClipboard);
    QFETCH(bool, javascriptCanPaste);
    QFETCH(bool, copyResult);
    QFETCH(bool, pasteResult);

    QWebEnginePage page;

    // check defaults
    QCOMPARE(page.settings()->testAttribute(QWebEngineSettings::JavascriptCanAccessClipboard),
             false);
    QCOMPARE(page.settings()->testAttribute(QWebEngineSettings::JavascriptCanPaste), false);

    // check accessors
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard,
                                  javascriptCanAccessClipboard);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanPaste,
                                  javascriptCanPaste);
    QCOMPARE(page.settings()->testAttribute(QWebEngineSettings::JavascriptCanAccessClipboard),
             javascriptCanAccessClipboard);
    QCOMPARE(page.settings()->testAttribute(QWebEngineSettings::JavascriptCanPaste),
             javascriptCanPaste);

    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml("<html><body>"
                 "<input type='text' value='OriginalText' id='myInput'/>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());

    // make sure that 'OriginalText' is selected
    evaluateJavaScriptSync(&page, "document.getElementById('myInput').select()");
    QCOMPARE(evaluateJavaScriptSync(&page, "window.getSelection().toString()").toString(),
             QStringLiteral("OriginalText"));

    // Check that the actual settings work by the
    // - return value of queryCommandEnabled and
    // - return value of execCommand
    // - comparing the clipboard / input field
    QGuiApplication::clipboard()->setText(QString());
    QCOMPARE(evaluateJavaScriptSync(&page, "document.queryCommandEnabled('copy')").toBool(),
             copyResult);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.execCommand('copy')").toBool(), copyResult);
    QTRY_COMPARE(QGuiApplication::clipboard()->text(),
                 (copyResult ? QString("OriginalText") : QString()));


    QGuiApplication::clipboard()->setText("AnotherText");
    QCOMPARE(evaluateJavaScriptSync(&page, "document.queryCommandEnabled('paste')").toBool(),
             pasteResult);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.execCommand('paste')").toBool(), pasteResult);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.getElementById('myInput').value").toString(),
                           (pasteResult ? QString("AnotherText") : QString("OriginalText")));
}

class NavigationRequestOverride : public QWebEnginePage
{
protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override
    {
        Q_UNUSED(type);

        if (isMainFrame && url.scheme().startsWith("data"))
            settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
            // TODO: note this setting is flaky, consider settings().commit()
        return true;
    }
};

void tst_QWebEngineSettings::setInAcceptNavigationRequest()
{
    NavigationRequestOverride page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    QVERIFY(!page.settings()->testAttribute(QWebEngineSettings::JavascriptEnabled));

    page.load(QUrl("about:blank"));
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(!page.settings()->testAttribute(QWebEngineSettings::JavascriptEnabled));

    page.setHtml("<html><body>"
                 "<script>document.write('PASS')</script>"
                 "<noscript>FAIL</noscript>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(page.settings()->testAttribute(QWebEngineSettings::JavascriptEnabled));
    QCOMPARE(toPlainTextSync(&page), QStringLiteral("PASS"));
}

void tst_QWebEngineSettings::disableReadingFromCanvas_data()
{
    QTest::addColumn<bool>("disableReadingFromCanvas");
    QTest::addColumn<bool>("result");
    QTest::newRow("disabled") << false << true;
    QTest::newRow("enabled") << true << false;
}

void tst_QWebEngineSettings::disableReadingFromCanvas()
{
    QFETCH(bool, disableReadingFromCanvas);
    QFETCH(bool, result);

    QWebEnginePage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    page.settings()->setAttribute(QWebEngineSettings::ReadingFromCanvasEnabled,
                                  !disableReadingFromCanvas);
    page.setHtml("<html><body>"
                 "<canvas id='myCanvas' width='200' height='40' style='border:1px solid "
                 "#000000;'></canvas>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QCOMPARE(page.settings()->testAttribute(QWebEngineSettings::ReadingFromCanvasEnabled),
             !disableReadingFromCanvas);

    const QString jsCode("(function(){"
                         "   var canvas = document.getElementById(\"myCanvas\");"
                         "   var ctx = canvas.getContext(\"2d\");"
                         "   ctx.fillStyle = \"rgb(255,0,255)\";"
                         "   ctx.fillRect(0, 0, 200, 40);"
                         "   try {"
                         "      src = canvas.toDataURL();"
                         "   }"
                         "   catch(err) {"
                         "      src = \"\";"
                         "   }"
                         "   return src.length ? true : false;"
                         "})();");
    QCOMPARE(evaluateJavaScriptSync(&page, jsCode).toBool(), result);
}

void tst_QWebEngineSettings::forceDarkMode()
{
    QWebEnginePage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);

    // based on: https://developer.chrome.com/blog/auto-dark-theme/#detecting-auto-dark-theme
    page.setHtml("<html><body>"
                 "<div id=\"detection\", style=\"display: none; background-color: canvas; color-scheme: light\"</div>"
                 "</body></html>");

    const QString isAutoDark("(() => {"
                         "  const detectionDiv = document.querySelector('#detection');"
                         "  return getComputedStyle(detectionDiv).backgroundColor != 'rgb(255, 255, 255)';"
                         "})()");

    QVERIFY(loadFinishedSpy.wait());
    QTRY_COMPARE(evaluateJavaScriptSync(&page, isAutoDark).toBool(), false);
    page.settings()->setAttribute(QWebEngineSettings::ForceDarkMode, true);
    QTRY_COMPARE(evaluateJavaScriptSync(&page, isAutoDark).toBool(), true);
}

void tst_QWebEngineSettings::forceDarkModeMultiView()
{
    QWebEngineView view1;
    QWebEngineView view2;
    QWebEnginePage *page1 = view1.page();
    QWebEnginePage *page2 = view2.page();
    page1->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    page2->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    view1.resize(300,300);
    view2.resize(300,300);
    view1.show();
    view2.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view1));
    QVERIFY(QTest::qWaitForWindowExposed(&view2));

    QSignalSpy loadFinishedSpy(page1, SIGNAL(loadFinished(bool)));
    QSignalSpy loadFinishedSpy2(page2, SIGNAL(loadFinished(bool)));
    QString html("<html><body>"
                 "<div id=\"detection\", style=\"display: none; background-color: canvas; color-scheme: light\"</div>"
                 "</body></html>");

    const QString isAutoDark("(() => {"
                            "  const detectionDiv = document.querySelector('#detection');"
                            "  return getComputedStyle(detectionDiv).backgroundColor != 'rgb(255, 255, 255)';"
                            "})()");

    view1.setHtml(html);
    QVERIFY(loadFinishedSpy.wait());
    view2.setHtml(html);
    QVERIFY(loadFinishedSpy2.wait());

    // both views has light color-scheme
    QTRY_COMPARE(evaluateJavaScriptSync(page1, isAutoDark).toBool(), false);
    QTRY_COMPARE(evaluateJavaScriptSync(page2, isAutoDark).toBool(), false);
    view1.settings()->setAttribute(QWebEngineSettings::ForceDarkMode, true);
    // dark mode should apply only for view1
    QTRY_COMPARE(evaluateJavaScriptSync(page1, isAutoDark).toBool(), true);
    QTRY_COMPARE(evaluateJavaScriptSync(page2, isAutoDark).toBool(), false);
}

QTEST_MAIN(tst_QWebEngineSettings)

#include "tst_qwebenginesettings.moc"
