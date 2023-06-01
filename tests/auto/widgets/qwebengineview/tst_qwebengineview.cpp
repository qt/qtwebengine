/*
    Copyright (C) 2016 The Qt Company Ltd.
    Copyright (C) 2009 Torch Mobile Inc.
    Copyright (C) 2009 Girish Ramakrishnan <girish@forwardbias.in>

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
#include <QtWebEngineCore/private/qtwebenginecore-config_p.h>
#include <qtest.h>
#include <util.h>
#include <private/qinputmethod_p.h>
#include <qpainter.h>
#include <qpagelayout.h>
#include <qwebengineview.h>
#include <qwebenginepage.h>
#include <qwebenginesettings.h>
#include <qaction.h>
#include <qdiriterator.h>
#include <qnetworkrequest.h>
#include <qstackedlayout.h>
#include <qtemporarydir.h>
#include <QClipboard>
#include <QCompleter>
#include <QDropEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QHBoxLayout>
#include <QMenu>
#include <QMimeData>
#include <QQuickItem>
#include <QQuickWidget>
#include <QtWebEngineCore/qwebenginehttprequest.h>
#include <QScopeGuard>
#include <QStringListModel>
#include <QTcpServer>
#include <QTcpSocket>
#include <QStyle>
#include <QWebEngineProfile>
#include <QtCore/qregularexpression.h>
#include <QtTest/private/qemulationdetector_p.h>

#define VERIFY_INPUTMETHOD_HINTS(actual, expect) \
    QVERIFY(actual == (expect | Qt::ImhNoPredictiveText | Qt::ImhNoTextHandles | Qt::ImhNoEditMenu));

#define QTRY_COMPARE_WITH_TIMEOUT_FAIL_BLOCK(__expr, __expected, __timeout, __fail_block) \
do { \
    QTRY_IMPL(((__expr) == (__expected)), __timeout);\
    if (__expr != __expected)\
        __fail_block\
    QCOMPARE((__expr), __expected); \
} while (0)

QT_BEGIN_NAMESPACE
namespace QTest {
    int Q_TESTLIB_EXPORT defaultMouseDelay();

    static void mouseEvent(QEvent::Type type, QWidget *widget, const QPoint &pos)
    {
        QTest::qWait(QTest::defaultMouseDelay());
        lastMouseTimestamp += QTest::defaultMouseDelay();
        QMouseEvent me(type, pos, widget->mapToGlobal(pos), Qt::LeftButton, Qt::LeftButton,
                       Qt::NoModifier);
        me.setTimestamp(++lastMouseTimestamp);
        QSpontaneKeyEvent::setSpontaneous(&me);
        qApp->sendEvent(widget, &me);
    }

    static void mouseMultiClick(QWidget *widget, const QPoint pos, int clickCount)
    {
        for (int i = 0; i < clickCount; ++i) {
            mouseEvent(QMouseEvent::MouseButtonPress, widget, pos);
            mouseEvent(QMouseEvent::MouseButtonRelease, widget, pos);
        }
        lastMouseTimestamp += mouseDoubleClickInterval;
    }
}
QT_END_NAMESPACE

class tst_QWebEngineView : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void renderingAfterMaxAndBack();
    void renderHints();
    void getWebKitVersion();

    void changePage_data();
    void changePage();
    void reusePage_data();
    void reusePage();
    void setLoadedPage();
    void microFocusCoordinates();
    void focusInputTypes();
    void unhandledKeyEventPropagation();
    void horizontalScrollbarTest();

    void crashTests();
#if !(defined(WTF_USE_QT_MOBILE_THEME) && WTF_USE_QT_MOBILE_THEME)
    void setPalette_data();
    void setPalette();
#endif
    void doNotSendMouseKeyboardEventsWhenDisabled();
    void doNotSendMouseKeyboardEventsWhenDisabled_data();
    void stopSettingFocusWhenDisabled();
    void stopSettingFocusWhenDisabled_data();
    void focusOnNavigation_data();
    void focusOnNavigation();
    void focusInternalRenderWidgetHostViewQuickItem();
    void doNotBreakLayout();

    void changeLocale();
    void mixLangLocale_data();
    void mixLangLocale();
    void inputMethodsTextFormat_data();
    void inputMethodsTextFormat();
    void keyboardEvents();
    void keyboardFocusAfterPopup();
    void mouseClick();
    void postData();
    void inputFieldOverridesShortcuts();

    void softwareInputPanel();
    void inputContextQueryInput();
    void inputMethods();
    void textSelectionInInputField();
    void textSelectionOutOfInputField();
    void hiddenText();
    void emptyInputMethodEvent();
    void imeComposition();
    void imeCompositionQueryEvent_data();
    void imeCompositionQueryEvent();
    void newlineInTextarea();
    void imeJSInputEvents();

    void mouseLeave();

#if QT_CONFIG(clipboard)
    void globalMouseSelection();
#endif
    void noContextMenu();
    void contextMenu_data();
    void contextMenu();
    void webUIURLs_data();
    void webUIURLs();
    void visibilityState();
    void visibilityState2();
    void visibilityState3();
    void jsKeyboardEvent_data();
    void jsKeyboardEvent();
    void deletePage();
    void autoDeleteOnExternalPageDelete();
    void closeOpenerTab();
    void switchPage();
    void setPageDeletesImplicitPage();
    void setPageDeletesImplicitPage2();
    void setViewDeletesImplicitPage();
    void setPagePreservesExplicitPage();
    void setViewPreservesExplicitPage();
    void closeDiscardsPage();
    void loadAfterRendererCrashed();
    void inspectElement();
    void navigateOnDrop_data();
    void navigateOnDrop();
    void datalist();
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_QWebEngineView::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_QWebEngineView::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_QWebEngineView::init()
{
}

// This will be called after every test function.
void tst_QWebEngineView::cleanup()
{
    QTRY_COMPARE(QApplication::topLevelWidgets().size(), 0);
}

void tst_QWebEngineView::renderHints()
{
#if !defined(QWEBENGINEVIEW_RENDERHINTS)
    QSKIP("QWEBENGINEVIEW_RENDERHINTS");
#else
    QWebEngineView webView;

    // default is only text antialiasing + smooth pixmap transform
    QVERIFY(!(webView.renderHints() & QPainter::Antialiasing));
    QVERIFY(webView.renderHints() & QPainter::TextAntialiasing);
    QVERIFY(webView.renderHints() & QPainter::SmoothPixmapTransform);
#if QT_DEPRECATED_SINCE(5, 14)
    QVERIFY(!(webView.renderHints() & QPainter::HighQualityAntialiasing));
#endif
    QVERIFY(!(webView.renderHints() & QPainter::Antialiasing));

    webView.setRenderHint(QPainter::Antialiasing, true);
    QVERIFY(webView.renderHints() & QPainter::Antialiasing);
    QVERIFY(webView.renderHints() & QPainter::TextAntialiasing);
    QVERIFY(webView.renderHints() & QPainter::SmoothPixmapTransform);
#if QT_DEPRECATED_SINCE(5, 14)
    QVERIFY(!(webView.renderHints() & QPainter::HighQualityAntialiasing));
#endif
    QVERIFY(!(webView.renderHints() & QPainter::Antialiasing));

    webView.setRenderHint(QPainter::Antialiasing, false);
    QVERIFY(!(webView.renderHints() & QPainter::Antialiasing));
    QVERIFY(webView.renderHints() & QPainter::TextAntialiasing);
    QVERIFY(webView.renderHints() & QPainter::SmoothPixmapTransform);
#if QT_DEPRECATED_SINCE(5, 14)
    QVERIFY(!(webView.renderHints() & QPainter::HighQualityAntialiasing));
#endif
    QVERIFY(!(webView.renderHints() & QPainter::Antialiasing));

    webView.setRenderHint(QPainter::SmoothPixmapTransform, true);
    QVERIFY(!(webView.renderHints() & QPainter::Antialiasing));
    QVERIFY(webView.renderHints() & QPainter::TextAntialiasing);
    QVERIFY(webView.renderHints() & QPainter::SmoothPixmapTransform);
#if QT_DEPRECATED_SINCE(5, 14)
    QVERIFY(!(webView.renderHints() & QPainter::HighQualityAntialiasing));
#endif
    QVERIFY(!(webView.renderHints() & QPainter::Antialiasing));

    webView.setRenderHint(QPainter::SmoothPixmapTransform, false);
    QVERIFY(webView.renderHints() & QPainter::TextAntialiasing);
    QVERIFY(!(webView.renderHints() & QPainter::SmoothPixmapTransform));
#if QT_DEPRECATED_SINCE(5, 14)
    QVERIFY(!(webView.renderHints() & QPainter::HighQualityAntialiasing));
#endif
    QVERIFY(!(webView.renderHints() & QPainter::Antialiasing));
#endif
}

void tst_QWebEngineView::getWebKitVersion()
{
#if !defined(QWEBENGINEVERSION)
    QSKIP("QWEBENGINEVERSION");
#else
    QVERIFY(qWebKitVersion().toDouble() > 0);
#endif
}

void tst_QWebEngineView::changePage_data()
{
    QString html = "<html><head><title>%1</title>"
                   "<link rel='icon' href='qrc:///resources/image2.png'></head></html>";
    QUrl urlFrom("data:text/html," + html.arg("TitleFrom"));
    QUrl urlTo("data:text/html," + html.arg("TitleTo"));
    QUrl nullPage("data:text/html,<html/>");
    QTest::addColumn<QUrl>("urlFrom");
    QTest::addColumn<QUrl>("urlTo");
    QTest::addColumn<bool>("fromIsNullPage");
    QTest::addColumn<bool>("toIsNullPage");
    QTest::newRow("From empty page to url") << nullPage << urlTo << true << false;
    QTest::newRow("From url to empty content page") << urlFrom << nullPage << false << true;
    QTest::newRow("From one content to another") << urlFrom << urlTo << false << false;
}

void tst_QWebEngineView::changePage()
{
    QScopedPointer<QWebEngineView> view(new QWebEngineView); view->resize(640, 480); view->show();

    QFETCH(QUrl, urlFrom);
    QFETCH(QUrl, urlTo);
    QFETCH(bool, fromIsNullPage);
    QFETCH(bool, toIsNullPage);

    QSignalSpy spyUrl(view.get(), &QWebEngineView::urlChanged);
    QSignalSpy spyTitle(view.get(), &QWebEngineView::titleChanged);
    QSignalSpy spyIconUrl(view.get(), &QWebEngineView::iconUrlChanged);
    QSignalSpy spyIcon(view.get(), &QWebEngineView::iconChanged);

    QScopedPointer<QWebEnginePage> pageFrom(new QWebEnginePage);
    QSignalSpy pageFromLoadSpy(pageFrom.get(), &QWebEnginePage::loadFinished);
    QSignalSpy pageFromIconLoadSpy(pageFrom.get(), &QWebEnginePage::iconChanged);
    pageFrom->load(urlFrom);
    QTRY_COMPARE(pageFromLoadSpy.size(), 1);
    QCOMPARE(pageFromLoadSpy.last().value(0).toBool(), true);
    if (!fromIsNullPage) {
        QTRY_COMPARE(pageFromIconLoadSpy.size(), 1);
        QVERIFY(!pageFromIconLoadSpy.last().value(0).isNull());
    }

    view->setPage(pageFrom.get());
    QCOMPARE(view->page(), pageFrom.get());
    QCOMPARE(QWebEngineView::forPage(pageFrom.get()), view.get());

    QTRY_COMPARE(spyUrl.size(), 1);
    QCOMPARE(spyUrl.last().value(0).toUrl(), pageFrom->url());
    QTRY_COMPARE(spyTitle.size(), 1);
    QCOMPARE(spyTitle.last().value(0).toString(), pageFrom->title());

    QTRY_COMPARE(spyIconUrl.size(), fromIsNullPage ? 0 : 1);
    QTRY_COMPARE(spyIcon.size(), fromIsNullPage ? 0 : 1);
    if (!fromIsNullPage) {
        QVERIFY(!pageFrom->iconUrl().isEmpty());
        QCOMPARE(spyIconUrl.last().value(0).toUrl(), pageFrom->iconUrl());
        QCOMPARE(spyIcon.last().value(0).value<QIcon>().availableSizes(),
                 pageFrom->icon().availableSizes());
    }

    QScopedPointer<QWebEnginePage> pageTo(new QWebEnginePage);
    QSignalSpy pageToLoadSpy(pageTo.get(), &QWebEnginePage::loadFinished);
    QSignalSpy pageToIconLoadSpy(pageTo.get(), &QWebEnginePage::iconChanged);
    pageTo->load(urlTo);
    QTRY_COMPARE(pageToLoadSpy.size(), 1);
    QCOMPARE(pageToLoadSpy.last().value(0).toBool(), true);
    if (!toIsNullPage) {
        QTRY_COMPARE(pageToIconLoadSpy.size(), 1);
        QVERIFY(!pageToIconLoadSpy.last().value(0).isNull());
    }

    view->setPage(pageTo.get());
    QCOMPARE(view->page(), pageTo.get());
    QCOMPARE(QWebEngineView::forPage(pageTo.get()), view.get());
    QCOMPARE(QWebEngineView::forPage(pageFrom.get()), nullptr);

    QTRY_COMPARE(spyUrl.size(), 2);
    QCOMPARE(spyUrl.last().value(0).toUrl(), pageTo->url());
    QTRY_COMPARE(spyTitle.size(), 2);
    QCOMPARE(spyTitle.last().value(0).toString(), pageTo->title());

    bool iconIsSame = fromIsNullPage == toIsNullPage;
    int iconChangeNotifyCount = fromIsNullPage ? (iconIsSame ? 0 : 1) : (iconIsSame ? 1 : 2);

    QTRY_COMPARE(spyIconUrl.size(), iconChangeNotifyCount);
    QTRY_COMPARE(spyIcon.size(), iconChangeNotifyCount);
    QCOMPARE(pageFrom->iconUrl() == pageTo->iconUrl(), iconIsSame);
    if (!iconIsSame) {
        QCOMPARE(spyIconUrl.last().value(0).toUrl(), pageTo->iconUrl());
        QCOMPARE(spyIcon.last().value(0).value<QIcon>().availableSizes(),
                 pageTo->icon().availableSizes());
    }

    // verify no emits on destroy with the same number of signals in spy
    view.reset();
    qApp->processEvents();
    QTRY_COMPARE(spyUrl.size(), 2);
    QTRY_COMPARE(spyTitle.size(), 2);
    QTRY_COMPARE(spyIconUrl.size(), iconChangeNotifyCount);
    QTRY_COMPARE(spyIcon.size(), iconChangeNotifyCount);
}

void tst_QWebEngineView::reusePage_data()
{
    QTest::addColumn<QString>("html");
    QTest::newRow("WithoutPlugin") << "<html><body id='b'>text</body></html>";
    QTest::newRow("WindowedPlugin") << QString("<html><body id='b'>text<embed src='resources/test.swf'></embed></body></html>");
    QTest::newRow("WindowlessPlugin") << QString("<html><body id='b'>text<embed src='resources/test.swf' wmode=\"transparent\"></embed></body></html>");
}

void tst_QWebEngineView::reusePage()
{
    if (!QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);

    QDir::setCurrent(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath());

    QFETCH(QString, html);
    QWebEngineView* view1 = new QWebEngineView;
    QPointer<QWebEnginePage> page = new QWebEnginePage;
    view1->setPage(page.data());
    page.data()->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    page->setHtml(html, QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()));
    if (html.contains("</embed>")) {
        // some reasonable time for the PluginStream to feed test.swf to flash and start painting
        QSignalSpy spyFinished(view1, &QWebEngineView::loadFinished);
        QVERIFY(spyFinished.wait(20000));
    }

    view1->show();
    QVERIFY(QTest::qWaitForWindowExposed(view1));
    delete view1;
    QVERIFY(page != nullptr); // deleting view must not have deleted the page, since it's not a child of view

    QWebEngineView *view2 = new QWebEngineView;
    view2->setPage(page.data());
    view2->show(); // in Windowless mode, you should still be able to see the plugin here
    QVERIFY(QTest::qWaitForWindowExposed(view2));
    delete view2;

    delete page.data(); // must not crash

    QDir::setCurrent(QApplication::applicationDirPath());
}

void tst_QWebEngineView::setLoadedPage()
{
    // MEMO load page first to make sure that just simple attach to view would draw its content
    QWebEnginePage page;
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    page.setHtml(QString("<html><body bgcolor=\"%1\"></body></html>").arg(QColor(Qt::yellow).name()));
    QTRY_VERIFY(loadSpy.size() == 1 && loadSpy.first().first().toBool());

    QWebEngineView view;
    view.resize(480, 320);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    view.setPage(&page);
    QTRY_COMPARE(view.grab().toImage().pixelColor(QPoint(view.width() / 2, view.height() / 2)), Qt::yellow);
}

// Class used in crashTests
class WebViewCrashTest : public QObject {
    Q_OBJECT
    QWebEngineView* m_view;
public:
    bool m_invokedStop;
    bool m_stopBypassed;


    WebViewCrashTest(QWebEngineView* view)
      : m_view(view)
      , m_invokedStop(false)
      , m_stopBypassed(false)
    {
        view->connect(view, SIGNAL(loadProgress(int)), this, SLOT(loading(int)));
    }

private Q_SLOTS:
    void loading(int progress)
    {
        qDebug() << "progress: " << progress;
        if (progress > 0 && progress < 100) {
            QVERIFY(!m_invokedStop);
            m_view->stop();
            m_invokedStop = true;
        } else if (!m_invokedStop && progress == 100) {
            m_stopBypassed = true;
        }
    }
};


// Should not crash.
void tst_QWebEngineView::crashTests()
{
    // Test if loading can be stopped in loadProgress handler without crash.
    // Test page should have frames.
    QWebEngineView view;
    WebViewCrashTest tester(&view);
    QUrl url("qrc:///resources/index.html");
    view.load(url);

    // If the verification fails, it means that either stopping doesn't work, or the hardware is
    // too slow to load the page and thus to slow to issue the first loadProgress > 0 signal.
    QTRY_VERIFY_WITH_TIMEOUT(tester.m_invokedStop || tester.m_stopBypassed, 10000);
    if (tester.m_stopBypassed)
        QEXPECT_FAIL("", "Loading was too fast to stop", Continue);
    QVERIFY(tester.m_invokedStop);
}

void tst_QWebEngineView::microFocusCoordinates()
{
    QWebEngineView webView;
    webView.resize(640, 480);
    webView.show();
    QVERIFY(QTest::qWaitForWindowExposed(&webView));

    QSignalSpy scrollSpy(webView.page(), SIGNAL(scrollPositionChanged(QPointF)));
    QSignalSpy loadFinishedSpy(&webView, SIGNAL(loadFinished(bool)));
    webView.page()->setHtml("<html><body>"
                            "<input type='text' id='input1' value='' maxlength='20'/><br>"
                            "<canvas id='canvas1' width='500' height='500'></canvas>"
                            "<input type='password'/><br>"
                            "<canvas id='canvas2' width='500' height='500'></canvas>"
                            "</body></html>");
    QVERIFY(loadFinishedSpy.wait());

    evaluateJavaScriptSync(webView.page(), "document.getElementById('input1').focus()");
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));

    QTRY_VERIFY(webView.focusProxy()->inputMethodQuery(Qt::ImCursorRectangle).isValid());
    QVariant initialMicroFocus = webView.focusProxy()->inputMethodQuery(Qt::ImCursorRectangle);

    evaluateJavaScriptSync(webView.page(), "window.scrollBy(0, 50)");
    QTRY_VERIFY(scrollSpy.size() > 0);

    QTRY_VERIFY(webView.focusProxy()->inputMethodQuery(Qt::ImCursorRectangle).isValid());
    QVariant currentMicroFocus = webView.focusProxy()->inputMethodQuery(Qt::ImCursorRectangle);

    QCOMPARE(initialMicroFocus.toRect().translated(QPoint(0,-50)), currentMicroFocus.toRect());
}

void tst_QWebEngineView::focusInputTypes()
{
    const QPlatformInputContext *platformInputContext = QGuiApplicationPrivate::platformIntegration()->inputContext();
    bool imeHasHiddenTextCapability = platformInputContext && platformInputContext->hasCapability(QPlatformInputContext::HiddenTextCapability);

    QWebEngineView webView;
    webView.resize(640, 480);
    webView.show();
    QVERIFY(QTest::qWaitForWindowExposed(&webView));

    QSignalSpy loadFinishedSpy(&webView, SIGNAL(loadFinished(bool)));
    webView.load(QUrl("qrc:///resources/input_types.html"));
    QVERIFY(loadFinishedSpy.wait());

    auto inputMethodQuery = [&webView](Qt::InputMethodQuery query) {
        QInputMethodQueryEvent event(query);
        QApplication::sendEvent(webView.focusProxy(), &event);
        return event.value(query);
    };

    // 'text' field
    QPoint textInputCenter = elementCenter(webView.page(), "textInput");
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("textInput"));
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), Qt::ImhPreferLowercase);
    QVERIFY(webView.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());

    // 'password' field
    QPoint passwordInputCenter = elementCenter(webView.page(), "passwordInput");
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, passwordInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("passwordInput"));
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), (Qt::ImhSensitiveData | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase | Qt::ImhHiddenText));
    QVERIFY(!webView.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    QTRY_COMPARE(platformInputContext->inputMethodAccepted(), imeHasHiddenTextCapability);

    // 'tel' field
    QPoint telInputCenter = elementCenter(webView.page(), "telInput");
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, telInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("telInput"));
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), Qt::ImhDialableCharactersOnly);
    QVERIFY(webView.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());

    // 'number' field
    QPoint numberInputCenter = elementCenter(webView.page(), "numberInput");
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, numberInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("numberInput"));
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), Qt::ImhFormattedNumbersOnly);
    QVERIFY(webView.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());

    // 'email' field
    QPoint emailInputCenter = elementCenter(webView.page(), "emailInput");
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, emailInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("emailInput"));
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), Qt::ImhEmailCharactersOnly);
    QVERIFY(webView.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());

    // 'url' field
    QPoint urlInputCenter = elementCenter(webView.page(), "urlInput");
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, urlInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("urlInput"));
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), (Qt::ImhUrlCharactersOnly | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase));
    QVERIFY(webView.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());

    // 'password' field
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, passwordInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("passwordInput"));
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), (Qt::ImhSensitiveData | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase | Qt::ImhHiddenText));
    QVERIFY(!webView.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    QTRY_COMPARE(platformInputContext->inputMethodAccepted(), imeHasHiddenTextCapability);

    // 'text' type
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("textInput"));
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), Qt::ImhPreferLowercase);
    QVERIFY(webView.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());

    // 'password' field
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, passwordInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("passwordInput"));
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), (Qt::ImhSensitiveData | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase | Qt::ImhHiddenText));
    QVERIFY(!webView.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    QTRY_COMPARE(platformInputContext->inputMethodAccepted(), imeHasHiddenTextCapability);

    // 'text area' field
    QPoint textAreaCenter = elementCenter(webView.page(), "textArea");
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, textAreaCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("textArea"));
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), (Qt::ImhMultiLine | Qt::ImhPreferLowercase));
    QVERIFY(webView.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
}

class KeyEventRecordingWidget : public QWidget {
public:
    ~KeyEventRecordingWidget() { qDeleteAll(pressEvents); qDeleteAll(releaseEvents); }
    QList<QKeyEvent *> pressEvents;
    QList<QKeyEvent *> releaseEvents;
    void keyPressEvent(QKeyEvent *e) override { pressEvents << e->clone(); }
    void keyReleaseEvent(QKeyEvent *e) override { releaseEvents << e->clone(); }
};

void tst_QWebEngineView::unhandledKeyEventPropagation()
{
    KeyEventRecordingWidget parentWidget;
    QWebEngineView webView(&parentWidget);
    webView.resize(640, 480);
    parentWidget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&webView));

    QSignalSpy loadFinishedSpy(&webView, SIGNAL(loadFinished(bool)));
    webView.load(QUrl("qrc:///resources/keyboardEvents.html"));
    QTRY_VERIFY_WITH_TIMEOUT(loadFinishedSpy.size() > 0, 20000);

    evaluateJavaScriptSync(webView.page(), "document.getElementById('first_div').focus()");
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("first_div"));

    QTest::sendKeyEvent(QTest::Press, webView.focusProxy(), Qt::Key_Right, QString(), Qt::NoModifier);
    QTest::sendKeyEvent(QTest::Release, webView.focusProxy(), Qt::Key_Right, QString(), Qt::NoModifier);
    // Right arrow key is unhandled thus focus is not changed
    QTRY_COMPARE(parentWidget.releaseEvents.size(), 1);
    QCOMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("first_div"));

    QTest::sendKeyEvent(QTest::Press, webView.focusProxy(), Qt::Key_Tab, QString(), Qt::NoModifier);
    QTest::sendKeyEvent(QTest::Release, webView.focusProxy(), Qt::Key_Tab, QString(), Qt::NoModifier);
    // Tab key is handled thus focus is changed
    QTRY_COMPARE(parentWidget.releaseEvents.size(), 2);
    QCOMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("second_div"));

    QTest::sendKeyEvent(QTest::Press, webView.focusProxy(), Qt::Key_Left, QString(), Qt::NoModifier);
    QTest::sendKeyEvent(QTest::Release, webView.focusProxy(), Qt::Key_Left, QString(), Qt::NoModifier);
    // Left arrow key is unhandled thus focus is not changed
    QTRY_COMPARE(parentWidget.releaseEvents.size(), 3);
    QCOMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("second_div"));

    // Focus the button and press 'y'.
    evaluateJavaScriptSync(webView.page(), "document.getElementById('submit_button').focus()");
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), "document.activeElement.id").toString(), QStringLiteral("submit_button"));
    QTest::sendKeyEvent(QTest::Press, webView.focusProxy(), Qt::Key_Y, 'y', Qt::NoModifier);
    QTest::sendKeyEvent(QTest::Release, webView.focusProxy(), Qt::Key_Y, 'y', Qt::NoModifier);
    QTRY_COMPARE(parentWidget.releaseEvents.size(), 4);

    // The page will consume the Tab key to change focus between elements while the arrow
    // keys won't be used.
    QCOMPARE(parentWidget.pressEvents.size(), 3);
    QCOMPARE(parentWidget.pressEvents[0]->key(), (int)Qt::Key_Right);
    QCOMPARE(parentWidget.pressEvents[1]->key(), (int)Qt::Key_Left);
    QCOMPARE(parentWidget.pressEvents[2]->key(), (int)Qt::Key_Y);

    // Key releases will all come back unconsumed.
    QCOMPARE(parentWidget.releaseEvents[0]->key(), (int)Qt::Key_Right);
    QCOMPARE(parentWidget.releaseEvents[1]->key(), (int)Qt::Key_Tab);
    QCOMPARE(parentWidget.releaseEvents[2]->key(), (int)Qt::Key_Left);
    QCOMPARE(parentWidget.releaseEvents[3]->key(), (int)Qt::Key_Y);
}

void tst_QWebEngineView::horizontalScrollbarTest()
{
    QString html("<html><body>"
                 "<div style='width: 1000px; height: 1000px; background-color: green' />"
                 "</body></html>");

    QWebEngineView view;
    view.setFixedSize(600, 600);
    view.show();

    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadSpy(view.page(), SIGNAL(loadFinished(bool)));
    view.setHtml(html);
    QTRY_COMPARE(loadSpy.size(), 1);

    QVERIFY(view.page()->scrollPosition() == QPoint(0, 0));
    QSignalSpy scrollSpy(view.page(), SIGNAL(scrollPositionChanged(QPointF)));

    // Note: The test below assumes that the layout direction is Qt::LeftToRight.
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, QPoint(550, 595));
    scrollSpy.wait();
    QVERIFY(view.page()->scrollPosition().x() > 0);

    // Note: The test below assumes that the layout direction is Qt::LeftToRight.
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, QPoint(20, 595));
    scrollSpy.wait();
    QVERIFY(view.page()->scrollPosition() == QPoint(0, 0));
}


#if !(defined(WTF_USE_QT_MOBILE_THEME) && WTF_USE_QT_MOBILE_THEME)
void tst_QWebEngineView::setPalette_data()
{
    QTest::addColumn<bool>("active");
    QTest::addColumn<bool>("background");
    QTest::newRow("activeBG") << true << true;
    QTest::newRow("activeFG") << true << false;
    QTest::newRow("inactiveBG") << false << true;
    QTest::newRow("inactiveFG") << false << false;
}

// Render a QWebEngineView to a QImage twice, each time with a different palette set,
// verify that images rendered are not the same, confirming WebCore usage of
// custom palette on selections.
void tst_QWebEngineView::setPalette()
{
#if !defined(QWEBCONTENTVIEW_SETPALETTE)
    QSKIP("QWEBCONTENTVIEW_SETPALETTE");
#else
    QString html = "<html><head></head>"
                   "<body>"
                   "Some text here"
                   "</body>"
                   "</html>";

    QFETCH(bool, active);
    QFETCH(bool, background);

    QWidget* activeView = 0;

    // Use controlView to manage active/inactive state of test views by raising
    // or lowering their position in the window stack.
    QWebEngineView controlView;
    controlView.setHtml(html);

    QWebEngineView view1;

    QPalette palette1;
    QBrush brush1(Qt::red);
    brush1.setStyle(Qt::SolidPattern);
    if (active && background) {
        // Rendered image must have red background on an active QWebEngineView.
        palette1.setBrush(QPalette::Active, QPalette::Highlight, brush1);
    } else if (active && !background) {
        // Rendered image must have red foreground on an active QWebEngineView.
        palette1.setBrush(QPalette::Active, QPalette::HighlightedText, brush1);
    } else if (!active && background) {
        // Rendered image must have red background on an inactive QWebEngineView.
        palette1.setBrush(QPalette::Inactive, QPalette::Highlight, brush1);
    } else if (!active && !background) {
        // Rendered image must have red foreground on an inactive QWebEngineView.
        palette1.setBrush(QPalette::Inactive, QPalette::HighlightedText, brush1);
    }

    view1.setPalette(palette1);
    view1.setHtml(html);
    view1.page()->setViewportSize(view1.page()->contentsSize());
    view1.show();

    QTest::qWaitForWindowExposed(&view1);

    if (!active) {
        controlView.show();
        QTest::qWaitForWindowExposed(&controlView);
        activeView = &controlView;
        controlView.activateWindow();
    } else {
        view1.activateWindow();
        activeView = &view1;
    }

    QTRY_COMPARE(QApplication::activeWindow(), activeView);

    view1.page()->triggerAction(QWebEnginePage::SelectAll);

    QImage img1(view1.page()->viewportSize(), QImage::Format_ARGB32);
    QPainter painter1(&img1);
    view1.page()->render(&painter1);
    painter1.end();
    view1.close();
    controlView.close();

    QWebEngineView view2;

    QPalette palette2;
    QBrush brush2(Qt::blue);
    brush2.setStyle(Qt::SolidPattern);
    if (active && background) {
        // Rendered image must have blue background on an active QWebEngineView.
        palette2.setBrush(QPalette::Active, QPalette::Highlight, brush2);
    } else if (active && !background) {
        // Rendered image must have blue foreground on an active QWebEngineView.
        palette2.setBrush(QPalette::Active, QPalette::HighlightedText, brush2);
    } else if (!active && background) {
        // Rendered image must have blue background on an inactive QWebEngineView.
        palette2.setBrush(QPalette::Inactive, QPalette::Highlight, brush2);
    } else if (!active && !background) {
        // Rendered image must have blue foreground on an inactive QWebEngineView.
        palette2.setBrush(QPalette::Inactive, QPalette::HighlightedText, brush2);
    }

    view2.setPalette(palette2);
    view2.setHtml(html);
    view2.page()->setViewportSize(view2.page()->contentsSize());
    view2.show();

    QTest::qWaitForWindowExposed(&view2);

    if (!active) {
        controlView.show();
        QTest::qWaitForWindowExposed(&controlView);
        activeView = &controlView;
        controlView.activateWindow();
    } else {
        view2.activateWindow();
        activeView = &view2;
    }

    QTRY_COMPARE(QApplication::activeWindow(), activeView);

    view2.page()->triggerAction(QWebEnginePage::SelectAll);

    QImage img2(view2.page()->viewportSize(), QImage::Format_ARGB32);
    QPainter painter2(&img2);
    view2.page()->render(&painter2);
    painter2.end();

    view2.close();
    controlView.close();

    QVERIFY(img1 != img2);
#endif
}
#endif

void tst_QWebEngineView::renderingAfterMaxAndBack()
{
#if !defined(QWEBENGINEPAGE_RENDER)
    QSKIP("QWEBENGINEPAGE_RENDER");
#else
    QUrl url = QUrl("data:text/html,<html><head></head>"
                   "<body width=1024 height=768 bgcolor=red>"
                   "</body>"
                   "</html>");

    QWebEngineView view;
    view.page()->load(url);
    QSignalSpy spyFinished(&view, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());
    view.show();

    view.page()->settings()->setMaximumPagesInCache(3);

    QTest::qWaitForWindowExposed(&view);

    QPixmap reference(view.page()->viewportSize());
    reference.fill(Qt::red);

    QPixmap image(view.page()->viewportSize());
    QPainter painter(&image);
    view.page()->render(&painter);

    QCOMPARE(image, reference);

    QUrl url2 = QUrl("data:text/html,<html><head></head>"
                     "<body width=1024 height=768 bgcolor=blue>"
                     "</body>"
                     "</html>");
    view.page()->load(url2);

    QVERIFY(spyFinished.wait());

    view.showMaximized();

    QTest::qWaitForWindowExposed(&view);

    QPixmap reference2(view.page()->viewportSize());
    reference2.fill(Qt::blue);

    QPixmap image2(view.page()->viewportSize());
    QPainter painter2(&image2);
    view.page()->render(&painter2);

    QCOMPARE(image2, reference2);

    view.back();

    QPixmap reference3(view.page()->viewportSize());
    reference3.fill(Qt::red);
    QPixmap image3(view.page()->viewportSize());
    QPainter painter3(&image3);
    view.page()->render(&painter3);

    QCOMPARE(image3, reference3);
#endif
}

class KeyboardAndMouseEventRecordingWidget : public QWidget {
public:
    explicit KeyboardAndMouseEventRecordingWidget(QWidget *parent = 0) :
        QWidget(parent), m_eventCounter(0) {}

    bool event(QEvent *event) override
    {
        QString eventString;
        switch (event->type()) {
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
        case QEvent::TabletMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        case QEvent::TouchCancel:
        case QEvent::ContextMenu:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
#if QT_CONFIG(wheelevent)
        case QEvent::Wheel:
#endif
            ++m_eventCounter;
            event->setAccepted(true);
            QDebug(&eventString) << event;
            m_eventHistory.append(eventString);
            return true;
        default:
            break;
        }
        return QWidget::event(event);
    }

    void clearEventCount()
    {
        m_eventCounter = 0;
    }

    int eventCount()
    {
        return m_eventCounter;
    }

    void printEventHistory()
    {
        qDebug() << "Received events are:";
        for (int i = 0; i < m_eventHistory.size(); ++i) {
            qDebug() << m_eventHistory[i];
        }
    }

private:
    int m_eventCounter;
    QList<QString> m_eventHistory;
};

void tst_QWebEngineView::doNotSendMouseKeyboardEventsWhenDisabled()
{
    QFETCH(bool, viewEnabled);
    QFETCH(int, resultEventCount);

    KeyboardAndMouseEventRecordingWidget parentWidget;
    parentWidget.resize(640, 480);
    QWebEngineView webView(&parentWidget);
    webView.setEnabled(viewEnabled);
    parentWidget.setLayout(new QStackedLayout);
    parentWidget.layout()->addWidget(&webView);
    webView.resize(640, 480);
    parentWidget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&webView));

    QSignalSpy loadSpy(&webView, SIGNAL(loadFinished(bool)));
    webView.setHtml("<html><head><title>Title</title></head><body>Hello"
                    "<input id=\"input\" type=\"text\"></body></html>");
    QTRY_COMPARE(loadSpy.size(), 1);

    // When the webView is enabled, the events are swallowed by it, and the parent widget
    // does not receive any events, otherwise all events are processed by the parent widget.
    parentWidget.clearEventCount();
    QTest::mousePress(parentWidget.windowHandle(), Qt::LeftButton);
    QTest::mouseMove(parentWidget.windowHandle(), QPoint(100, 100));
    QTest::mouseRelease(parentWidget.windowHandle(), Qt::LeftButton,
                        Qt::KeyboardModifiers(), QPoint(100, 100));

    // Wait a bit for the mouse events to be processed, so they don't interfere with the js focus
    // below.
    QTest::qWait(100);
    evaluateJavaScriptSync(webView.page(), "document.getElementById(\"input\").focus()");
    QTest::keyPress(parentWidget.windowHandle(), Qt::Key_H);

    // Wait a bit for the key press to be handled. We have to do it, because the compare
    // below could immediately finish successfully, without alloing for the events to be handled.
    QTest::qWait(100);
    QTRY_COMPARE_WITH_TIMEOUT_FAIL_BLOCK(parentWidget.eventCount(), resultEventCount,
                                         1000, parentWidget.printEventHistory(););
}

void tst_QWebEngineView::doNotSendMouseKeyboardEventsWhenDisabled_data()
{
    QTest::addColumn<bool>("viewEnabled");
    QTest::addColumn<int>("resultEventCount");

    QTest::newRow("enabled view receives events") << true << 0;
    QTest::newRow("disabled view does not receive events") << false << 4;
}

void tst_QWebEngineView::stopSettingFocusWhenDisabled()
{
    QFETCH(bool, viewEnabled);
    QFETCH(bool, focusResult);

    QWebEngineView webView;
    webView.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    webView.resize(640, 480);
    webView.show();
    webView.setEnabled(viewEnabled);
    QVERIFY(QTest::qWaitForWindowExposed(&webView));

    QSignalSpy loadSpy(&webView, SIGNAL(loadFinished(bool)));
    webView.setHtml("<html><head><title>Title</title></head><body>Hello"
                    "<input id=\"input\" type=\"text\"></body></html>");
    QTRY_COMPARE(loadSpy.size(), 1);

    QTRY_COMPARE_WITH_TIMEOUT(webView.hasFocus(), focusResult, 1000);
    evaluateJavaScriptSync(webView.page(), "document.getElementById(\"input\").focus()");
    QTRY_COMPARE_WITH_TIMEOUT(webView.hasFocus(), focusResult, 1000);
}

void tst_QWebEngineView::stopSettingFocusWhenDisabled_data()
{
    QTest::addColumn<bool>("viewEnabled");
    QTest::addColumn<bool>("focusResult");

    QTest::newRow("enabled view gets focus") << true << true;
    QTest::newRow("disabled view does not get focus") << false << false;
}

void tst_QWebEngineView::focusOnNavigation_data()
{
    QTest::addColumn<bool>("focusOnNavigation");
    QTest::addColumn<bool>("viewReceivedFocus");
    QTest::newRow("focusOnNavigation true") << true << true;
    QTest::newRow("focusOnNavigation false") << false << false;
}

void tst_QWebEngineView::focusOnNavigation()
{
    QFETCH(bool, focusOnNavigation);
    QFETCH(bool, viewReceivedFocus);

#define triggerJavascriptFocus()\
    evaluateJavaScriptSync(webView->page(), "document.getElementById(\"input\").focus()");
#define loadAndTriggerFocusAndCompare()\
    QTRY_COMPARE(loadSpy.count(), 1);\
    triggerJavascriptFocus();\
    QTRY_COMPARE(webView->hasFocus(), viewReceivedFocus);

    // Create a container widget, that will hold a line edit that has initial focus, and a web
    // engine view.
    QScopedPointer<QWidget> containerWidget(new QWidget);
    QLineEdit *label = new QLineEdit;
    label->setText(QString::fromLatin1("Text"));
    label->setFocus();

    // Create the web view, and set its focusOnNavigation property.
    QWebEngineView *webView = new QWebEngineView;
    QWebEngineSettings *settings = webView->page()->settings();
    settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, focusOnNavigation);
    webView->resize(300, 300);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(label);
    layout->addWidget(webView);

    containerWidget->setLayout(layout);
    containerWidget->show();
    QVERIFY(QTest::qWaitForWindowExposed(containerWidget.data()));

    // Load the content, invoke javascript focus on the view, and check which widget has focus.
    QSignalSpy loadSpy(webView, SIGNAL(loadFinished(bool)));
    webView->setHtml("<html><head><title>Title</title></head><body>Hello"
                    "<input id=\"input\" type=\"text\"></body></html>");
    loadAndTriggerFocusAndCompare();

    // Load a different page, and check focus.
    loadSpy.clear();
    webView->setHtml("<html><head><title>Title</title></head><body>Hello 2"
                    "<input id=\"input\" type=\"text\"></body></html>");
    loadAndTriggerFocusAndCompare();

    // Navigate to previous page in history, check focus.
    loadSpy.clear();
    webView->triggerPageAction(QWebEnginePage::Back);
    loadAndTriggerFocusAndCompare();

    // Navigate to next page in history, check focus.
    loadSpy.clear();
    webView->triggerPageAction(QWebEnginePage::Forward);
    loadAndTriggerFocusAndCompare();

    // Reload page, check focus.
    loadSpy.clear();
    webView->triggerPageAction(QWebEnginePage::Reload);
    loadAndTriggerFocusAndCompare();

    // Reload page bypassing cache, check focus.
    loadSpy.clear();
    webView->triggerPageAction(QWebEnginePage::ReloadAndBypassCache);
    loadAndTriggerFocusAndCompare();

    // Manually forcing focus on web view should work.
    webView->setFocus();
    QTRY_COMPARE(webView->hasFocus(), true);


    // Clean up.
#undef loadAndTriggerFocusAndCompare
#undef triggerJavascriptFocus
}

void tst_QWebEngineView::focusInternalRenderWidgetHostViewQuickItem()
{
    // Create a container widget, that will hold a line edit that has initial focus, and a web
    // engine view.
    QScopedPointer<QWidget> containerWidget(new QWidget);
    QLineEdit *label = new QLineEdit;
    label->setText(QString::fromLatin1("Text"));
    label->setFocus();

    // Create the web view, and set its focusOnNavigation property to false, so it doesn't
    // get initial focus.
    QWebEngineView *webView = new QWebEngineView;
    QWebEngineSettings *settings = webView->page()->settings();
    settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    webView->resize(300, 100);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(webView);

    containerWidget->resize(300, 200);
    containerWidget->setLayout(layout);
    containerWidget->show();
    QVERIFY(QTest::qWaitForWindowExposed(containerWidget.data()));

    // Load the content, and check that focus is not set.
    QSignalSpy loadSpy(webView, SIGNAL(loadFinished(bool)));
    webView->setHtml("<html><body>"
                     "  <input id='input1' type='text'/>"
                     "</body></html>");
    QTRY_COMPARE(loadSpy.size(), 1);
    QTRY_COMPARE(webView->hasFocus(), false);

    // Manually trigger focus.
    webView->setFocus();

    // Check that focus is set in QWebEngineView and all internal classes.
    QTRY_COMPARE(webView->hasFocus(), true);

    QQuickWidget *webEngineQuickWidget = qobject_cast<QQuickWidget *>(webView->focusProxy());
    QVERIFY(webEngineQuickWidget);
    QTRY_COMPARE(webEngineQuickWidget->hasFocus(), true);

    QQuickItem *root = webEngineQuickWidget->rootObject();
    // The root item should not has focus, otherwise it would handle input events
    // instead of the RenderWidgetHostViewQtDelegateItem.
    QVERIFY(!root->hasFocus());

    QCOMPARE(root->childItems().size(), 1);
    QQuickItem *renderWidgetHostViewQtDelegateItem = root->childItems().at(0);
    QVERIFY(renderWidgetHostViewQtDelegateItem);
    QTRY_COMPARE(renderWidgetHostViewQtDelegateItem->hasFocus(), true);
    // Test if QWebEngineView handles key events.
    QTRY_COMPARE(renderWidgetHostViewQtDelegateItem->hasActiveFocus(), true);

    // Key events should not be forwarded to the unfocused input field.
    QTRY_COMPARE(evaluateJavaScriptSync(webView->page(),
                                        "document.getElementById('input1').value").toString(),
                                        QStringLiteral(""));
    QTest::keyClick(webView->focusProxy(), Qt::Key_X);
    QTest::qWait(100);
    QTRY_COMPARE(evaluateJavaScriptSync(webView->page(),
                 "document.getElementById('input1').value").toString(),
                 QStringLiteral(""));

    // Focus the input field. Focus rectangle is expected to appear around the input field.
    evaluateJavaScriptSync(webView->page(), "document.getElementById('input1').focus()");
    QTRY_COMPARE(evaluateJavaScriptSync(webView->page(),
                                        "document.activeElement.id").toString(),
                                        QStringLiteral("input1"));

    // Test the focused input field with a key event.
    QTest::keyClick(webView->focusProxy(), Qt::Key_X);
    QTRY_COMPARE(evaluateJavaScriptSync(webView->page(),
                                        "document.getElementById('input1').value").toString(),
                                        QStringLiteral("x"));
}

void tst_QWebEngineView::doNotBreakLayout()
{
    QScopedPointer<QWidget> containerWidget(new QWidget);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(new QWidget);
    layout->addWidget(new QWidget);
    layout->addWidget(new QWidget);
    layout->addWidget(new QWebEngineView);

    containerWidget->setLayout(layout);
    containerWidget->setGeometry(50, 50, 800, 600);
    containerWidget->show();
    QVERIFY(QTest::qWaitForWindowExposed(containerWidget.data()));

    QSize previousSize = static_cast<QWidgetItem *>(layout->itemAt(0))->widget()->size();
    for (int i = 1; i < layout->count(); i++) {
        QSize actualSize = static_cast<QWidgetItem *>(layout->itemAt(i))->widget()->size();
        // There could be smaller differences on some platforms
        QVERIFY(qAbs(previousSize.width() - actualSize.width()) <= 2);
        QVERIFY(qAbs(previousSize.height() - actualSize.height()) <= 2);
        previousSize = actualSize;
    }
}

void tst_QWebEngineView::changeLocale()
{
    if (QTestPrivate::isRunningArmOnX86())
        QSKIP("Does not work with QEMU. (QTBUG-94911)");

    QStringList errorLines;
    QUrl url("http://non.existent/");

    QLocale::setDefault(QLocale("de"));
    QWebEngineView viewDE;
    QSignalSpy loadFinishedSpyDE(&viewDE, SIGNAL(loadFinished(bool)));
    viewDE.load(url);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpyDE.size(), 1, 20000);

    QTRY_VERIFY(!toPlainTextSync(viewDE.page()).isEmpty());
    errorLines = toPlainTextSync(viewDE.page()).split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
    QCOMPARE(errorLines.first().toUtf8(), QByteArrayLiteral("Die Website ist nicht erreichbar"));

    QLocale::setDefault(QLocale("en"));
    QWebEngineView viewEN;
    QSignalSpy loadFinishedSpyEN(&viewEN, SIGNAL(loadFinished(bool)));
    viewEN.load(url);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpyEN.size(), 1, 20000);

    QTRY_VERIFY(!toPlainTextSync(viewEN.page()).isEmpty());
    errorLines = toPlainTextSync(viewEN.page()).split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
    QCOMPARE(errorLines.first().toUtf8(), QByteArrayLiteral("This site can\xE2\x80\x99t be reached"));

    // Reset error page
    viewDE.load(QUrl("about:blank"));
    QVERIFY(loadFinishedSpyDE.wait());
    loadFinishedSpyDE.clear();

    // Check whether an existing QWebEngineView keeps the language settings after changing the default locale
    viewDE.load(url);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpyDE.size(), 1, 20000);

    QTRY_VERIFY(!toPlainTextSync(viewDE.page()).isEmpty());
    errorLines = toPlainTextSync(viewDE.page()).split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
    QCOMPARE(errorLines.first().toUtf8(), QByteArrayLiteral("Die Website ist nicht erreichbar"));
}

void tst_QWebEngineView::mixLangLocale_data()
{
    QTest::addColumn<QString>("locale");
    QTest::addColumn<QByteArray>("formattedNumber");
    QTest::newRow("en_DK") << "en-DK" << QByteArray("1.234.567.890");
    QTest::newRow("de")    << "de"    << QByteArray("1.234.567.890");
    QTest::newRow("de_CH") << "de-CH" << QByteArray("1’234’567’890");
    QTest::newRow("eu_ES") << "eu-ES" << QByteArray("1.234.567.890");
    QTest::newRow("hu_HU") << "hu-HU" << QByteArray("1\xC2\xA0""234\xC2\xA0""567\xC2\xA0""890"); // no-break spaces
}

void tst_QWebEngineView::mixLangLocale()
{
    QFETCH(QString, locale);
    QFETCH(QByteArray, formattedNumber);

    QLocale::setDefault(QLocale(locale));

    QWebEngineView view;
    QSignalSpy loadSpy(&view, &QWebEngineView::loadFinished);

    bool terminated = false;
    auto sc = connect(view.page(), &QWebEnginePage::renderProcessTerminated, [&] () { terminated = true; });

    view.load(QUrl("qrc:///resources/dummy.html"));
    QTRY_VERIFY(terminated || loadSpy.size() == 1);

    QVERIFY2(!terminated,
        qPrintable(QString("Locale [%1] terminated: %2, loaded: %3").arg(locale).arg(terminated).arg(loadSpy.size())));
    QVERIFY(loadSpy.first().first().toBool());

    QString content = toPlainTextSync(view.page());
    QVERIFY2(!content.isEmpty() && content.contains("test content"), qPrintable(content));

    QCOMPARE(evaluateJavaScriptSync(view.page(), "navigator.language").toString(), QLocale().bcp47Name());

    if (locale == "eu-ES")
        QEXPECT_FAIL("", "Basque number formatting is somehow dependent on environment", Continue);
    QCOMPARE(evaluateJavaScriptSync(view.page(), "Number(1234567890).toLocaleString()").toByteArray(), formattedNumber);

    QLocale::setDefault(QLocale("en"));
}

void tst_QWebEngineView::inputMethodsTextFormat_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("length");
    QTest::addColumn<int>("underlineStyle");
    QTest::addColumn<QColor>("underlineColor");
    QTest::addColumn<QColor>("backgroundColor");

    QTest::newRow("") << QString("") << 0 << 0 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Q") << QString("Q") << 0 << 1 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Qt") << QString("Qt") << 0 << 1 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Qt") << QString("Qt") << 0 << 2 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Qt") << QString("Qt") << 1 << 1 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Qt ") << QString("Qt ") << 0 << 1 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Qt ") << QString("Qt ") << 1 << 1 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Qt ") << QString("Qt ") << 2 << 1 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Qt ") << QString("Qt ") << 2 << -1 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Qt ") << QString("Qt ") << -2 << 3 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Qt ") << QString("Qt ") << -1 << -1 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("Qt ") << QString("Qt ") << 0 << 3 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("The Qt") << QString("The Qt") << 0 << 1 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("The Qt Company") << QString("The Qt Company") << 0 << 1 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("red") << QColor();
    QTest::newRow("The Qt Company") << QString("The Qt Company") << 0 << 3 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("green") << QColor();
    QTest::newRow("The Qt Company") << QString("The Qt Company") << 4 << 2 << static_cast<int>(QTextCharFormat::SingleUnderline) << QColor("green") << QColor("red");
    QTest::newRow("The Qt Company") << QString("The Qt Company") << 7 << 7 << static_cast<int>(QTextCharFormat::NoUnderline) << QColor("green") << QColor("red");
    QTest::newRow("The Qt Company") << QString("The Qt Company") << 7 << 7 << static_cast<int>(QTextCharFormat::NoUnderline) << QColor() << QColor("red");
}


void tst_QWebEngineView::inputMethodsTextFormat()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));

    view.setHtml("<html><body>"
                 " <input type='text' id='input1' style='font-family: serif' value='' maxlength='20'/>"
                 "</body></html>");
    QTRY_COMPARE(loadFinishedSpy.size(), 1);

    evaluateJavaScriptSync(view.page(), "document.getElementById('input1').focus()");
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QFETCH(QString, string);
    QFETCH(int, start);
    QFETCH(int, length);
    QFETCH(int, underlineStyle);
    QFETCH(QColor, underlineColor);
    QFETCH(QColor, backgroundColor);

    QList<QInputMethodEvent::Attribute> attrs;
    QTextCharFormat format;
    format.setUnderlineStyle(static_cast<QTextCharFormat::UnderlineStyle>(underlineStyle));
    format.setUnderlineColor(underlineColor);

    // Setting background color is disabled for Qt WebEngine because some IME manager
    // sets background color to black and there is no API for setting the foreground color.
    // This may result black text on black background. However, we still test it to ensure
    // changing background color doesn't cause any crash.
    if (backgroundColor.isValid())
        format.setBackground(QBrush(backgroundColor));

    attrs.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, start, length, format));

    QInputMethodEvent im(string, attrs);
    QVERIFY(QApplication::sendEvent(view.focusProxy(), &im));
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), string);
}

void tst_QWebEngineView::keyboardEvents()
{
    QWebEngineView view;
    view.show();
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.load(QUrl("qrc:///resources/keyboardEvents.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);

    QStringList elements;
    elements << "first_div" << "second_div";
    elements << "text_input" << "radio1" << "checkbox1" << "checkbox2";
    elements << "number_input" << "range_input" << "search_input";
    elements << "submit_button" << "combobox" << "first_hyperlink" << "second_hyperlink";

    // Iterate over the elements of the test page with the Tab key. This tests whether any
    // element blocks the in-page navigation by Tab.
    for (const QString &elementId : elements) {
        QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), elementId);
        QTest::keyPress(view.focusProxy(), Qt::Key_Tab);
    }

    // Move back to the radio buttons with the Shift+Tab key combination
    for (int i = 0; i < 10; ++i)
        QTest::keyPress(view.focusProxy(), Qt::Key_Tab, Qt::ShiftModifier);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("radio2"));

    // Test the Space key by checking a radio button
    QVERIFY(!evaluateJavaScriptSync(view.page(), "document.getElementById('radio2').checked").toBool());
    QTest::keyClick(view.focusProxy(), Qt::Key_Space);
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('radio2').checked").toBool());

    // Test the Left key by switching the radio button
    QVERIFY(!evaluateJavaScriptSync(view.page(), "document.getElementById('radio1').checked").toBool());
    QTest::keyPress(view.focusProxy(), Qt::Key_Left);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("radio1"));
    QVERIFY(!evaluateJavaScriptSync(view.page(), "document.getElementById('radio2').checked").toBool());
    QVERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('radio1').checked").toBool());

    // Test the Space key by unchecking a checkbox
    evaluateJavaScriptSync(view.page(), "document.getElementById('checkbox1').focus()");
    QVERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('checkbox1').checked").toBool());
    QTest::keyClick(view.focusProxy(), Qt::Key_Space);
    QTRY_VERIFY(!evaluateJavaScriptSync(view.page(), "document.getElementById('checkbox1').checked").toBool());

    // Test the Up and Down keys by changing the value of a spinbox
    evaluateJavaScriptSync(view.page(), "document.getElementById('number_input').focus()");
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('number_input').value").toInt(), 5);
    QTest::keyPress(view.focusProxy(), Qt::Key_Up);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('number_input').value").toInt(), 6);
    QTest::keyPress(view.focusProxy(), Qt::Key_Down);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('number_input').value").toInt(), 5);

    // Test the Left, Right, Home, PageUp, End and PageDown keys by changing the value of a slider
    evaluateJavaScriptSync(view.page(), "document.getElementById('range_input').focus()");
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('range_input').value").toString(), QStringLiteral("5"));
    QTest::keyPress(view.focusProxy(), Qt::Key_Left);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('range_input').value").toString(), QStringLiteral("4"));
    QTest::keyPress(view.focusProxy(), Qt::Key_Right);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('range_input').value").toString(), QStringLiteral("5"));
    QTest::keyPress(view.focusProxy(), Qt::Key_Home);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('range_input').value").toString(), QStringLiteral("0"));
    QTest::keyPress(view.focusProxy(), Qt::Key_PageUp);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('range_input').value").toString(), QStringLiteral("1"));
    QTest::keyPress(view.focusProxy(), Qt::Key_End);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('range_input').value").toString(), QStringLiteral("10"));
    QTest::keyPress(view.focusProxy(), Qt::Key_PageDown);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('range_input').value").toString(), QStringLiteral("9"));

    // Test the Escape key by removing the content of a search field
    evaluateJavaScriptSync(view.page(), "document.getElementById('search_input').focus()");
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('search_input').value").toString(), QStringLiteral("test"));
    QTest::keyPress(view.focusProxy(), Qt::Key_Escape);
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('search_input').value").toString().isEmpty());

    // Test the alpha keys by changing the values in a combobox
    evaluateJavaScriptSync(view.page(), "document.getElementById('combobox').focus()");
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('combobox').value").toString(), QStringLiteral("a"));
    QTest::keyPress(view.focusProxy(), Qt::Key_B);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('combobox').value").toString(), QStringLiteral("b"));
    // Must wait with the second key press to simulate selection of another element
    QTest::keyPress(view.focusProxy(), Qt::Key_C, Qt::NoModifier, 1100 /* blink::typeAheadTimeout + 0.1s */);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('combobox').value").toString(), QStringLiteral("c"));

    // Test the Enter key by loading a page with a hyperlink
    evaluateJavaScriptSync(view.page(), "document.getElementById('first_hyperlink').focus()");
    QTest::keyPress(view.focusProxy(), Qt::Key_Enter);
    QVERIFY(loadFinishedSpy.wait());
}

class WebViewWithUrlBar : public QWidget {
public:
    QLineEdit *lineEdit = new QLineEdit;
    QCompleter *urlCompleter = new QCompleter({ QStringLiteral("test") }, lineEdit);
    QWebEngineView *webView = new QWebEngineView;
    QVBoxLayout *layout = new QVBoxLayout;

    WebViewWithUrlBar()
    {
        resize(500, 500);
        setLayout(layout);
        layout->addWidget(lineEdit);
        layout->addWidget(webView);
        lineEdit->setCompleter(urlCompleter);
        lineEdit->setFocus();
    }
};

void tst_QWebEngineView::keyboardFocusAfterPopup()
{
    const QString html = QStringLiteral(
        "<html>"
        "  <body onload=\"document.getElementById('input1').focus()\">"
        "    <input id=input1 type=text/>"
        "  </body>"
        "</html>");
    WebViewWithUrlBar window;
    QSignalSpy loadFinishedSpy(window.webView, &QWebEngineView::loadFinished);
    connect(window.lineEdit, &QLineEdit::editingFinished, [&] { window.webView->setHtml(html); });
    window.webView->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    window.show();

    // Focus will initially go to the QLineEdit.
    QTRY_COMPARE(QApplication::focusWidget(), window.lineEdit);

    // Trigger QCompleter's popup and select the first suggestion.
    QTest::keyPress(QApplication::focusWindow(), Qt::Key_T);
    QTest::keyRelease(QApplication::focusWindow(), Qt::Key_T);
    QTRY_VERIFY(QApplication::activePopupWidget());
    QTest::keyPress(QApplication::focusWindow(), Qt::Key_Down);
    QTest::keyRelease(QApplication::focusWindow(), Qt::Key_Down);
    QTest::keyPress(QApplication::focusWindow(), Qt::Key_Enter);
    QTest::keyRelease(QApplication::focusWindow(), Qt::Key_Enter);

    // Due to FocusOnNavigationEnabled, focus should now move to the webView.
    QTRY_COMPARE(QApplication::focusWidget(), window.webView->focusProxy());

    // Keyboard events sent to the window should go to the <input> element.
    QVERIFY(loadFinishedSpy.size() || loadFinishedSpy.wait());
    QTest::keyPress(QApplication::focusWindow(), Qt::Key_X);
    QTest::keyRelease(QApplication::focusWindow(), Qt::Key_X);
    QTRY_COMPARE(evaluateJavaScriptSync(window.webView->page(), "document.getElementById('input1').value").toString(),
                 QStringLiteral("x"));
}

void tst_QWebEngineView::mouseClick()
{
    QWebEngineView view;
    view.show();
    view.resize(200, 200);
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QPoint textInputCenter;

    // Single Click
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    selectionChangedSpy.clear();

    view.setHtml("<html><body>"
                 "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());

    QVERIFY(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString().isEmpty());
    textInputCenter = elementCenter(view.page(), "input");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input"));
    QCOMPARE(selectionChangedSpy.size(), 0);
    QVERIFY(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString().isEmpty());

    // Double click
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    selectionChangedSpy.clear();

    view.setHtml("<html><body onload='document.getElementById(\"input\").focus()'>"
                 "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());

    textInputCenter = elementCenter(view.page(), "input");
    QTest::mouseMultiClick(view.focusProxy(), textInputCenter, 2);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 1);
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QStringLiteral("Company"));

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 2);
    QVERIFY(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString().isEmpty());

    // Triple click
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    selectionChangedSpy.clear();

    view.setHtml("<html><body onload='document.getElementById(\"input\").focus()'>"
                 "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());

    textInputCenter = elementCenter(view.page(), "input");
    QTest::mouseMultiClick(view.focusProxy(), textInputCenter, 3);
    QVERIFY(selectionChangedSpy.wait());
    QTRY_COMPARE(selectionChangedSpy.size(), 2);
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QStringLiteral("The Qt Company"));

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 3);
    QVERIFY(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString().isEmpty());
}

void tst_QWebEngineView::postData()
{
    QMap<QString, QString> postData;
    // use reserved characters to make the test harder to pass
    postData[QStringLiteral("Spä=m")] = QStringLiteral("ëgg:s");
    postData[QStringLiteral("foo\r\n")] = QStringLiteral("ba&r");

    QEventLoop eventloop;

    // Set up dummy "HTTP" server
    QTcpServer server;
    connect(&server, &QTcpServer::newConnection, this, [this, &server, &eventloop, &postData](){
        QTcpSocket* socket = server.nextPendingConnection();

        connect(socket, &QAbstractSocket::disconnected, this, [&eventloop](){
            eventloop.quit();
        });

        connect(socket, &QIODevice::readyRead, this, [socket, &server, &postData](){
            QByteArray rawData = socket->readAll();
            QStringList lines = QString::fromLocal8Bit(rawData).split("\r\n");

            // examine request
            QStringList request = lines[0].split(" ", Qt::SkipEmptyParts);
            bool requestOk = request.size() > 2
                          && request[2].toUpper().startsWith("HTTP/")
                          && request[0].toUpper() == "POST"
                          && request[1] == "/";
            if (!requestOk) // POST and HTTP/... can be switched(?)
                requestOk =  request.size() > 2
                          && request[0].toUpper().startsWith("HTTP/")
                          && request[2].toUpper() == "POST"
                          && request[1] == "/";

            // examine headers
            int line = 1;
            bool headersOk = true;
            for (; headersOk && line < lines.size(); line++) {
                QStringList headerParts = lines[line].split(":");
                if (headerParts.size() < 2)
                    break;
                QString headerKey = headerParts[0].trimmed().toLower();
                QString headerValue = headerParts[1].trimmed().toLower();

                if (headerKey == "host")
                    headersOk = headersOk && (headerValue == "127.0.0.1")
                                          && (headerParts.size() == 3)
                                          && (headerParts[2].trimmed()
                                              == QString::number(server.serverPort()));
                if (headerKey == "content-type")
                    headersOk = headersOk && (headerValue == "application/x-www-form-urlencoded");
            }

            // examine body
            bool bodyOk = true;
            if (lines.size() == line+2) {
                QStringList postedFields = lines[line+1].split("&");
                QMap<QString, QString> postedData;
                for (int i = 0; bodyOk && i < postedFields.size(); i++) {
                    QStringList postedField = postedFields[i].split("=");
                    if (postedField.size() == 2)
                        postedData[QUrl::fromPercentEncoding(postedField[0].toLocal8Bit())]
                                 = QUrl::fromPercentEncoding(postedField[1].toLocal8Bit());
                    else
                        bodyOk = false;
                }
                bodyOk = bodyOk && (postedData == postData);
            } else { // no body at all or more than 1 line
                bodyOk = false;
            }

            // send response
            socket->write("HTTP/1.1 200 OK\r\n");
            socket->write("Content-Type: text/html\r\n");
            socket->write("Content-Length: 39\r\n\r\n");
            if (requestOk && headersOk && bodyOk)
                //             6     6     11         7      7      2 = 39 (Content-Length)
                socket->write("<html><body>Test Passed</body></html>\r\n");
            else
                socket->write("<html><body>Test Failed</body></html>\r\n");
            socket->flush();

            if (!requestOk || !headersOk || !bodyOk) {
                qDebug() << "Dummy HTTP Server: received request was not as expected";
                qDebug() << rawData;
                QVERIFY(requestOk); // one of them will yield useful output and make the test fail
                QVERIFY(headersOk);
                QVERIFY(bodyOk);
            }

            socket->close();
        });
    });
    if (!server.listen())
        QFAIL("Dummy HTTP Server: listen() failed");

    // Manual, hard coded client (commented out, but not removed - for reference and just in case)
    /*
    QTcpSocket client;
    connect(&client, &QIODevice::readyRead, this, [&client, &eventloop](){
        qDebug() << "Dummy HTTP client: data received";
        qDebug() << client.readAll();
        eventloop.quit();
    });
    connect(&client, &QAbstractSocket::connected, this, [&client](){
        client.write("HTTP/1.1 / GET\r\n\r\n");
    });
    client.connectToHost(QHostAddress::LocalHost, server.serverPort());
    */

    // send the POST request
    QWebEngineView view;
    QString sPort = QString::number(server.serverPort());
    view.load(QWebEngineHttpRequest::postRequest(QUrl("http://127.0.0.1:"+sPort), postData));

    // timeout after 10 seconds
    QTimer timeoutGuard(this);
    connect(&timeoutGuard, &QTimer::timeout, this, [&eventloop](){
        eventloop.quit();
        QFAIL("Dummy HTTP Server: waiting for data timed out");
    });
    timeoutGuard.setSingleShot(true);
    timeoutGuard.start(10000);

    // start the test
    eventloop.exec();

    timeoutGuard.stop();
    server.close();
}

void tst_QWebEngineView::inputFieldOverridesShortcuts()
{
    QWebEngineView view;
    bool actionTriggered = false;
    QAction *action = new QAction(&view);
    connect(action, &QAction::triggered, [&actionTriggered] () { actionTriggered = true; });
    view.addAction(action);

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(QString("<html><body>"
                         "<button id=\"btn1\" type=\"button\">push it real good</button>"
                         "<input id=\"input1\" type=\"text\" value=\"x\">"
                         "<input id=\"pass1\" type=\"password\" value=\"x\">"
                         "</body></html>"));
    QVERIFY(loadFinishedSpy.wait());

    view.show();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    auto inputFieldValue = [&view] () -> QString {
        return evaluateJavaScriptSync(view.page(),
                                      "document.getElementById('input1').value").toString();
    };

    auto passwordFieldValue = [&view] () -> QString {
        return evaluateJavaScriptSync(view.page(),
                                      "document.getElementById('pass1').value").toString();
    };

    // The input form is not focused. The action is triggered on pressing Shift+Delete.
    action->setShortcut(Qt::SHIFT | Qt::Key_Delete);
    QTest::keyClick(view.windowHandle(), Qt::Key_Delete, Qt::ShiftModifier);
    QTRY_VERIFY(actionTriggered);
    QCOMPARE(inputFieldValue(), QString("x"));

    // The input form is not focused. The action is triggered on pressing X.
    action->setShortcut(Qt::Key_X);
    actionTriggered = false;
    QTest::keyClick(view.windowHandle(), Qt::Key_X);
    QTRY_VERIFY(actionTriggered);
    QCOMPARE(inputFieldValue(), QString("x"));

    // The input form is focused. The action is not triggered, and the form's text changed.
    evaluateJavaScriptSync(view.page(), "document.getElementById('input1').focus();");
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));
    actionTriggered = false;
    QTest::keyClick(view.windowHandle(), Qt::Key_Y);
    QTRY_COMPARE(inputFieldValue(), QString("yx"));
    QTest::keyClick(view.windowHandle(), Qt::Key_X);
    QTRY_COMPARE(inputFieldValue(), QString("yxx"));
    QVERIFY(!actionTriggered);

    // The password input form is focused. The action is not triggered, and the form's text changed.
    evaluateJavaScriptSync(view.page(), "document.getElementById('pass1').focus();");
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("pass1"));
    actionTriggered = false;
    QTest::keyClick(view.windowHandle(), Qt::Key_Y);
    QTRY_COMPARE(passwordFieldValue(), QString("yx"));
    QTest::keyClick(view.windowHandle(), Qt::Key_X);
    QTRY_COMPARE(passwordFieldValue(), QString("yxx"));
    QVERIFY(!actionTriggered);

    // The input form is focused. Make sure we don't override all short cuts.
    // A Ctrl-1 action is no default Qt key binding and should be triggerable.
    evaluateJavaScriptSync(view.page(), "document.getElementById('input1').focus();");
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));
    action->setShortcut(Qt::CTRL | Qt::Key_1);
    QTest::keyClick(view.windowHandle(), Qt::Key_1, Qt::ControlModifier);
    QTRY_VERIFY(actionTriggered);
    QCOMPARE(inputFieldValue(), QString("yxx"));

    // The input form is focused. The following shortcuts are not overridden
    // thus handled by Qt WebEngine. Make sure the subsequent shortcuts with text
    // character don't cause assert due to an unconsumed editor command.
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QTest::keyClick(view.windowHandle(), Qt::Key_C, Qt::ControlModifier);
    QTest::keyClick(view.windowHandle(), Qt::Key_V, Qt::ControlModifier);
    QTest::keyClick(view.windowHandle(), Qt::Key_V, Qt::ControlModifier);
    QTRY_COMPARE(inputFieldValue(), QString("yxxyxx"));

    // Remove focus from the input field. A QKeySequence::Copy action must be triggerable.
    evaluateJavaScriptSync(view.page(), "document.getElementById('btn1').focus();");
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("btn1"));
    action->setShortcut(QKeySequence::Copy);
    actionTriggered = false;
    QTest::keyClick(view.windowHandle(), Qt::Key_C, Qt::ControlModifier);
    QTRY_VERIFY(actionTriggered);
}

struct InputMethodInfo
{
    InputMethodInfo(const int cursorPosition,
                    const int anchorPosition,
                    QString surroundingText,
                    QString selectedText)
        : cursorPosition(cursorPosition)
        , anchorPosition(anchorPosition)
        , surroundingText(surroundingText)
        , selectedText(selectedText)
    {}

    int cursorPosition;
    int anchorPosition;
    QString surroundingText;
    QString selectedText;
};

class TestInputContext : public QPlatformInputContext
{
public:
    TestInputContext()
    : m_visible(false)
    {
        QInputMethodPrivate* inputMethodPrivate = QInputMethodPrivate::get(qApp->inputMethod());
        inputMethodPrivate->testContext = this;
    }

    ~TestInputContext()
    {
        QInputMethodPrivate* inputMethodPrivate = QInputMethodPrivate::get(qApp->inputMethod());
        inputMethodPrivate->testContext = 0;
    }

    void showInputPanel() override { m_visible = true; }
    void hideInputPanel() override { m_visible = false; }
    bool isInputPanelVisible() const override { return m_visible; }

    void update(Qt::InputMethodQueries queries) override
    {
        if (!qApp->focusObject())
            return;

        if (!(queries & Qt::ImQueryInput))
            return;

        QInputMethodQueryEvent imQueryEvent(Qt::ImQueryInput);
        QApplication::sendEvent(qApp->focusObject(), &imQueryEvent);

        const int cursorPosition = imQueryEvent.value(Qt::ImCursorPosition).toInt();
        const int anchorPosition = imQueryEvent.value(Qt::ImAnchorPosition).toInt();
        QString surroundingText = imQueryEvent.value(Qt::ImSurroundingText).toString();
        QString selectedText = imQueryEvent.value(Qt::ImCurrentSelection).toString();

        infos.append(InputMethodInfo(cursorPosition, anchorPosition, surroundingText, selectedText));
    }

    bool m_visible;
    QList<InputMethodInfo> infos;
};

void tst_QWebEngineView::softwareInputPanel()
{
    TestInputContext testContext;
    QWebEngineView view;
    view.resize(640, 480);
    view.show();

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='' size='50'/>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());

    QPoint textInputCenter = elementCenter(view.page(), "input1");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));

    // This part of the test checks if the SIP (Software Input Panel) is triggered,
    // which normally happens on mobile platforms, when a user input form receives
    // a mouse click.
    int inputPanel = view.style()->styleHint(QStyle::SH_RequestSoftwareInputPanel);

    // For non-mobile platforms RequestSoftwareInputPanel event is not called
    // because there is no SIP (Software Input Panel) triggered. In the case of a
    // mobile platform, an input panel, e.g. virtual keyboard, is usually invoked
    // and the RequestSoftwareInputPanel event is called. For these two situations
    // this part of the test can verified as the checks below.
    if (inputPanel)
        QTRY_VERIFY(testContext.isInputPanelVisible());
    else
        QTRY_VERIFY(!testContext.isInputPanelVisible());
    testContext.hideInputPanel();

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_VERIFY(testContext.isInputPanelVisible());

    view.setHtml("<html><body><p id='para'>nothing to input here</p></body></html>");
    QVERIFY(loadFinishedSpy.wait());
    testContext.hideInputPanel();

    QPoint paraCenter = elementCenter(view.page(), "para");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, paraCenter);

    QVERIFY(!testContext.isInputPanelVisible());

    // Check sending RequestSoftwareInputPanel event
    view.page()->setHtml("<html><body>"
                         "  <input type='text' id='input1' value='QtWebEngine inputMethod'/>"
                         "  <div id='btnDiv' onclick='i=document.getElementById(&quot;input1&quot;); i.focus();'>abc</div>"
                         "</body></html>");
    QVERIFY(loadFinishedSpy.wait());

    QPoint btnDivCenter = elementCenter(view.page(), "btnDiv");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, btnDivCenter);

    QVERIFY(!testContext.isInputPanelVisible());
}

void tst_QWebEngineView::inputContextQueryInput()
{
    QWebEngineView view;
    view.resize(640, 480);
    view.show();

    // testContext will be destroyed before the view, so no events are sent accidentally
    // when the view is destroyed.
    TestInputContext testContext;

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='' size='50'/>"
                 "</body></html>");
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QCOMPARE(testContext.infos.size(), 0);

    // Set focus on an input field.
    QPoint textInputCenter = elementCenter(view.page(), "input1");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(testContext.infos.size(), 2);
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));
    foreach (const InputMethodInfo &info, testContext.infos) {
        QCOMPARE(info.cursorPosition, 0);
        QCOMPARE(info.anchorPosition, 0);
        QCOMPARE(info.surroundingText, QStringLiteral(""));
        QCOMPARE(info.selectedText, QStringLiteral(""));
    }
    testContext.infos.clear();

    // Change content of an input field from JavaScript.
    evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value='QtWebEngine';");
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 11);
    QCOMPARE(testContext.infos[0].anchorPosition, 11);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("QtWebEngine"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    testContext.infos.clear();

    // Change content of an input field by key press.
    QTest::keyClick(view.focusProxy(), Qt::Key_Exclam);
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 12);
    QCOMPARE(testContext.infos[0].anchorPosition, 12);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("QtWebEngine!"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    testContext.infos.clear();

    // Change cursor position.
    QTest::keyClick(view.focusProxy(), Qt::Key_Left);
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 11);
    QCOMPARE(testContext.infos[0].anchorPosition, 11);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("QtWebEngine!"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    testContext.infos.clear();

    // Selection by IME.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent::Attribute newSelection(QInputMethodEvent::Selection, 2, 12, QVariant());
        attributes.append(newSelection);
        QInputMethodEvent event("", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 2);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);

    // As a first step, Chromium moves the cursor to the start of the selection.
    // We don't filter this in QtWebEngine because we don't know yet if this is part of a selection.
    QCOMPARE(testContext.infos[0].cursorPosition, 2);
    QCOMPARE(testContext.infos[0].anchorPosition, 2);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("QtWebEngine!"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));

    // The update of the selection.
    QCOMPARE(testContext.infos[1].cursorPosition, 12);
    QCOMPARE(testContext.infos[1].anchorPosition, 2);
    QCOMPARE(testContext.infos[1].surroundingText, QStringLiteral("QtWebEngine!"));
    QCOMPARE(testContext.infos[1].selectedText, QStringLiteral("WebEngine!"));
    testContext.infos.clear();
    selectionChangedSpy.clear();

    // Clear selection by IME.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent::Attribute newSelection(QInputMethodEvent::Selection, 0, 0, QVariant());
        attributes.append(newSelection);
        QInputMethodEvent event("", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 1);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 0);
    QCOMPARE(testContext.infos[0].anchorPosition, 0);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("QtWebEngine!"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    testContext.infos.clear();
    selectionChangedSpy.clear();

    // Compose text.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("123", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 3);
    QCOMPARE(testContext.infos[0].anchorPosition, 3);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("QtWebEngine!"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QStringLiteral("123QtWebEngine!"));
    testContext.infos.clear();

    // Cancel composition.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 2);
    foreach (const InputMethodInfo &info, testContext.infos) {
        QCOMPARE(info.cursorPosition, 0);
        QCOMPARE(info.anchorPosition, 0);
        QCOMPARE(info.surroundingText, QStringLiteral("QtWebEngine!"));
        QCOMPARE(info.selectedText, QStringLiteral(""));
    }
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QStringLiteral("QtWebEngine!"));
    testContext.infos.clear();

    // Commit text.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString(QStringLiteral("123"), 0, 0);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 3);
    QCOMPARE(testContext.infos[0].anchorPosition, 3);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("123QtWebEngine!"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QStringLiteral("123QtWebEngine!"));
    testContext.infos.clear();

    // Focus out.
    QTest::keyPress(view.focusProxy(), Qt::Key_Tab);
    QTRY_COMPARE(testContext.infos.size(), 1);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral(""));
    testContext.infos.clear();
}

void tst_QWebEngineView::inputMethods()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.settings()->setFontFamily(QWebEngineSettings::SerifFont, view.settings()->fontFamily(QWebEngineSettings::FixedFont));
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' style='font-family: serif' value='' maxlength='20' size='50'/>"
                 "</body></html>");
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QPoint textInputCenter = elementCenter(view.page(), "input1");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));

    // ImCursorRectangle
    QVariant variant = view.focusProxy()->inputMethodQuery(Qt::ImCursorRectangle);
    QVERIFY(elementGeometry(view.page(), "input1").contains(variant.toRect().topLeft()));

    // We assigned the serif font family to be the same as the fixed font family.
    // Then test ImFont on a serif styled element, we should get our fixed font family.
    variant = view.focusProxy()->inputMethodQuery(Qt::ImFont);
    QFont font = variant.value<QFont>();
    QEXPECT_FAIL("", "UNIMPLEMENTED: RenderWidgetHostViewQt::inputMethodQuery(Qt::ImFont)", Continue);
    QCOMPARE(view.settings()->fontFamily(QWebEngineSettings::FixedFont), font.family());

    QList<QInputMethodEvent::Attribute> inputAttributes;

    // Insert text
    {
        QString text = QStringLiteral("QtWebEngine");
        QInputMethodEvent eventText(text, inputAttributes);
        QApplication::sendEvent(view.focusProxy(), &eventText);
        QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), text);
        QCOMPARE(selectionChangedSpy.size(), 0);
    }

    {
        QString text = QStringLiteral("QtWebEngine");
        QInputMethodEvent eventText("", inputAttributes);
        eventText.setCommitString(text, 0, 0);
        QApplication::sendEvent(view.focusProxy(), &eventText);
        QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), text);
        QCOMPARE(selectionChangedSpy.size(), 0);
    }

    // ImMaximumTextLength
    QEXPECT_FAIL("", "UNIMPLEMENTED: RenderWidgetHostViewQt::inputMethodQuery(Qt::ImMaximumTextLength)", Continue);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImMaximumTextLength).toInt(), 20);

    // Set selection
    inputAttributes << QInputMethodEvent::Attribute(QInputMethodEvent::Selection, 3, 2, QVariant());
    QInputMethodEvent eventSelection1("", inputAttributes);

    QApplication::sendEvent(view.focusProxy(), &eventSelection1);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);

    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 3);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 5);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString("eb"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngine"));

    // Set selection with negative length
    inputAttributes << QInputMethodEvent::Attribute(QInputMethodEvent::Selection, 6, -5, QVariant());
    QInputMethodEvent eventSelection2("", inputAttributes);
    QApplication::sendEvent(view.focusProxy(), &eventSelection2);
    QTRY_COMPARE(selectionChangedSpy.size(), 2);

    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 1);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 6);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString("tWebE"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngine"));

    QList<QInputMethodEvent::Attribute> attributes;
    // Clear the selection, so the next test does not clear any contents.
    QInputMethodEvent::Attribute newSelection(QInputMethodEvent::Selection, 0, 0, QVariant());
    attributes.append(newSelection);
    QInputMethodEvent eventComposition("composition", attributes);
    QApplication::sendEvent(view.focusProxy(), &eventComposition);
    QTRY_COMPARE(selectionChangedSpy.size(), 3);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));

    // An ongoing composition should not change the surrounding text before it is committed.
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngine"));

    // Cancel current composition first
    inputAttributes << QInputMethodEvent::Attribute(QInputMethodEvent::Selection, 0, 0, QVariant());
    QInputMethodEvent eventSelection3("", inputAttributes);
    QApplication::sendEvent(view.focusProxy(), &eventSelection3);

    // Cancelling composition should not clear the surrounding text
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngine"));
}

void tst_QWebEngineView::textSelectionInInputField()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='QtWebEngine' size='50'/>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    // Tests for Selection when the Editor is NOT in Composition mode

    // LEFT to RIGHT selection
    // Mouse click event moves the current cursor to the end of the text
    QPoint textInputCenter = elementCenter(view.page(), "input1");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 11);
    // There was no selection to be changed by the click
    QCOMPARE(selectionChangedSpy.size(), 0);

    QList<QInputMethodEvent::Attribute> attributes;
    QInputMethodEvent event(QString(), attributes);
    event.setCommitString("XXX", 0, 0);
    QApplication::sendEvent(view.focusProxy(), &event);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngineXXX"));
    QCOMPARE(selectionChangedSpy.size(), 0);

    event.setCommitString(QString(), -2, 2); // Erase two characters.
    QApplication::sendEvent(view.focusProxy(), &event);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngineX"));
    QCOMPARE(selectionChangedSpy.size(), 0);

    event.setCommitString(QString(), -1, 1); // Erase one character.
    QApplication::sendEvent(view.focusProxy(), &event);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngine"));
    QCOMPARE(selectionChangedSpy.size(), 0);

    // Move to the start of the line
    QTest::keyClick(view.focusProxy(), Qt::Key_Home);

    // Move 2 characters RIGHT
    for (int j = 0; j < 2; ++j)
        QTest::keyClick(view.focusProxy(), Qt::Key_Right);

    // Select to the end of the line
    QTest::keyClick(view.focusProxy(), Qt::Key_End, Qt::ShiftModifier);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 1);

    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 2);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString("WebEngine"));

    // RIGHT to LEFT selection
    // Deselect the selection (this moves the current cursor to the end of the text)
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 2);

    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));

    // Move 2 characters LEFT
    for (int i = 0; i < 2; ++i)
        QTest::keyClick(view.focusProxy(), Qt::Key_Left);

    // Select to the start of the line
    QTest::keyClick(view.focusProxy(), Qt::Key_Home, Qt::ShiftModifier);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 3);

    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 9);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString("QtWebEngi"));
}

void tst_QWebEngineView::textSelectionOutOfInputField()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml("<html><body>"
                 "  This is a text"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QCOMPARE(selectionChangedSpy.size(), 0);
    QVERIFY(!view.hasSelection());
    QVERIFY(view.page()->selectedText().isEmpty());

    // Simple click should not update text selection, however it updates selection bounds in Chromium
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, view.geometry().center());
    QCOMPARE(selectionChangedSpy.size(), 0);
    QVERIFY(!view.hasSelection());
    QVERIFY(view.page()->selectedText().isEmpty());

    // Select text by ctrl+a
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 1);
    QVERIFY(view.hasSelection());
    QCOMPARE(view.page()->selectedText(), QString("This is a text"));

    // Deselect text by mouse click
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, view.geometry().center());
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 2);
    QVERIFY(!view.hasSelection());
    QVERIFY(view.page()->selectedText().isEmpty());

    // Select text by ctrl+a
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 3);
    QVERIFY(view.hasSelection());
    QCOMPARE(view.page()->selectedText(), QString("This is a text"));

    // Deselect text via discard+undiscard
    view.hide();
    view.page()->setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    view.show();
    QVERIFY(loadFinishedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 4);
    QVERIFY(!view.hasSelection());
    QVERIFY(view.page()->selectedText().isEmpty());

    selectionChangedSpy.clear();
    view.setHtml("<html><body>"
                 "  This is a text"
                 "  <br>"
                 "  <input type='text' id='input1' value='QtWebEngine' size='50'/>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QCOMPARE(selectionChangedSpy.size(), 0);
    QVERIFY(!view.hasSelection());
    QVERIFY(view.page()->selectedText().isEmpty());

    // Make sure the input field does not have the focus
    evaluateJavaScriptSync(view.page(), "document.getElementById('input1').blur()");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString().isEmpty());

    // Select the whole page by ctrl+a
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 1);
    QVERIFY(view.hasSelection());
    QVERIFY(view.page()->selectedText().startsWith(QString("This is a text")));

    // Remove selection by clicking into an input field
    QPoint textInputCenter = elementCenter(view.page(), "input1");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));
    QCOMPARE(selectionChangedSpy.size(), 2);
    QVERIFY(!view.hasSelection());
    QVERIFY(view.page()->selectedText().isEmpty());

    // Select the content of the input field by ctrl+a
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 3);
    QVERIFY(view.hasSelection());
    QCOMPARE(view.page()->selectedText(), QString("QtWebEngine"));

    // Deselect input field's text by mouse click
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, view.geometry().center());
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 4);
    QVERIFY(!view.hasSelection());
    QVERIFY(view.page()->selectedText().isEmpty());
}

void tst_QWebEngineView::hiddenText()
{
    QWebEngineView view;
    view.resize(640, 480);
    view.show();

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='QtWebEngine' size='50'/><br>"
                 "  <input type='password' id='password1'/>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());

    QPoint passwordInputCenter = elementCenter(view.page(), "password1");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, passwordInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("password1"));

    QVERIFY(!view.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QVERIFY(view.focusProxy()->inputMethodHints() & Qt::ImhHiddenText);

    QPoint textInputCenter = elementCenter(view.page(), "input1");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));
    QVERIFY(!(view.focusProxy()->inputMethodHints() & Qt::ImhHiddenText));
}

void tst_QWebEngineView::emptyInputMethodEvent()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='QtWebEngine'/>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    evaluateJavaScriptSync(view.page(), "var inputEle = document.getElementById('input1'); inputEle.focus(); inputEle.select();");
    QTRY_COMPARE(selectionChangedSpy.size(), 1);

    // 1. Empty input method event does not clear text
    QInputMethodEvent emptyEvent;
    QVERIFY(QApplication::sendEvent(view.focusProxy(), &emptyEvent));
    qApp->processEvents();
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QStringLiteral("QtWebEngine"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QStringLiteral("QtWebEngine"));

    // Reset: clear input field
    evaluateJavaScriptSync(view.page(), "var inputEle = document.getElementById('input1').value = ''");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString().isEmpty());
    QTRY_VERIFY(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString().isEmpty());

    // 2. Cancel IME composition with empty input method event
    // Start IME composition
    QList<QInputMethodEvent::Attribute> attributes;
    QInputMethodEvent eventComposition("a", attributes);
    QVERIFY(QApplication::sendEvent(view.focusProxy(), &eventComposition));
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QStringLiteral("a"));
    QTRY_VERIFY(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString().isEmpty());

    // Cancel IME composition
    QVERIFY(QApplication::sendEvent(view.focusProxy(), &emptyEvent));
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString().isEmpty());
    QTRY_VERIFY(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString().isEmpty());

    // Try key press after cancelled IME composition
    QTest::keyClick(view.focusProxy(), Qt::Key_B);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QStringLiteral("b"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QStringLiteral("b"));
}

void tst_QWebEngineView::imeComposition()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='QtWebEngine inputMethod'/>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    evaluateJavaScriptSync(view.page(), "var inputEle = document.getElementById('input1'); inputEle.focus(); inputEle.select();");
    QTRY_COMPARE(selectionChangedSpy.size(), 1);

    // Clear the selection, also cancel the ongoing composition if there is one.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent::Attribute newSelection(QInputMethodEvent::Selection, 0, 0, QVariant());
        attributes.append(newSelection);
        QInputMethodEvent event("", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
        selectionChangedSpy.wait();
        QCOMPARE(selectionChangedSpy.size(), 2);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngine inputMethod"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));

    selectionChangedSpy.clear();


    // 1. Insert a character to the beginning of the line.
    // Send temporary text, which makes the editor has composition 'm'.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("m", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngine inputMethod"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));
    QCOMPARE(selectionChangedSpy.size(), 0);

    // Send temporary text, which makes the editor has composition 'n'.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("n", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngine inputMethod"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));
    QCOMPARE(selectionChangedSpy.size(), 0);

    // Send commit text, which makes the editor conforms composition.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString("o");
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("oQtWebEngine inputMethod"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 1);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 1);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));
    QCOMPARE(selectionChangedSpy.size(), 0);


    // 2. insert a character to the middle of the line.
    // Send temporary text, which makes the editor has composition 'd'.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("d", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("oQtWebEngine inputMethod"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 1);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 1);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));
    QCOMPARE(selectionChangedSpy.size(), 0);

    // Send commit text, which makes the editor conforms composition.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString("e");
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("oeQtWebEngine inputMethod"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 2);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 2);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));
    QCOMPARE(selectionChangedSpy.size(), 0);


    // 3. Insert a character to the end of the line.
    QTest::keyClick(view.focusProxy(), Qt::Key_End);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 25);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 25);

    // Send temporary text, which makes the editor has composition 't'.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("t", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("oeQtWebEngine inputMethod"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 25);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 25);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));
    QCOMPARE(selectionChangedSpy.size(), 0);

    // Send commit text, which makes the editor conforms composition.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString("t");
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("oeQtWebEngine inputMethodt"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 26);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 26);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));
    QCOMPARE(selectionChangedSpy.size(), 0);


    // 4. Replace the selection.
#ifndef Q_OS_MACOS
    QTest::keyClick(view.focusProxy(), Qt::Key_Left, Qt::ShiftModifier | Qt::ControlModifier);
#else
    QTest::keyClick(view.focusProxy(), Qt::Key_Left, Qt::ShiftModifier | Qt::AltModifier);
#endif
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 1);

    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("oeQtWebEngine inputMethodt"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 14);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 26);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString("inputMethodt"));

    // Send temporary text, which makes the editor has composition 'w'.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("w", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
        // The new composition should clear the previous selection
        QVERIFY(selectionChangedSpy.wait());
        QCOMPARE(selectionChangedSpy.size(), 2);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("oeQtWebEngine "));
    // The cursor should be positioned at the end of the composition text
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 15);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 15);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));

    // Send commit text, which makes the editor conforms composition.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString("2");
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    // There is no text selection to be changed at this point thus we can't wait for selectionChanged signal.
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("oeQtWebEngine 2"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 15);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 15);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));
    QCOMPARE(selectionChangedSpy.size(), 2);
    selectionChangedSpy.clear();


    // 5. Mimic behavior of QtVirtualKeyboard with enabled text prediction.
    evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value='QtWebEngine';");
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QString("QtWebEngine"));

    // Move cursor into position.
    QTest::keyClick(view.focusProxy(), Qt::Key_Home);
    for (int j = 0; j < 2; ++j)
        QTest::keyClick(view.focusProxy(), Qt::Key_Right);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 2);

    // Turn text into composition by using negative start position.
    {
        int replaceFrom = -1 * view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt();
        int replaceLength = view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString().size();

        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("QtWebEngine", attributes);
        event.setCommitString(QString(), replaceFrom, replaceLength);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString(""));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QString("QtWebEngine"));

    // Commit.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event(QString(), attributes);
        event.setCommitString("QtWebEngine", 0, 0);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("QtWebEngine"));
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), QString(""));
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QString("QtWebEngine"));
    QCOMPARE(selectionChangedSpy.size(), 0);
}

void tst_QWebEngineView::newlineInTextarea()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.page()->setHtml("<html><body>"
                         "  <textarea rows='5' cols='1' id='input1'></textarea>"
                         "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    evaluateJavaScriptSync(view.page(), "var inputEle = document.getElementById('input1'); inputEle.focus(); inputEle.select();");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString().isEmpty());

    // Enter Key without key text
    QKeyEvent keyPressEnter(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    QKeyEvent keyReleaseEnter(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);
    QApplication::sendEvent(view.focusProxy(), &keyPressEnter);
    QApplication::sendEvent(view.focusProxy(), &keyReleaseEnter);

    QList<QInputMethodEvent::Attribute> attribs;

    QInputMethodEvent eventText(QString(), attribs);
    eventText.setCommitString("\n");
    QApplication::sendEvent(view.focusProxy(), &eventText);

    QInputMethodEvent eventText2(QString(), attribs);
    eventText2.setCommitString("third line");
    QApplication::sendEvent(view.focusProxy(), &eventText2);

    qApp->processEvents();
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QString("\n\nthird line"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("\n\nthird line"));

    // Enter Key with key text '\r'
    evaluateJavaScriptSync(view.page(), "var inputEle = document.getElementById('input1'); inputEle.value = ''; inputEle.focus(); inputEle.select();");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString().isEmpty());

    QKeyEvent keyPressEnterWithCarriageReturn(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier, "\r");
    QKeyEvent keyReleaseEnterWithCarriageReturn(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);
    QApplication::sendEvent(view.focusProxy(), &keyPressEnterWithCarriageReturn);
    QApplication::sendEvent(view.focusProxy(), &keyReleaseEnterWithCarriageReturn);

    QApplication::sendEvent(view.focusProxy(), &eventText);
    QApplication::sendEvent(view.focusProxy(), &eventText2);

    qApp->processEvents();
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QString("\n\nthird line"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("\n\nthird line"));

    // Enter Key with key text '\n'
    evaluateJavaScriptSync(view.page(), "var inputEle = document.getElementById('input1'); inputEle.value = ''; inputEle.focus(); inputEle.select();");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString().isEmpty());

    QKeyEvent keyPressEnterWithLineFeed(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier, "\n");
    QKeyEvent keyReleaseEnterWithLineFeed(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier, "\n");
    QApplication::sendEvent(view.focusProxy(), &keyPressEnterWithLineFeed);
    QApplication::sendEvent(view.focusProxy(), &keyReleaseEnterWithLineFeed);

    QApplication::sendEvent(view.focusProxy(), &eventText);
    QApplication::sendEvent(view.focusProxy(), &eventText2);

    qApp->processEvents();
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QString("\n\nthird line"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("\n\nthird line"));

    // Enter Key with key text "\n\r"
    evaluateJavaScriptSync(view.page(), "var inputEle = document.getElementById('input1'); inputEle.value = ''; inputEle.focus(); inputEle.select();");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString().isEmpty());

    QKeyEvent keyPressEnterWithLFCR(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier, "\n\r");
    QKeyEvent keyReleaseEnterWithLFCR(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier, "\n\r");
    QApplication::sendEvent(view.focusProxy(), &keyPressEnterWithLFCR);
    QApplication::sendEvent(view.focusProxy(), &keyReleaseEnterWithLFCR);

    QApplication::sendEvent(view.focusProxy(), &eventText);
    QApplication::sendEvent(view.focusProxy(), &eventText2);

    qApp->processEvents();
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QString("\n\nthird line"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("\n\nthird line"));

    // Return Key without key text
    evaluateJavaScriptSync(view.page(), "var inputEle = document.getElementById('input1'); inputEle.value = ''; inputEle.focus(); inputEle.select();");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString().isEmpty());

    QKeyEvent keyPressReturn(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    QKeyEvent keyReleaseReturn(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);
    QApplication::sendEvent(view.focusProxy(), &keyPressReturn);
    QApplication::sendEvent(view.focusProxy(), &keyReleaseReturn);

    QApplication::sendEvent(view.focusProxy(), &eventText);
    QApplication::sendEvent(view.focusProxy(), &eventText2);

    qApp->processEvents();
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QString("\n\nthird line"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("\n\nthird line"));
}

void tst_QWebEngineView::imeJSInputEvents()
{
    QWebEngineView view;
    view.resize(640, 480);
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.show();

    auto logLines = [&view]() -> QStringList {
        return evaluateJavaScriptSync(view.page(), "log.textContent").toString().split("\n").filter(QRegularExpression(".+"));
    };

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.page()->setHtml("<html>"
                         "<head><script>"
                         "  var input, log;"
                         "  function verboseEvent(ev) {"
                         "      log.textContent += ev + ' ' + ev.type + ' ' + ev.data + '\\n';"
                         "  }"
                         "  function clear(ev) {"
                         "      log.textContent = '';"
                         "      input.textContent = '';"
                         "  }"
                         "  function init() {"
                         "      input = document.getElementById('input');"
                         "      log = document.getElementById('log');"
                         "      events = [ 'textInput', 'beforeinput', 'input', 'compositionstart', 'compositionupdate', 'compositionend' ];"
                         "      for (var e in events)"
                         "          input.addEventListener(events[e], verboseEvent);"
                         "  }"
                         "</script></head>"
                         "<body onload='init()'>"
                         "  <div id='input' contenteditable='true' style='border-style: solid;'></div>"
                         "  <pre id='log'></pre>"
                         "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    evaluateJavaScriptSync(view.page(), "document.getElementById('input').focus()");
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input"));

    // 1. Commit text (this is how dead keys work on Linux).
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString("commit");
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    // Simply committing text should not trigger any JS composition event.
    QTRY_COMPARE(logLines().size(), 3);
    QCOMPARE(logLines()[0], QStringLiteral("[object InputEvent] beforeinput commit"));
    QCOMPARE(logLines()[1], QStringLiteral("[object TextEvent] textInput commit"));
    QCOMPARE(logLines()[2], QStringLiteral("[object InputEvent] input commit"));

    evaluateJavaScriptSync(view.page(), "clear()");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "log.textContent + input.textContent").toString().isEmpty());

    // 2. Start composition then commit text (this is how dead keys work on macOS).
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("preedit", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(logLines().size(), 4);
    QCOMPARE(logLines()[0], QStringLiteral("[object CompositionEvent] compositionstart "));
    QCOMPARE(logLines()[1], QStringLiteral("[object InputEvent] beforeinput preedit"));
    QCOMPARE(logLines()[2], QStringLiteral("[object CompositionEvent] compositionupdate preedit"));
    QCOMPARE(logLines()[3], QStringLiteral("[object InputEvent] input preedit"));

    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString("commit");
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(logLines().size(), 9);
    QCOMPARE(logLines()[4], QStringLiteral("[object InputEvent] beforeinput commit"));
    QCOMPARE(logLines()[5], QStringLiteral("[object CompositionEvent] compositionupdate commit"));
    QCOMPARE(logLines()[6], QStringLiteral("[object TextEvent] textInput commit"));
    QCOMPARE(logLines()[7], QStringLiteral("[object InputEvent] input commit"));
    QCOMPARE(logLines()[8], QStringLiteral("[object CompositionEvent] compositionend commit"));

    evaluateJavaScriptSync(view.page(), "clear()");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "log.textContent + input.textContent").toString().isEmpty());

    // 3. Start composition then cancel it with an empty IME event.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("preedit", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(logLines().size(), 4);
    QCOMPARE(logLines()[0], QStringLiteral("[object CompositionEvent] compositionstart "));
    QCOMPARE(logLines()[1], QStringLiteral("[object InputEvent] beforeinput preedit"));
    QCOMPARE(logLines()[2], QStringLiteral("[object CompositionEvent] compositionupdate preedit"));
    QCOMPARE(logLines()[3], QStringLiteral("[object InputEvent] input preedit"));

    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(logLines().size(), 9);
    QCOMPARE(logLines()[4], QStringLiteral("[object InputEvent] beforeinput "));
    QCOMPARE(logLines()[5], QStringLiteral("[object CompositionEvent] compositionupdate "));
    QCOMPARE(logLines()[6], QStringLiteral("[object TextEvent] textInput "));
    QCOMPARE(logLines()[7], QStringLiteral("[object InputEvent] input null"));
    QCOMPARE(logLines()[8], QStringLiteral("[object CompositionEvent] compositionend "));

    evaluateJavaScriptSync(view.page(), "clear()");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "log.textContent + input.textContent").toString().isEmpty());

    // 4. Send empty IME event.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    // No JS event is expected.
    QTest::qWait(100);
    QVERIFY(logLines().isEmpty());

    evaluateJavaScriptSync(view.page(), "clear()");
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "log.textContent + input.textContent").toString().isEmpty());
}

void tst_QWebEngineView::imeCompositionQueryEvent_data()
{
    QTest::addColumn<QString>("receiverObjectName");
    QTest::newRow("focusObject") << QString("focusObject");
    QTest::newRow("focusProxy") << QString("focusProxy");
    QTest::newRow("focusWidget") << QString("focusWidget");
}

void tst_QWebEngineView::imeCompositionQueryEvent()
{
    QWebEngineView view;
    view.resize(640, 480);
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);

    view.show();

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' />"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    evaluateJavaScriptSync(view.page(), "document.getElementById('input1').focus()");
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));

    QObject *input = nullptr;

    QFETCH(QString, receiverObjectName);
    if (receiverObjectName == "focusObject") {
        QTRY_VERIFY(qApp->focusObject());
        input = qApp->focusObject();
    } else if (receiverObjectName == "focusProxy") {
        QTRY_VERIFY(view.focusProxy());
        input = view.focusProxy();
    } else if (receiverObjectName == "focusWidget") {
        QTRY_VERIFY(view.focusWidget());
        input = view.focusWidget();
    }

    QInputMethodQueryEvent srrndTextQuery(Qt::ImSurroundingText);
    QInputMethodQueryEvent cursorPosQuery(Qt::ImCursorPosition);
    QInputMethodQueryEvent anchorPosQuery(Qt::ImAnchorPosition);

    // Set composition
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("composition", attributes);
        QApplication::sendEvent(input, &event);
        qApp->processEvents();
    }
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QString("composition"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);

    QApplication::sendEvent(input, &srrndTextQuery);
    QApplication::sendEvent(input, &cursorPosQuery);
    QApplication::sendEvent(input, &anchorPosQuery);
    qApp->processEvents();

    QTRY_COMPARE(srrndTextQuery.value(Qt::ImSurroundingText).toString(), QString(""));
    QTRY_COMPARE(cursorPosQuery.value(Qt::ImCursorPosition).toInt(), 11);
    QTRY_COMPARE(anchorPosQuery.value(Qt::ImAnchorPosition).toInt(), 11);

    // Send commit
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString("composition");
        QApplication::sendEvent(input, &event);
        qApp->processEvents();
    }
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value").toString(), QString("composition"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("composition"));

    QApplication::sendEvent(input, &srrndTextQuery);
    QApplication::sendEvent(input, &cursorPosQuery);
    QApplication::sendEvent(input, &anchorPosQuery);
    qApp->processEvents();

    QTRY_COMPARE(srrndTextQuery.value(Qt::ImSurroundingText).toString(), QString("composition"));
    QTRY_COMPARE(cursorPosQuery.value(Qt::ImCursorPosition).toInt(), 11);
    QTRY_COMPARE(anchorPosQuery.value(Qt::ImAnchorPosition).toInt(), 11);
}

#if QT_CONFIG(clipboard)
void tst_QWebEngineView::globalMouseSelection()
{
    if (!QApplication::clipboard()->supportsSelection()) {
        QSKIP("Test only relevant for systems with selection");
        return;
    }

    QApplication::clipboard()->clear(QClipboard::Selection);
    QWebEngineView view;
    view.resize(640, 480);
    view.show();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='QtWebEngine' size='50' />"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());

    // Select text via JavaScript
    evaluateJavaScriptSync(view.page(), "var inputEle = document.getElementById('input1'); inputEle.focus(); inputEle.select();");
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QVERIFY(QApplication::clipboard()->text(QClipboard::Selection).isEmpty());

    // Deselect the selection (this moves the current cursor to the end of the text)
    QPoint textInputCenter = elementCenter(view.page(), "input1");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 2);
    QVERIFY(QApplication::clipboard()->text(QClipboard::Selection).isEmpty());

    // Select to the start of the line
    QTest::keyClick(view.focusProxy(), Qt::Key_Home, Qt::ShiftModifier);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 3);
    QCOMPARE(QApplication::clipboard()->text(QClipboard::Selection), QStringLiteral("QtWebEngine"));
}
#endif

void tst_QWebEngineView::noContextMenu()
{
    QWidget wrapper;
    wrapper.setContextMenuPolicy(Qt::CustomContextMenu);

    connect(&wrapper, &QWidget::customContextMenuRequested, [&wrapper](const QPoint &pt) {
        QMenu* menu = new QMenu(&wrapper);
        menu->addAction("Action1");
        menu->addAction("Action2");
        menu->popup(pt);
    });

    QWebEngineView view(&wrapper);
    view.setContextMenuPolicy(Qt::NoContextMenu);
    wrapper.show();

    QVERIFY(view.findChildren<QMenu *>().isEmpty());
    QVERIFY(wrapper.findChildren<QMenu *>().isEmpty());
    QTest::mouseMove(wrapper.windowHandle(), QPoint(10,10));
    QTest::mouseClick(wrapper.windowHandle(), Qt::RightButton);

    QTRY_COMPARE(wrapper.findChildren<QMenu *>().size(), 1);
    QVERIFY(view.findChildren<QMenu *>().isEmpty());
}

void tst_QWebEngineView::contextMenu_data()
{
    QTest::addColumn<int>("childrenCount");
    QTest::addColumn<bool>("isCustomMenu");
    QTest::addColumn<Qt::ContextMenuPolicy>("contextMenuPolicy");
    QTest::newRow("defaultContextMenu") << 1 << false << Qt::DefaultContextMenu;
    QTest::newRow("customContextMenu")  << 1 << true  << Qt::CustomContextMenu;
    QTest::newRow("preventContextMenu") << 0 << false << Qt::PreventContextMenu;
}

void tst_QWebEngineView::contextMenu()
{
    QFETCH(int, childrenCount);
    QFETCH(bool, isCustomMenu);
    QFETCH(Qt::ContextMenuPolicy, contextMenuPolicy);

    QWebEngineView view;

    QMenu *customMenu = nullptr;
    if (contextMenuPolicy == Qt::CustomContextMenu) {
        connect(&view, &QWebEngineView::customContextMenuRequested, [&view, &customMenu] (const QPoint &pt) {
            Q_ASSERT(!customMenu);
            customMenu = new QMenu(&view);
            customMenu->addAction("Action1");
            customMenu->addAction("Action2");
            customMenu->popup(pt);
        });
    }

    view.setContextMenuPolicy(contextMenuPolicy);

    // input is supposed to be skipped before first real navigation in >= 79
    QSignalSpy loadSpy(&view, &QWebEngineView::loadFinished);
    view.load(QUrl("about:blank"));
    view.resize(640, 480);
    view.show();
    QTRY_COMPARE(loadSpy.size(), 1);

    QVERIFY(view.findChildren<QMenu *>().isEmpty());
    QTest::mouseMove(view.windowHandle(), QPoint(10,10));
    QTest::mouseClick(view.windowHandle(), Qt::RightButton);

    // verify for zero children will always succeed, so should be tested with at least minor timeout
    if (childrenCount <= 0) {
        QVERIFY(!QTest::qWaitFor([&view] () { return view.findChildren<QMenu *>().size() > 0; }, 500));
    } else {
        QTRY_COMPARE(view.findChildren<QMenu *>().size(), childrenCount);
        if (isCustomMenu) {
            QCOMPARE(view.findChildren<QMenu *>().first(), customMenu);
        }
    }
    QCOMPARE(!!customMenu, isCustomMenu);
}

void tst_QWebEngineView::mouseLeave()
{
    QScopedPointer<QWidget> containerWidget(new QWidget);

    QLabel *label = new QLabel(containerWidget.data());
    label->setStyleSheet("background-color: red;");
    label->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    label->setMinimumHeight(100);

    QWebEngineView *view = new QWebEngineView(containerWidget.data());
    view->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    view->setMinimumHeight(100);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignTop);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(label);
    layout->addWidget(view);
    containerWidget->setLayout(layout);
    containerWidget->show();
    QVERIFY(QTest::qWaitForWindowExposed(containerWidget.data()));
    QTest::mouseMove(containerWidget->windowHandle(), QPoint(1, 1));

    auto innerText = [view]() -> QString {
        return evaluateJavaScriptSync(view->page(), "document.getElementById('testDiv').innerText").toString();
    };

    QSignalSpy loadFinishedSpy(view, SIGNAL(loadFinished(bool)));
    view->setHtml("<html>"
                  "<head><script>"
                  "function init() {"
                  " var div = document.getElementById('testDiv');"
                  " div.onmouseenter = function(e) { div.innerText = 'Mouse IN' };"
                  " div.onmouseleave = function(e) { div.innerText = 'Mouse OUT' };"
                  "}"
                  "</script></head>"
                  "<body onload='init()' style='margin: 0px; padding: 0px'>"
                  " <div id='testDiv' style='width: 100%; height: 100%; background-color: green' />"
                  "</body>"
                  "</html>");
    QVERIFY(loadFinishedSpy.wait());
    // Make sure the testDiv text is empty.
    evaluateJavaScriptSync(view->page(), "document.getElementById('testDiv').innerText = ''");
    QTRY_VERIFY(innerText().isEmpty());

    QTest::mouseMove(containerWidget->windowHandle(), QPoint(50, 150));
    QTRY_COMPARE(innerText(), QStringLiteral("Mouse IN"));
    QTest::mouseMove(containerWidget->windowHandle(), QPoint(50, 50));
    QTRY_COMPARE(innerText(), QStringLiteral("Mouse OUT"));
}

void tst_QWebEngineView::webUIURLs_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<bool>("supported");
    QTest::newRow("about") << QUrl("chrome://about") << false;
    QTest::newRow("accessibility") << QUrl("chrome://accessibility") << true;
    QTest::newRow("app-service-internals") << QUrl("chrome://app-service-internals") << false;
    QTest::newRow("app-settings") << QUrl("chrome://app-settings") << false;
    QTest::newRow("apps") << QUrl("chrome://apps") << false;
    QTest::newRow("attribution-internals") << QUrl("chrome://attribution-internals") << true;
    QTest::newRow("autofill-internals") << QUrl("chrome://autofill-internals") << false;
    QTest::newRow("blob-internals") << QUrl("chrome://blob-internals") << true;
    QTest::newRow("bluetooth-internals") << QUrl("chrome://bluetooth-internals") << false;
    QTest::newRow("bookmarks") << QUrl("chrome://bookmarks") << false;
    QTest::newRow("chrome-urls") << QUrl("chrome://chrome-urls") << false;
    QTest::newRow("components") << QUrl("chrome://components") << false;
    QTest::newRow("connectors-internals") << QUrl("chrome://connectors-internals") << false;
    QTest::newRow("crashes") << QUrl("chrome://crashes") << false;
    QTest::newRow("credits") << QUrl("chrome://credits") << false;
    QTest::newRow("device-log") << QUrl("chrome://device-log") << true;
    QTest::newRow("dino") << QUrl("chrome://dino") << false; // It works but this is an error page
    QTest::newRow("discards") << QUrl("chrome://discards") << false;
    QTest::newRow("download-internals") << QUrl("chrome://download-internals") << false;
    QTest::newRow("downloads") << QUrl("chrome://downloads") << false;
    QTest::newRow("extensions") << QUrl("chrome://extensions") << false;
    QTest::newRow("extensions-internals") << QUrl("chrome://extensions-internals") << false;
    QTest::newRow("flags") << QUrl("chrome://flags") << false;
    QTest::newRow("gcm-internals") << QUrl("chrome://gcm-internals") << false;
    QTest::newRow("gpu") << QUrl("chrome://gpu") << true;
    QTest::newRow("help") << QUrl("chrome://help") << false;
    QTest::newRow("histograms") << QUrl("chrome://histograms") << true;
    QTest::newRow("history") << QUrl("chrome://history") << false;
    QTest::newRow("history-clusters-internals") << QUrl("chrome://history-clusters-internals") << false;
    QTest::newRow("indexeddb-internals") << QUrl("chrome://indexeddb-internals") << true;
    QTest::newRow("inspect") << QUrl("chrome://inspect") << false;
    QTest::newRow("interstitials") << QUrl("chrome://interstitials") << false;
    QTest::newRow("invalidations") << QUrl("chrome://invalidations") << false;
    QTest::newRow("linux-proxy-config") << QUrl("chrome://linux-proxy-config") << false;
    QTest::newRow("local-state") << QUrl("chrome://local-state") << false;
    QTest::newRow("management") << QUrl("chrome://management") << false;
    QTest::newRow("media-engagement") << QUrl("chrome://media-engagement") << false;
    QTest::newRow("media-internals") << QUrl("chrome://media-internals") << true;
    QTest::newRow("nacl") << QUrl("chrome://nacl") << false;
    QTest::newRow("net-export") << QUrl("chrome://net-export") << false;
    QTest::newRow("net-internals") << QUrl("chrome://net-internals") << true;
    QTest::newRow("network-error") << QUrl("chrome://network-error") << false;
    QTest::newRow("network-errors") << QUrl("chrome://network-errors") << true;
    QTest::newRow("ntp-tiles-internals") << QUrl("chrome://ntp-tiles-internals") << false;
    QTest::newRow("omnibox") << QUrl("chrome://omnibox") << false;
    QTest::newRow("optimization-guide-internals") << QUrl("chrome://optimization-guide-internals") << false;
    QTest::newRow("password-manager-internals") << QUrl("chrome://password-manager-internals") << false;
    QTest::newRow("policy") << QUrl("chrome://policy") << false;
    QTest::newRow("predictors") << QUrl("chrome://predictors") << false;
    QTest::newRow("prefs-internals") << QUrl("chrome://prefs-internals") << false;
    QTest::newRow("print") << QUrl("chrome://print") << false;
    QTest::newRow("process-internals") << QUrl("chrome://process-internals") << true;
    QTest::newRow("quota-internals") << QUrl("chrome://quota-internals") << true;
    QTest::newRow("safe-browsing") << QUrl("chrome://safe-browsing") << false;
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    QTest::newRow("sandbox") << QUrl("chrome://sandbox") << true;
#else
    QTest::newRow("sandbox") << QUrl("chrome://sandbox") << false;
#endif
    QTest::newRow("serviceworker-internals") << QUrl("chrome://serviceworker-internals") << true;
    QTest::newRow("settings") << QUrl("chrome://settings") << false;
    QTest::newRow("signin-internals") << QUrl("chrome://signin-internals") << false;
    QTest::newRow("site-engagement") << QUrl("chrome://site-engagement") << false;
    QTest::newRow("sync-internals") << QUrl("chrome://sync-internals") << false;
    QTest::newRow("system") << QUrl("chrome://system") << false;
    QTest::newRow("terms") << QUrl("chrome://terms") << false;
    QTest::newRow("tracing") << QUrl("chrome://tracing") << true;
    QTest::newRow("translate-internals") << QUrl("chrome://translate-internals") << false;
    QTest::newRow("ukm") << QUrl("chrome://ukm") << true;
    QTest::newRow("usb-internals") << QUrl("chrome://usb-internals") << false;
    QTest::newRow("user-actions") << QUrl("chrome://user-actions") << true;
    QTest::newRow("version") << QUrl("chrome://version") << false;
    QTest::newRow("web-app-internals") << QUrl("chrome://web-app-internals") << false;
#if QT_CONFIG(webengine_webrtc)
    QTest::newRow("webrtc-internals") << QUrl("chrome://webrtc-internals") << true;
#if QT_CONFIG(webengine_extensions)
    QTest::newRow("webrtc-logs") << QUrl("chrome://webrtc-logs") << true;
#endif // QT_CONFIG(webengine_extensions)
#endif // QT_CONFIG(webengine_webrtc)
    QTest::newRow("whats-new") << QUrl("chrome://whats-new") << false;
}

void tst_QWebEngineView::webUIURLs()
{
    QFETCH(QUrl, url);
    QFETCH(bool, supported);

    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.load(url);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 90000);
    QCOMPARE(loadFinishedSpy.takeFirst().at(0).toBool(), supported);
}

void tst_QWebEngineView::visibilityState()
{
    QWebEngineView view;
    QSignalSpy spy(&view, &QWebEngineView::loadFinished);
    view.load(QStringLiteral("about:blank"));
    QVERIFY(spy.size() || spy.wait());
    QVERIFY(spy.takeFirst().takeFirst().toBool());
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.visibilityState").toString(), QStringLiteral("hidden"));
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.visibilityState").toString(), QStringLiteral("visible"));
}

void tst_QWebEngineView::visibilityState2()
{
    QWebEngineView view;
    QSignalSpy spy(&view, &QWebEngineView::loadFinished);
    view.show();
    view.load(QStringLiteral("about:blank"));
    view.hide();
    QVERIFY(spy.size() || spy.wait());
    QVERIFY(spy.takeFirst().takeFirst().toBool());
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.visibilityState").toString(), QStringLiteral("hidden"));
}

void tst_QWebEngineView::visibilityState3()
{
    QWebEnginePage page1;
    QWebEnginePage page2;
    QSignalSpy spy1(&page1, &QWebEnginePage::loadFinished);
    QSignalSpy spy2(&page2, &QWebEnginePage::loadFinished);
    page1.load(QStringLiteral("about:blank"));
    page2.load(QStringLiteral("about:blank"));
    QVERIFY(spy1.size() || spy1.wait());
    QVERIFY(spy2.size() || spy2.wait());
    QWebEngineView view;
    view.setPage(&page1);
    view.show();
    QCOMPARE(evaluateJavaScriptSync(&page1, "document.visibilityState").toString(), QStringLiteral("visible"));
    QCOMPARE(evaluateJavaScriptSync(&page2, "document.visibilityState").toString(), QStringLiteral("hidden"));
    view.setPage(&page2);
    QCOMPARE(evaluateJavaScriptSync(&page1, "document.visibilityState").toString(), QStringLiteral("hidden"));
    QCOMPARE(evaluateJavaScriptSync(&page2, "document.visibilityState").toString(), QStringLiteral("visible"));
}

void tst_QWebEngineView::jsKeyboardEvent_data()
{
    QTest::addColumn<char>("key");
    QTest::addColumn<Qt::KeyboardModifiers>("modifiers");
    QTest::addColumn<QString>("expected");

#if defined(Q_OS_MACOS)
    // See Qt::AA_MacDontSwapCtrlAndMeta
    Qt::KeyboardModifiers controlModifier = Qt::MetaModifier;
#else
    Qt::KeyboardModifiers controlModifier = Qt::ControlModifier;
#endif

    QTest::newRow("Ctrl+Shift+A") << 'A' << (controlModifier | Qt::ShiftModifier) << QStringLiteral(
                                         "16,ShiftLeft,Shift,false,true,false;"
                                         "17,ControlLeft,Control,true,true,false;"
                                         "65,KeyA,A,true,true,false;");
    QTest::newRow("Ctrl+z") << 'z' << controlModifier << QStringLiteral(
                                   "17,ControlLeft,Control,true,false,false;"
                                   "90,KeyZ,z,true,false,false;");
}

void tst_QWebEngineView::jsKeyboardEvent()
{
    QWebEngineView view;
    evaluateJavaScriptSync(
        view.page(),
        "var log = '';"
        "addEventListener('keydown', (ev) => {"
        "  log += [ev.keyCode, ev.code, ev.key, ev.ctrlKey, ev.shiftKey, ev.altKey].join(',') + ';';"
        "});");

    QFETCH(char, key);
    QFETCH(Qt::KeyboardModifiers, modifiers);
    QFETCH(QString, expected);

    // Note that this only tests the fallback code path where native scan codes are not used.
    QTest::keyClick(view.focusProxy(), key, modifiers);
    QTRY_VERIFY(evaluateJavaScriptSync(view.page(), "log") != QVariant(QString()));
    QCOMPARE(evaluateJavaScriptSync(view.page(), "log"), expected);
}

void tst_QWebEngineView::deletePage()
{
    QWebEngineView view;
    QWebEnginePage *page = view.page();
    QVERIFY(page);
    QCOMPARE(page->parent(), &view);
    delete page;
    // Test that a new page is created and that it is useful:
    QVERIFY(view.page());
    QSignalSpy spy(view.page(), &QWebEnginePage::loadFinished);
    view.page()->load(QStringLiteral("about:blank"));
    QTRY_VERIFY(spy.size());
}

void tst_QWebEngineView::autoDeleteOnExternalPageDelete()
{
    QPointer<QWebEngineView> view = new QWebEngineView;
    QPointer<QWebEnginePage> page = new QWebEnginePage;
    auto sg = qScopeGuard([&] () { delete view; delete page; });

    QSignalSpy spy(page, &QWebEnginePage::loadFinished);
    view->setPage(page);
    view->show();
    view->resize(320, 240);
    page->load(QUrl("about:blank"));
    QTRY_VERIFY(spy.size());
    QVERIFY(page->parent() != view);

    auto sc = QObject::connect(page, &QWebEnginePage::destroyed, view, &QWebEngineView::deleteLater);
    QTimer::singleShot(0, page, &QObject::deleteLater);
    QTRY_VERIFY(!page);
    QTRY_VERIFY(!view);
}

class TestView : public QWebEngineView {
    Q_OBJECT
public:
    TestView(QWidget *parent = nullptr) : QWebEngineView(parent)
    {
    }

    QWebEngineView *createWindow(QWebEnginePage::WebWindowType) override
    {
        TestView *view = new TestView(parentWidget());
        createdWindows.append(view);
        return view;
    }
    QList<TestView *> createdWindows;
};

void tst_QWebEngineView::closeOpenerTab()
{
    QWidget rootWidget;
    rootWidget.resize(600, 400);
    auto *testView = new TestView(&rootWidget);
    testView->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    QSignalSpy loadFinishedSpy(testView, SIGNAL(loadFinished(bool)));
    testView->setUrl(QStringLiteral("about:blank"));
    QTRY_VERIFY(loadFinishedSpy.size());
    testView->page()->runJavaScript(QStringLiteral("window.open('about:blank','_blank')"));
    QTRY_COMPARE(testView->createdWindows.size(), 1);
    auto *newView = testView->createdWindows.at(0);
    newView->show();
    rootWidget.show();
    QVERIFY(QTest::qWaitForWindowExposed(newView));
    QVERIFY(newView->focusProxy()->isVisible());
    delete testView;
    QVERIFY(newView->focusProxy()->isVisible());
}

void tst_QWebEngineView::switchPage()
{
      QWebEngineProfile profile;
      QWebEnginePage page1(&profile);
      QWebEnginePage page2(&profile);
      QSignalSpy loadFinishedSpy1(&page1, SIGNAL(loadFinished(bool)));
      QSignalSpy loadFinishedSpy2(&page2, SIGNAL(loadFinished(bool)));
      // TODO fixme: page without the view has no real widget behind, so
      // reading graphical content will fail, add view for now.
      QWebEngineView webView1(&page1, nullptr);
      QWebEngineView webView2(&page2, nullptr);
      page1.setHtml("<html><body bgcolor=\"#000000\"></body></html>");
      page2.setHtml("<html><body bgcolor=\"#ffffff\"></body></html>");
      QTRY_VERIFY(loadFinishedSpy1.size() && loadFinishedSpy2.size());
      QWebEngineView webView;
      webView.resize(300,300);
      webView.show();
      webView.setPage(&page1);
      QTRY_COMPARE(webView.grab().toImage().pixelColor(QPoint(150,150)), Qt::black);
      webView.setPage(&page2);
      QTRY_COMPARE(webView.grab().toImage().pixelColor(QPoint(150,150)), Qt::white);
      webView.setPage(&page1);
      QTRY_COMPARE(webView.grab().toImage().pixelColor(QPoint(150,150)), Qt::black);
}

void tst_QWebEngineView::setPageDeletesImplicitPage()
{
    QWebEngineView view;
    QPointer<QWebEnginePage> implicitPage = view.page();
    QWebEnginePage explicitPage;
    view.setPage(&explicitPage);
    QCOMPARE(view.page(), &explicitPage);
    QVERIFY(!implicitPage); // should be deleted
}

void tst_QWebEngineView::setPageDeletesImplicitPage2()
{
    QWebEngineView view1;
    QWebEngineView view2;
    QPointer<QWebEnginePage> implicitPage = view1.page();
    view2.setPage(view1.page());
    QVERIFY(implicitPage);
    QVERIFY(view1.page() != implicitPage);
    QWebEnginePage explicitPage;
    view2.setPage(&explicitPage);
    QCOMPARE(view2.page(), &explicitPage);
    QVERIFY(!implicitPage); // should be deleted
}

void tst_QWebEngineView::setViewDeletesImplicitPage()
{
    QWebEngineView view;
    QPointer<QWebEnginePage> implicitPage = view.page();
    QWebEnginePage explicitPage;
    view.setPage(&explicitPage);
    QCOMPARE(view.page(), &explicitPage);
    QVERIFY(!implicitPage); // should be deleted
}

void tst_QWebEngineView::setPagePreservesExplicitPage()
{
    QWebEngineView view;
    QPointer<QWebEnginePage> explicitPage1 = new QWebEnginePage(&view);
    QPointer<QWebEnginePage> explicitPage2 = new QWebEnginePage(&view);
    view.setPage(explicitPage1.data());
    view.setPage(explicitPage2.data());
    QCOMPARE(view.page(), explicitPage2.data());
    QVERIFY(explicitPage1); // should not be deleted
}

void tst_QWebEngineView::setViewPreservesExplicitPage()
{
    QWebEngineView view;
    QPointer<QWebEnginePage> explicitPage1 = new QWebEnginePage(&view);
    QPointer<QWebEnginePage> explicitPage2 = new QWebEnginePage(&view);
    view.setPage(explicitPage1.data());
    view.setPage(explicitPage2.data());
    QCOMPARE(view.page(), explicitPage2.data());
    QVERIFY(explicitPage1); // should not be deleted
}

void tst_QWebEngineView::closeDiscardsPage()
{
    QWebEngineProfile profile;
    QWebEngineView view(&profile, nullptr);
    view.resize(300, 300);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QCOMPARE(view.page()->isVisible(), true);
    QCOMPARE(view.page()->lifecycleState(), QWebEnginePage::LifecycleState::Active);
    view.close();
    QCOMPARE(view.page()->isVisible(), false);
    QCOMPARE(view.page()->lifecycleState(), QWebEnginePage::LifecycleState::Discarded);
}


void tst_QWebEngineView::loadAfterRendererCrashed()
{
    QWebEngineView view;
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    bool terminated = false;
    connect(view.page(), &QWebEnginePage::renderProcessTerminated, [&] () { terminated = true; });
    view.load(QUrl("chrome://crash"));
    QTRY_VERIFY_WITH_TIMEOUT(terminated, 30000);

    QSignalSpy loadSpy(&view, &QWebEngineView::loadFinished);
    view.load(QUrl("qrc:///resources/dummy.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QVERIFY(loadSpy.first().first().toBool());
}

void tst_QWebEngineView::inspectElement()
{
    QWebEngineView view;
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    auto page = view.page();
    // shouldn't do anything until page is set
    page->triggerAction(QWebEnginePage::InspectElement);
    QTest::qWait(100);

    QSignalSpy spy(&view, &QWebEngineView::loadFinished);
    view.load(QUrl("data:text/plain,foobarbaz"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 12000);

    // shouldn't do anything since inspector is not attached
    page->triggerAction(QWebEnginePage::InspectElement);
    QTest::qWait(100);

    QWebEngineView inspectorView;
    inspectorView.resize(640, 480);
    inspectorView.show();
    QVERIFY(QTest::qWaitForWindowExposed(&inspectorView));
    inspectorView.page()->setInspectedPage(page);

    page->triggerAction(QWebEnginePage::InspectElement);
    // TODO verify somehow
    QTest::qWait(100);
}

void tst_QWebEngineView::navigateOnDrop_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<bool>("navigateOnDrop");
    QTest::newRow("file") << QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).absoluteFilePath("resources/dummy.html")) << true;
    QTest::newRow("qrc") << QUrl("qrc:///resources/dummy.html") << true;
    QTest::newRow("file_no_navigate") << QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).absoluteFilePath("resources/dummy.html")) << false;
    QTest::newRow("qrc_no_navigate") << QUrl("qrc:///resources/dummy.html") << false;
}

void tst_QWebEngineView::navigateOnDrop()
{
    QFETCH(QUrl, url);
    QFETCH(bool, navigateOnDrop);
    struct WebEngineView : QWebEngineView {
        QWebEngineView* createWindow(QWebEnginePage::WebWindowType /* type */) override { return this; }
    } view;
    view.page()->settings()->setAttribute(QWebEngineSettings::NavigateOnDropEnabled, navigateOnDrop);
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadSpy(&view, &QWebEngineView::loadFinished);
    QMimeData mimeData;
    mimeData.setUrls({ url });

    auto sendEvents = [&] () {
        QDragEnterEvent dee(view.rect().center(), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&view, &dee);
        QDropEvent de(view.rect().center(), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&view, &de);
    };

    sendEvents();
    if (navigateOnDrop) {
        QTRY_COMPARE(loadSpy.size(), 1);
        QVERIFY(loadSpy.last().first().toBool());
        QCOMPARE(view.url(), url);
    } else {
        QTest::qWait(500);
        QCOMPARE(loadSpy.size(), 0);
        QVERIFY(view.url() != url);
    }

    // Check dynamically changing the setting
    loadSpy.clear();
    view.page()->settings()->setAttribute(QWebEngineSettings::NavigateOnDropEnabled, !navigateOnDrop);
    view.setUrl(QUrl("about:blank"));
    QTRY_COMPARE(loadSpy.size(), 1);

    sendEvents();
    if (!navigateOnDrop) {
        QTRY_COMPARE(loadSpy.size(), 2);
        QVERIFY(loadSpy.last().first().toBool());
        QCOMPARE(view.url(), url);
    } else {
        QTest::qWait(500);
        QCOMPARE(loadSpy.size(), 1);
        QVERIFY(view.url() != url);
    }
}

void tst_QWebEngineView::datalist()
{
    QString html("<html><body>"
                 "<input id='browserInput' list='browserDatalist'>"
                 "<datalist id='browserDatalist'>"
                 "  <option value='Internet Explorer'>"
                 "  <option value='Firefox'>"
                 "  <option value='Chrome'>"
                 "  <option value='Opera'>"
                 "  <option value='Safari'>"
                 "</datalist>"
                 "</body></html>");

    QWebEngineView view;
    view.resize(200, 400);
    view.show();

    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadSpy(&view, &QWebEngineView::loadFinished);
    view.setHtml(html);
    QTRY_COMPARE(loadSpy.size(), 1);

    QString listValuesJS("(function() {"
                         "  var browserDatalist = document.getElementById('browserDatalist');"
                         "  var options = browserDatalist.options;"
                         "  var result = [];"
                         "  for (let i = 0; i < options.length; ++i) {"
                         "    result.push(options[i].value);"
                         "  }"
                         "  return result;"
                         "})();");
    QStringList values = evaluateJavaScriptSync(view.page(), listValuesJS).toStringList();
    QCOMPARE(values, QStringList({ "Internet Explorer", "Firefox", "Chrome", "Opera", "Safari" }));
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('browserInput').value;")
                     .toString(),
             QStringLiteral(""));

    auto listView = [&view]() -> QListView * {
        if (QApplication::topLevelWidgets().size() == 1) {
            // No popup case.
            return nullptr;
        }

        QWidget *autofillPopupWidget = nullptr;
        for (QWidget *w : QApplication::topLevelWidgets()) {
            if (w != &view) {
                autofillPopupWidget = w;
                break;
            }
        }

        if (!autofillPopupWidget)
            return nullptr;

        for (QObject *o : autofillPopupWidget->children()) {
            if (QListView *listView = qobject_cast<QListView *>(o))
                return listView;
        }

        return nullptr;
    };

    // Make sure there is no open popup yet.
    QVERIFY(!listView());
    // Click in the input field.
    QPoint browserInputCenter = elementCenter(view.page(), "browserInput");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, browserInputCenter);
    // Wait for the popup.
    QTRY_VERIFY(listView());

    // No suggestion is selected.
    QCOMPARE(listView()->currentIndex(), QModelIndex());
    QCOMPARE(listView()->model()->rowCount(), 5);

    // Accepting suggestion does nothing.
    QTest::keyClick(view.windowHandle(), Qt::Key_Enter);
    QVERIFY(listView());
    QCOMPARE(listView()->currentIndex(), QModelIndex());

    // Escape should close popup.
    QTest::keyClick(view.windowHandle(), Qt::Key_Escape);
    QTRY_VERIFY(!listView());

    // Key Down should open the popup and select the first suggestion.
    QTest::keyClick(view.windowHandle(), Qt::Key_Down);
    QTRY_VERIFY(listView());
    QCOMPARE(listView()->currentIndex().row(), 0);

    // Test keyboard navigation in list.
    QTest::keyClick(view.windowHandle(), Qt::Key_Up);
    QCOMPARE(listView()->currentIndex().row(), 4);
    QTest::keyClick(view.windowHandle(), Qt::Key_Up);
    QCOMPARE(listView()->currentIndex().row(), 3);
    QTest::keyClick(view.windowHandle(), Qt::Key_PageDown);
    QCOMPARE(listView()->currentIndex().row(), 4);
    QTest::keyClick(view.windowHandle(), Qt::Key_PageUp);
    QCOMPARE(listView()->currentIndex().row(), 0);
    QTest::keyClick(view.windowHandle(), Qt::Key_Down);
    QCOMPARE(listView()->currentIndex().row(), 1);
    QTest::keyClick(view.windowHandle(), Qt::Key_Down);
    QCOMPARE(listView()->currentIndex().row(), 2);

    // Test accepting suggestion.
    QCOMPARE(static_cast<QStringListModel *>(listView()->model())
                     ->data(listView()->currentIndex())
                     .toString(),
             QStringLiteral("Chrome"));
    QTest::keyClick(view.windowHandle(), Qt::Key_Enter);
    QTRY_COMPARE(
            evaluateJavaScriptSync(view.page(), "document.getElementById('browserInput').value")
                    .toString(),
            QStringLiteral("Chrome"));
    // Accept closes popup.
    QTRY_VERIFY(!listView());

    // Clear input field, should not trigger popup.
    evaluateJavaScriptSync(view.page(), "document.getElementById('browserInput').value = ''");
    QVERIFY(!listView());

    // Filter suggestions.
    QTest::keyClick(view.windowHandle(), Qt::Key_F);
    QTRY_VERIFY(listView());
    QCOMPARE(listView()->model()->rowCount(), 2);
    QCOMPARE(listView()->currentIndex(), QModelIndex());
    QCOMPARE(static_cast<QStringListModel *>(listView()->model())
                     ->data(listView()->model()->index(0, 0))
                     .toString(),
             QStringLiteral("Firefox"));
    QCOMPARE(static_cast<QStringListModel *>(listView()->model())
                     ->data(listView()->model()->index(1, 0))
                     .toString(),
             QStringLiteral("Safari"));
    QTest::keyClick(view.windowHandle(), Qt::Key_I);
    QTRY_COMPARE(listView()->model()->rowCount(), 1);
    QCOMPARE(listView()->currentIndex(), QModelIndex());
    QCOMPARE(static_cast<QStringListModel *>(listView()->model())
                     ->data(listView()->model()->index(0, 0))
                     .toString(),
             QStringLiteral("Firefox"));
    QTest::keyClick(view.windowHandle(), Qt::Key_L);
    // Mismatch should close popup.
    QTRY_VERIFY(!listView());
    QTRY_COMPARE(
            evaluateJavaScriptSync(view.page(), "document.getElementById('browserInput').value")
                    .toString(),
            QStringLiteral("fil"));
}

QTEST_MAIN(tst_QWebEngineView)
#include "tst_qwebengineview.moc"
