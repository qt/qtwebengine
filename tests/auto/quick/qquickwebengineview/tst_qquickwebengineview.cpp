// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testwindow.h"
#include "quickutil.h"

#include <QScopedPointer>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qregularexpression.h>
#include <QtGui/qclipboard.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtQml/QQmlEngine>
#include <QtTest/QtTest>
#include <QtWebEngineQuick/QQuickWebEngineProfile>
#include <QtGui/private/qinputmethod_p.h>
#include <QtWebEngineQuick/private/qquickwebenginescriptcollection_p.h>
#include <QtWebEngineQuick/private/qquickwebenginesettings_p.h>
#include <QtWebEngineQuick/private/qquickwebengineview_p.h>
#include <QtWebEngineCore/private/qtwebenginecore-config_p.h>
#include <qpa/qplatforminputcontext.h>
#include <QtTest/private/qemulationdetector_p.h>

#include <functional>

class tst_QQuickWebEngineView : public QObject {
    Q_OBJECT
public:
    tst_QQuickWebEngineView();

private Q_SLOTS:
    void init();
    void cleanup();

    void navigationStatusAtStartup();
    void stopEnabledAfterLoadStarted();
    void baseUrl();
    void loadEmptyUrl();
    void loadEmptyPageViewVisible();
    void loadEmptyPageViewHidden();
    void loadNonexistentFileUrl();
    void backAndForward();
    void reload();
    void stop();
    void loadProgress();

    void show();
    void showWebEngineView();
    void removeFromCanvas();
    void multipleWebEngineViewWindows();
    void multipleWebEngineViews();
    void titleUpdate();
    void transparentWebEngineViews();

    void inputMethod();
    void inputMethodHints();
    void inputContextQueryInput();
    void interruptImeTextComposition_data();
    void interruptImeTextComposition();
    void basicRenderingSanity();
    void setZoomFactor();
    void printToPdf();
    void stopSettingFocusWhenDisabled();
    void stopSettingFocusWhenDisabled_data();
    void inputEventForwardingDisabledWhenActiveFocusOnPressDisabled();

    void changeLocale();
    void userScripts();
    void javascriptClipboard_data();
    void javascriptClipboard();
    void setProfile();
#if QT_CONFIG(accessibility)
    void focusChild_data();
    void focusChild();
#endif
    void htmlSelectPopup();

private:
    inline QQuickWebEngineView *newWebEngineView();
    inline QQuickWebEngineView *webEngineView() const;
    QUrl urlFromTestPath(const char *localFilePath);
    void runJavaScript(const QString &script);
    QString m_testSourceDirPath;
    QScopedPointer<TestWindow> m_window;
    QScopedPointer<QQmlComponent> m_component;

     QPointingDevice *touchDevice() {
         static auto d = QScopedPointer<QPointingDevice>(QTest::createTouchDevice());
         return d.get();
     }
};

tst_QQuickWebEngineView::tst_QQuickWebEngineView()
{
    QtWebEngineQuick::initialize();
    QQuickWebEngineProfile::defaultProfile()->setOffTheRecord(true);

    m_testSourceDirPath = QDir(QT_TESTCASE_SOURCEDIR).canonicalPath();
    if (!m_testSourceDirPath.endsWith(QLatin1Char('/')))
        m_testSourceDirPath.append(QLatin1Char('/'));

    static QQmlEngine *engine = new QQmlEngine(this);
    m_component.reset(new QQmlComponent(engine, this));
    m_component->setData(QByteArrayLiteral("import QtQuick\n"
                                           "import QtWebEngine\n"
                                           "WebEngineView {}")
                         , QUrl());
}

QQuickWebEngineView *tst_QQuickWebEngineView::newWebEngineView()
{
    QObject *viewInstance = m_component->create();
    QQuickWebEngineView *webEngineView = qobject_cast<QQuickWebEngineView*>(viewInstance);
    return webEngineView;
}

void tst_QQuickWebEngineView::init()
{
    m_window.reset(new TestWindow(newWebEngineView()));
}

void tst_QQuickWebEngineView::cleanup()
{
    m_window.reset();
}

inline QQuickWebEngineView *tst_QQuickWebEngineView::webEngineView() const
{
    return static_cast<QQuickWebEngineView*>(m_window->webEngineView.data());
}

QUrl tst_QQuickWebEngineView::urlFromTestPath(const char *localFilePath)
{
    return QUrl::fromLocalFile(m_testSourceDirPath + QString::fromUtf8(localFilePath));
}

void tst_QQuickWebEngineView::runJavaScript(const QString &script)
{
    webEngineView()->runJavaScript(script);
}

void tst_QQuickWebEngineView::navigationStatusAtStartup()
{
    QCOMPARE(webEngineView()->canGoBack(), false);

    QCOMPARE(webEngineView()->canGoForward(), false);

    QCOMPARE(webEngineView()->isLoading(), false);
}

void tst_QQuickWebEngineView::stopEnabledAfterLoadStarted()
{
    QCOMPARE(webEngineView()->isLoading(), false);

    LoadStartedCatcher catcher(webEngineView());
    webEngineView()->setUrl(urlFromTestPath("html/basic_page.html"));
    QSignalSpy spy(&catcher, &LoadStartedCatcher::finished);
    QVERIFY(spy.wait());

    QCOMPARE(webEngineView()->isLoading(), true);

    QVERIFY(waitForLoadSucceeded(webEngineView()));
}

void tst_QQuickWebEngineView::baseUrl()
{
    // Test the url is in a well defined state when instanciating the view, but before loading anything.
    QVERIFY(webEngineView()->url().isEmpty());
}

void tst_QQuickWebEngineView::loadEmptyUrl()
{
    webEngineView()->setUrl(QUrl());
    webEngineView()->setUrl(QUrl(QLatin1String("")));
}

void tst_QQuickWebEngineView::loadEmptyPageViewVisible()
{
    m_window->show();
    loadEmptyPageViewHidden();
}

void tst_QQuickWebEngineView::loadEmptyPageViewHidden()
{
    QSignalSpy loadSpy(webEngineView(), SIGNAL(loadingChanged(QWebEngineLoadingInfo)));

    webEngineView()->setUrl(urlFromTestPath("html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView()));

    QCOMPARE(loadSpy.size(), 2);
}

void tst_QQuickWebEngineView::loadNonexistentFileUrl()
{
    QSignalSpy loadSpy(webEngineView(), SIGNAL(loadingChanged(QWebEngineLoadingInfo)));

    webEngineView()->setUrl(urlFromTestPath("html/file_that_does_not_exist.html"));
    QVERIFY(waitForLoadFailed(webEngineView()));

    QCOMPARE(loadSpy.size(), 2);
}

void tst_QQuickWebEngineView::backAndForward()
{
    webEngineView()->setUrl(urlFromTestPath("html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView()));

    QCOMPARE(webEngineView()->url(), urlFromTestPath("html/basic_page.html"));

    webEngineView()->setUrl(urlFromTestPath("html/basic_page2.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView()));

    QCOMPARE(webEngineView()->url(), urlFromTestPath("html/basic_page2.html"));

    webEngineView()->goBack();
    QVERIFY(waitForLoadSucceeded(webEngineView()));

    QCOMPARE(webEngineView()->url(), urlFromTestPath("html/basic_page.html"));

    webEngineView()->goForward();
    QVERIFY(waitForLoadSucceeded(webEngineView()));

    QCOMPARE(webEngineView()->url(), urlFromTestPath("html/basic_page2.html"));
}

void tst_QQuickWebEngineView::reload()
{
    webEngineView()->setUrl(urlFromTestPath("html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView()));

    QCOMPARE(webEngineView()->url(), urlFromTestPath("html/basic_page.html"));

    webEngineView()->reload();
    QVERIFY(waitForLoadSucceeded(webEngineView()));

    QCOMPARE(webEngineView()->url(), urlFromTestPath("html/basic_page.html"));
}

void tst_QQuickWebEngineView::stop()
{
    webEngineView()->setUrl(urlFromTestPath("html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView()));

    QCOMPARE(webEngineView()->url(), urlFromTestPath("html/basic_page.html"));

    webEngineView()->stop();
}

void tst_QQuickWebEngineView::loadProgress()
{
    QCOMPARE(webEngineView()->loadProgress(), 0);

    webEngineView()->setUrl(urlFromTestPath("html/basic_page.html"));
    QSignalSpy loadProgressChangedSpy(webEngineView(), SIGNAL(loadProgressChanged()));
    QVERIFY(waitForLoadSucceeded(webEngineView()));

    loadProgressChangedSpy.wait();
    QTRY_COMPARE(webEngineView()->loadProgress(), 100);
}

void tst_QQuickWebEngineView::show()
{
    // This should not crash.
    m_window->show();
    QTest::qWait(200);
    m_window->hide();
}

void tst_QQuickWebEngineView::showWebEngineView()
{
    webEngineView()->setUrl(urlFromTestPath("html/direct-image-compositing.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView()));
    m_window->show();
    // This should not crash.
    webEngineView()->setVisible(true);
    QTest::qWait(200);
    webEngineView()->setVisible(false);
    QTest::qWait(200);
}

void tst_QQuickWebEngineView::removeFromCanvas()
{
    showWebEngineView();

    // This should not crash.
    QQuickItem *parent = webEngineView()->parentItem();
    QQuickItem noCanvasItem;
    webEngineView()->setParentItem(&noCanvasItem);
    QTest::qWait(200);
    webEngineView()->setParentItem(parent);
    webEngineView()->setVisible(true);
    QTest::qWait(200);
}

void tst_QQuickWebEngineView::multipleWebEngineViewWindows()
{
    showWebEngineView();

    // This should not crash.
    QQuickWebEngineView *webEngineView1 = newWebEngineView();
    QScopedPointer<TestWindow> window1(new TestWindow(webEngineView1));
    QQuickWebEngineView *webEngineView2 = newWebEngineView();
    QScopedPointer<TestWindow> window2(new TestWindow(webEngineView2));

    webEngineView1->setUrl(urlFromTestPath("html/scroll.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView1));
    window1->show();
    webEngineView1->setVisible(true);

    webEngineView2->setUrl(urlFromTestPath("html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView2));
    window2->show();
    webEngineView2->setVisible(true);
    QTest::qWait(200);
}

void tst_QQuickWebEngineView::multipleWebEngineViews()
{
    showWebEngineView();

    // This should not crash.
    QScopedPointer<QQuickWebEngineView> webEngineView1(newWebEngineView());
    webEngineView1->setParentItem(m_window->contentItem());
    QScopedPointer<QQuickWebEngineView> webEngineView2(newWebEngineView());
    webEngineView2->setParentItem(m_window->contentItem());

    webEngineView1->setSize(QSizeF(300, 400));
    webEngineView1->setUrl(urlFromTestPath("html/scroll.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView1.data()));
    webEngineView1->setVisible(true);

    webEngineView2->setSize(QSizeF(300, 400));
    webEngineView2->setUrl(urlFromTestPath("html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView2.data()));
    webEngineView2->setVisible(true);
    QTest::qWait(200);
}

QImage tryToGrabWindowUntil(QQuickWindow *window, std::function<bool(const QImage &)> checkImage,
                            int timeout = 5000)
{
    QImage grabbed;
    QElapsedTimer t;
    t.start();
    do {
        QTest::qWait(200);
        grabbed = window->grabWindow();
        if (checkImage(grabbed))
            break;
    } while (!t.hasExpired(timeout));
    return grabbed;
}

void tst_QQuickWebEngineView::basicRenderingSanity()
{
    if (QTestPrivate::isRunningArmOnX86())
        QSKIP("Grab does not work with QEMU.");

    showWebEngineView();

    webEngineView()->setUrl(QUrl(QString::fromUtf8("data:text/html,<html><body bgcolor=\"%2300ff00\"></body></html>")));
    QVERIFY(waitForLoadSucceeded(webEngineView()));

    // This should not crash.
    webEngineView()->setVisible(true);

    QRgb testColor = qRgba(0, 0xff, 0, 0xff);
    const QImage grabbedWindow = tryToGrabWindowUntil(m_window.data(),
                                                      [testColor] (const QImage &image) {
        return image.pixel(10, 10) == testColor;
    });
    QVERIFY(grabbedWindow.pixel(10, 10) == testColor);
    QVERIFY(grabbedWindow.pixel(100, 10) == testColor);
    QVERIFY(grabbedWindow.pixel(10, 100) == testColor);
    QVERIFY(grabbedWindow.pixel(100, 100) == testColor);
}

void tst_QQuickWebEngineView::titleUpdate()
{
    QSignalSpy titleSpy(webEngineView(), SIGNAL(titleChanged()));

    // Load page with no title
    webEngineView()->setUrl(urlFromTestPath("html/basic_page2.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView()));
    QCOMPARE(titleSpy.size(), 1);

    titleSpy.clear();

    // No titleChanged signal for failed load (with no error-page)
    webEngineView()->settings()->setErrorPageEnabled(false);
    webEngineView()->setUrl(urlFromTestPath("html/file_that_does_not_exist.html"));
    QVERIFY(waitForLoadFailed(webEngineView()));
    QCOMPARE(titleSpy.size(), 0);
}

void tst_QQuickWebEngineView::transparentWebEngineViews()
{
    if (QTestPrivate::isRunningArmOnX86())
        QSKIP("Grab does not work with QEMU.");

    showWebEngineView();

    // This should not crash.
    QScopedPointer<QQuickWebEngineView> webEngineView1(newWebEngineView());
    webEngineView1->setParentItem(m_window->contentItem());
    QScopedPointer<QQuickWebEngineView> webEngineView2(newWebEngineView());
    webEngineView2->setParentItem(m_window->contentItem());

    QVERIFY(webEngineView1->backgroundColor() != Qt::transparent);
    webEngineView2->setBackgroundColor(Qt::transparent);
    QVERIFY(webEngineView2->backgroundColor() == Qt::transparent);

    webEngineView1->setSize(QSizeF(300, 400));
    webEngineView1->loadHtml("<html><body bgcolor=\"red\"></body></html>");
    QVERIFY(waitForLoadSucceeded(webEngineView1.data(), 30000));

    webEngineView2->setSize(QSizeF(300, 400));
    webEngineView2->setUrl(urlFromTestPath("/html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView2.data()));

    // Result image: black text on red background.
    QSet<QRgb> colors;
    tryToGrabWindowUntil(m_window.data(), [&colors] (const QImage &image) {
        colors.clear();
        for (int i = 0; i < image.width(); i++)
            for (int j = 0; j < image.height(); j++)
                colors.insert(image.pixel(i, j));
        return colors.size() > 1;
    });

    QVERIFY(colors.size() > 1);
    QVERIFY(colors.contains(qRgb(0, 0, 0)));     // black
    QVERIFY(colors.contains(qRgb(255, 0, 0)));   // red
    for (auto color : colors) {
        QCOMPARE(qGreen(color), 0);
        QCOMPARE(qBlue(color), 0);
    }
}

void tst_QQuickWebEngineView::inputMethod()
{
    m_window->show();
    QQuickItem *input;

    QQuickWebEngineView *view = webEngineView();
    view->settings()->setFocusOnNavigationEnabled(true);
    view->setUrl(urlFromTestPath("html/inputmethod.html"));
    QVERIFY(waitForLoadSucceeded(view));

    QTRY_VERIFY(qobject_cast<QQuickItem *>(qApp->focusObject()));
    input = qobject_cast<QQuickItem *>(qApp->focusObject());
    QVERIFY(!input->flags().testFlag(QQuickItem::ItemAcceptsInputMethod));
    QVERIFY(!view->flags().testFlag(QQuickItem::ItemAcceptsInputMethod));

    runJavaScript("document.getElementById('inputField').focus();");
    QTRY_COMPARE(activeElementId(view), QStringLiteral("inputField"));
    QTRY_VERIFY(qobject_cast<QQuickItem *>(qApp->focusObject()));
    input = qobject_cast<QQuickItem *>(qApp->focusObject());
    QTRY_VERIFY(input->flags().testFlag(QQuickItem::ItemAcceptsInputMethod));
    QVERIFY(view->flags().testFlag(QQuickItem::ItemAcceptsInputMethod));

    runJavaScript("document.getElementById('inputField').blur();");
    QTRY_VERIFY(activeElementId(view).isEmpty());
    QTRY_VERIFY(qobject_cast<QQuickItem *>(qApp->focusObject()));
    input = qobject_cast<QQuickItem *>(qApp->focusObject());
    QTRY_VERIFY(!input->flags().testFlag(QQuickItem::ItemAcceptsInputMethod));
    QVERIFY(!view->flags().testFlag(QQuickItem::ItemAcceptsInputMethod));
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
        : commitCallCount(0)
        , resetCallCount(0)
    {
        QInputMethodPrivate* inputMethodPrivate = QInputMethodPrivate::get(qApp->inputMethod());
        inputMethodPrivate->testContext = this;
    }

    ~TestInputContext()
    {
        QInputMethodPrivate* inputMethodPrivate = QInputMethodPrivate::get(qApp->inputMethod());
        inputMethodPrivate->testContext = 0;
    }

    void commit() override { commitCallCount++; }
    void reset() override { resetCallCount++; }

    void update(Qt::InputMethodQueries queries) override
    {
        if (!qApp->focusObject())
            return;

        if (!(queries & Qt::ImQueryInput))
            return;

        QInputMethodQueryEvent imQueryEvent(Qt::ImQueryInput);
        QGuiApplication::sendEvent(qApp->focusObject(), &imQueryEvent);

        const int cursorPosition = imQueryEvent.value(Qt::ImCursorPosition).toInt();
        const int anchorPosition = imQueryEvent.value(Qt::ImAnchorPosition).toInt();
        QString surroundingText = imQueryEvent.value(Qt::ImSurroundingText).toString();
        QString selectedText = imQueryEvent.value(Qt::ImCurrentSelection).toString();

        infos.append(InputMethodInfo(cursorPosition, anchorPosition, surroundingText, selectedText));
    }

    int commitCallCount;
    int resetCallCount;
    QList<InputMethodInfo> infos;
};

void tst_QQuickWebEngineView::interruptImeTextComposition_data()
{
    QTest::addColumn<QString>("eventType");

    QTest::newRow("MouseButton") << QString("MouseButton");
#ifndef Q_OS_MACOS
    QTest::newRow("Touch") << QString("Touch");
#endif
}

void tst_QQuickWebEngineView::interruptImeTextComposition()
{
    m_window->show();
    QTRY_VERIFY(qApp->focusObject());
    QQuickItem *input;

    QQuickWebEngineView *view = webEngineView();
    view->settings()->setFocusOnNavigationEnabled(true);
    view->loadHtml("<html><body>"
                  "  <input type='text' id='input1' /><br>"
                  "  <input type='text' id='input2' />"
                  "</body></html>");
    QVERIFY(waitForLoadSucceeded(view));

    runJavaScript("document.getElementById('input1').focus();");
    QTRY_COMPARE(evaluateJavaScriptSync(view, "document.activeElement.id").toString(), QStringLiteral("input1"));

    TestInputContext testContext;

    // Send temporary text, which makes the editor has composition 'x'
    QList<QInputMethodEvent::Attribute> attributes;
    QInputMethodEvent event("x", attributes);
    input = qobject_cast<QQuickItem *>(qApp->focusObject());
    QGuiApplication::sendEvent(input, &event);
    QTRY_COMPARE(evaluateJavaScriptSync(view, "document.getElementById('input1').value").toString(), QStringLiteral("x"));

    // Focus 'input2' input field by an input event
    QFETCH(QString, eventType);
    if (eventType == "MouseButton") {
        QPoint textInputCenter = elementCenter(view, QStringLiteral("input2"));
        QTest::mouseClick(view->window(), Qt::LeftButton, {}, textInputCenter);
    } else if (eventType == "Touch") {
        QPoint textInputCenter = elementCenter(view, QStringLiteral("input2"));
        QTest::touchEvent(view->window(), touchDevice()).press(0, textInputCenter, view->window());
        QTest::touchEvent(view->window(), touchDevice()).release(0, textInputCenter, view->window());
    }
    QTRY_COMPARE(evaluateJavaScriptSync(view, "document.activeElement.id").toString(), QStringLiteral("input2"));
#ifndef Q_OS_WIN
    QTRY_COMPARE(testContext.commitCallCount, 1);
#else
    QTRY_COMPARE(testContext.resetCallCount, 1);
#endif

    // Check the composition text has been committed
    runJavaScript("document.getElementById('input1').focus();");
    QTRY_COMPARE(evaluateJavaScriptSync(view, "document.activeElement.id").toString(), QStringLiteral("input1"));
    input = qobject_cast<QQuickItem *>(qApp->focusObject());
    QTRY_COMPARE(input->inputMethodQuery(Qt::ImSurroundingText).toString(), QStringLiteral("x"));
}

void tst_QQuickWebEngineView::inputContextQueryInput()
{
    m_window->show();
    QTRY_VERIFY(qApp->focusObject());
    TestInputContext testContext;

    QQuickWebEngineView *view = webEngineView();
    view->settings()->setFocusOnNavigationEnabled(true);
    view->loadHtml("<html><body>"
                  "  <input type='text' id='input1' />"
                  "</body></html>");
    QVERIFY(waitForLoadSucceeded(view));
    QCOMPARE(testContext.infos.size(), 0);

    // Set focus on an input field.
    QPoint textInputCenter = elementCenter(view, "input1");
    QTest::mouseClick(view->window(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(testContext.infos.size(), 2);
    QCOMPARE(evaluateJavaScriptSync(view, "document.activeElement.id").toString(), QStringLiteral("input1"));
    foreach (const InputMethodInfo &info, testContext.infos) {
        QCOMPARE(info.cursorPosition, 0);
        QCOMPARE(info.anchorPosition, 0);
        QCOMPARE(info.surroundingText, QStringLiteral(""));
        QCOMPARE(info.selectedText, QStringLiteral(""));
    }
    testContext.infos.clear();

    // Change content of an input field from JavaScript.
    evaluateJavaScriptSync(view, "document.getElementById('input1').value='QtWebEngine';");
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 11);
    QCOMPARE(testContext.infos[0].anchorPosition, 11);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("QtWebEngine"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    testContext.infos.clear();

    // Change content of an input field by key press.
    QTest::keyClick(view->window(), Qt::Key_Exclam);
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 12);
    QCOMPARE(testContext.infos[0].anchorPosition, 12);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("QtWebEngine!"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    testContext.infos.clear();

    // Change cursor position.
    QTest::keyClick(view->window(), Qt::Key_Left);
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
        QGuiApplication::sendEvent(qApp->focusObject(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 2);

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

    // Clear selection by IME.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent::Attribute newSelection(QInputMethodEvent::Selection, 0, 0, QVariant());
        attributes.append(newSelection);
        QInputMethodEvent event("", attributes);
        QGuiApplication::sendEvent(qApp->focusObject(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 0);
    QCOMPARE(testContext.infos[0].anchorPosition, 0);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("QtWebEngine!"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    testContext.infos.clear();

    // Compose text.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("123", attributes);
        QGuiApplication::sendEvent(qApp->focusObject(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 3);
    QCOMPARE(testContext.infos[0].anchorPosition, 3);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("QtWebEngine!"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    QCOMPARE(evaluateJavaScriptSync(view, "document.getElementById('input1').value").toString(), QStringLiteral("123QtWebEngine!"));
    testContext.infos.clear();

    // Cancel composition.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        QGuiApplication::sendEvent(qApp->focusObject(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 2);
    foreach (const InputMethodInfo &info, testContext.infos) {
        QCOMPARE(info.cursorPosition, 0);
        QCOMPARE(info.anchorPosition, 0);
        QCOMPARE(info.surroundingText, QStringLiteral("QtWebEngine!"));
        QCOMPARE(info.selectedText, QStringLiteral(""));
    }
    QCOMPARE(evaluateJavaScriptSync(view, "document.getElementById('input1').value").toString(), QStringLiteral("QtWebEngine!"));
    testContext.infos.clear();

    // Commit text.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString(QStringLiteral("123"), 0, 0);
        QGuiApplication::sendEvent(qApp->focusObject(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 3);
    QCOMPARE(testContext.infos[0].anchorPosition, 3);
    QCOMPARE(testContext.infos[0].surroundingText, QStringLiteral("123QtWebEngine!"));
    QCOMPARE(testContext.infos[0].selectedText, QStringLiteral(""));
    QCOMPARE(evaluateJavaScriptSync(view, "document.getElementById('input1').value").toString(), QStringLiteral("123QtWebEngine!"));
    testContext.infos.clear();

    // Focus out.
    QTest::keyPress(view->window(), Qt::Key_Tab);
    QTRY_COMPARE(testContext.infos.size(), 1);
    QTRY_COMPARE(evaluateJavaScriptSync(view, "document.activeElement.id").toString(), QStringLiteral(""));
    testContext.infos.clear();
}

void tst_QQuickWebEngineView::inputMethodHints()
{
    m_window->show();
    QTRY_VERIFY(qApp->focusObject());
    QQuickItem *input;

    QQuickWebEngineView *view = webEngineView();
    view->settings()->setFocusOnNavigationEnabled(true);
    view->setUrl(urlFromTestPath("html/inputmethod.html"));
    QVERIFY(waitForLoadSucceeded(view));

    // Initialize input fields with values to check input method query is being updated.
    runJavaScript("document.getElementById('emailInputField').value = 'a@b.com';");
    runJavaScript("document.getElementById('editableDiv').innerText = 'bla';");

    // Setting focus on an input element results in an element in its shadow tree becoming the focus node.
    // Input hints should not be set from this shadow tree node but from the input element itself.
    runJavaScript("document.getElementById('emailInputField').focus();");
    QTRY_COMPARE(activeElementId(view), QStringLiteral("emailInputField"));
    input = qobject_cast<QQuickItem *>(qApp->focusObject());
    QTRY_COMPARE(input->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("a@b.com"));
    QVERIFY(input->flags().testFlag(QQuickItem::ItemAcceptsInputMethod));
    QVERIFY(view->flags().testFlag(QQuickItem::ItemAcceptsInputMethod));
    {
        QInputMethodQueryEvent query(Qt::ImHints);
        QGuiApplication::sendEvent(input, &query);
        QTRY_COMPARE(
                Qt::InputMethodHints(query.value(Qt::ImHints).toUInt() & Qt::ImhExclusiveInputMask),
                Qt::ImhEmailCharactersOnly);
    }

    // The focus of an editable DIV is given directly to it, so no shadow root element
    // is necessary. This tests the WebPage::editorState() method ability to get the
    // right element without breaking.
    runJavaScript("document.getElementById('editableDiv').focus();");
    QTRY_COMPARE(activeElementId(view), QStringLiteral("editableDiv"));
    input = qobject_cast<QQuickItem *>(qApp->focusObject());
    QTRY_COMPARE(input->inputMethodQuery(Qt::ImSurroundingText).toString(), QString("bla"));
    QVERIFY(input->flags().testFlag(QQuickItem::ItemAcceptsInputMethod));
    QVERIFY(view->flags().testFlag(QQuickItem::ItemAcceptsInputMethod));
    {
        QInputMethodQueryEvent query(Qt::ImHints);
        QGuiApplication::sendEvent(input, &query);
        QTRY_COMPARE(Qt::InputMethodHints(query.value(Qt::ImHints).toUInt()),
                     Qt::ImhPreferLowercase | Qt::ImhNoPredictiveText | Qt::ImhMultiLine
                             | Qt::ImhNoEditMenu | Qt::ImhNoTextHandles);
    }
}

void tst_QQuickWebEngineView::setZoomFactor()
{
    QQuickWebEngineView *view = webEngineView();
    m_window->show();
    view->setSize(QSizeF(320, 240));

    QCOMPARE(view->zoomFactor(), 1.0);
    view->setZoomFactor(2.5);
    QCOMPARE(view->zoomFactor(), 2.5);

    const QUrl url1 = urlFromTestPath("html/basic_page.html"), url2 = urlFromTestPath("html/basic_page2.html");

    view->setUrl(url1);
    QVERIFY(waitForLoadSucceeded(view));
    QCOMPARE(view->zoomFactor(), 2.5);

    view->setZoomFactor(0.1);
    QCOMPARE(view->zoomFactor(), 2.5);

    view->setZoomFactor(5.5);
    QCOMPARE(view->zoomFactor(), 2.5);

    QScopedPointer<QQuickWebEngineView> view2(newWebEngineView());
    view2->setSize(QSizeF(320, 240));
    view2->setParentItem(m_window->contentItem());

    // try loading different url and check new values after load
    for (auto &&p : {
            qMakePair(view, 2.5), // navigating away to different url should keep zoom
            qMakePair(view2.get(), 1.0), // same url navigation in diffent page shouldn't be affected
        }) {
        auto &&view = p.first; auto zoomFactor = p.second;
        view->setUrl(url2);
        QVERIFY(waitForLoadSucceeded(view));
        QCOMPARE(view->zoomFactor(), zoomFactor);
    }

    // should have no influence on first page
    view2->setZoomFactor(3.5);
    for (auto &&p : { qMakePair(view, 2.5), qMakePair(view2.get(), 3.5), })
        QCOMPARE(p.first->zoomFactor(), p.second);
}

void tst_QQuickWebEngineView::printToPdf()
{
#if !QT_CONFIG(webengine_printing_and_pdf)
    QSKIP("no webengine-printing-and-pdf");
#else
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_qwebengineview-XXXXXX");
    QVERIFY(tempDir.isValid());
    QQuickWebEngineView *view = webEngineView();
    view->setUrl(urlFromTestPath("html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(view));

    QSignalSpy savePdfSpy(view, SIGNAL(pdfPrintingFinished(const QString&, bool)));
    QString path = tempDir.path() + "/print_success.pdf";
    view->printToPdf(path, QQuickWebEngineView::A4, QQuickWebEngineView::Portrait);
    QTRY_VERIFY2(savePdfSpy.size() == 1, "Printing to PDF file failed without signal");
    QList<QVariant> successArguments = savePdfSpy.takeFirst();
    QVERIFY2(successArguments.at(0).toString() == path, "File path for first saved PDF does not match arguments");
    QVERIFY2(successArguments.at(1).toBool() == true, "Printing to PDF file failed though it should succeed");

#if !defined(Q_OS_WIN)
    path = tempDir.path() + "/print_//fail.pdf";
#else
    path = tempDir.path() + "/print_|fail.pdf";
#endif // #if !defined(Q_OS_WIN)
    view->printToPdf(path, QQuickWebEngineView::A4, QQuickWebEngineView::Portrait);
    QTRY_VERIFY2(savePdfSpy.size() == 1, "Printing to PDF file failed without signal");
    QList<QVariant> failedArguments = savePdfSpy.takeFirst();
    QVERIFY2(failedArguments.at(0).toString() == path, "File path for second saved PDF does not match arguments");
    QVERIFY2(failedArguments.at(1).toBool() == false, "Printing to PDF file succeeded though it should fail");
#endif // !QT_CONFIG(webengine_printing_and_pdf)
}

void tst_QQuickWebEngineView::stopSettingFocusWhenDisabled()
{
    QFETCH(bool, viewEnabled);
    QFETCH(bool, activeFocusResult);

    QQuickWebEngineView *view = webEngineView();
    m_window->show();
    view->settings()->setFocusOnNavigationEnabled(true);
    view->setSize(QSizeF(640, 480));
    view->setEnabled(viewEnabled);
    view->loadHtml("<html><head><title>Title</title></head><body>Hello"
                   "<input id=\"input\" type=\"text\"></body></html>");
    QVERIFY(waitForLoadSucceeded(view));

    // When enabled, the view should get active focus after the page is loaded.
    QTRY_COMPARE_WITH_TIMEOUT(view->hasActiveFocus(), activeFocusResult, 1000);
    view->runJavaScript("document.getElementById(\"input\").focus()");
    QTRY_COMPARE_WITH_TIMEOUT(view->hasActiveFocus(), activeFocusResult, 1000);
}

void tst_QQuickWebEngineView::stopSettingFocusWhenDisabled_data()
{
    QTest::addColumn<bool>("viewEnabled");
    QTest::addColumn<bool>("activeFocusResult");

    QTest::newRow("enabled view gets active focus") << true << true;
    QTest::newRow("disabled view does not get active focus") << false << false;
}

class MouseTouchEventRecordingItem : public QQuickItem {
    Q_OBJECT
public:
    explicit MouseTouchEventRecordingItem(QQuickItem* child, QQuickItem *parent = 0) :
        QQuickItem(parent), m_eventCounter(0), m_child(child) {
        setFlag(ItemHasContents);
        setAcceptedMouseButtons(Qt::AllButtons);
        setAcceptTouchEvents(true);
        setAcceptHoverEvents(true);
    }

    bool event(QEvent *event) override
    {
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
            ++m_eventCounter;
            event->accept();
            return true;
        default:
            break;
        }
        return QQuickItem::event(event);
    }

    void clearEventCount()
    {
        m_eventCounter = 0;
    }

    int eventCount()
    {
        return m_eventCounter;
    }

public Q_SLOTS:
    void changeWidth() {
        if (m_child)
            setWidth(m_child->width());
    }

    void changeHeight() {
        if (m_child)
            setHeight(m_child->height());
    }

private:
    int m_eventCounter;
    QQuickItem *m_child;
};

void tst_QQuickWebEngineView::inputEventForwardingDisabledWhenActiveFocusOnPressDisabled()
{
    QQuickWebEngineView *view = webEngineView();
    MouseTouchEventRecordingItem item(view);
    item.setParentItem(m_window->contentItem());

    // Resize the event recorder whenever the view is resized, so that all event positions
    // are contained in both of the item regions.
    QObject::connect(view, &QQuickItem::widthChanged, &item,
                     &MouseTouchEventRecordingItem::changeWidth);
    QObject::connect(view, &QQuickItem::heightChanged, &item,
                     &MouseTouchEventRecordingItem::changeHeight);
    view->setParentItem(&item);
    view->setSize(QSizeF(640, 480));
    m_window->show();

    // Simulate click and move of mouse, so that last known position in the application
    // is updated, thus a mouse move event is not generated when we don't expect it.
    QTest::mouseClick(view->window(), Qt::LeftButton);
    QTRY_COMPARE(item.eventCount(), 2);
    item.clearEventCount();

    // First disable view, so it does not receive focus on page load.
    view->setEnabled(false);
    view->setActiveFocusOnPress(false);
    view->loadHtml("<html><head>"
                   "<script>"
                   "window.onload = function() { document.getElementById(\"input\").focus(); }"
                   "</script>"
                   "<title>Title</title></head><body>Hello"
                   "<input id=\"input\" type=\"text\"></body></html>");
    QVERIFY(waitForLoadSucceeded(view));
    QTRY_COMPARE_WITH_TIMEOUT(view->hasActiveFocus(), false, 1000);

    // Enable the view back so we can try to interact with it.
    view->setEnabled(true);

    // Click on the view, to try and set focus.
    QTest::mouseClick(view->window(), Qt::LeftButton);

    // View should not have focus after click, because setActiveFocusOnPress is false.
    QTRY_COMPARE_WITH_TIMEOUT(view->hasActiveFocus(), false, 1000);

    // Now test sending various input events, to check that indeed all the input events are not
    // forwarded to Chromium, but rather processed and accepted by the view's parent item.
    QTest::mousePress(view->window(), Qt::LeftButton);
    QTest::mouseRelease(view->window(), Qt::LeftButton);

    QTest::touchEvent(view->window(), touchDevice()).press(0, QPoint(0,0), view->window());
    QTest::touchEvent(view->window(), touchDevice()).move(0, QPoint(1, 1), view->window());
    QTest::touchEvent(view->window(), touchDevice()).release(0, QPoint(1, 1), view->window());

    // We expect to catch 7 events - click = 2, press + release = 2, touches = 3.
    QCOMPARE(item.eventCount(), 7);

    // Manually forcing focus should still be possible.
    view->forceActiveFocus();
    QTRY_COMPARE_WITH_TIMEOUT(view->hasActiveFocus(), true, 1000);
}

void tst_QQuickWebEngineView::changeLocale()
{
    if (QTestPrivate::isRunningArmOnX86())
        QSKIP("Does not work with QEMU. (QTBUG-94911)");

    QStringList errorLines;
    QUrl url("http://non.existent/");

    QLocale::setDefault(QLocale("de"));
    QScopedPointer<QQuickWebEngineView> viewDE(newWebEngineView());
    viewDE->setUrl(url);
    QVERIFY(waitForLoadFailed(viewDE.data()));

    QTRY_VERIFY(!evaluateJavaScriptSync(viewDE.data(), "document.body").isNull());
    QTRY_VERIFY(!evaluateJavaScriptSync(viewDE.data(), "document.body.innerText").isNull());
    errorLines = evaluateJavaScriptSync(viewDE.data(), "document.body.innerText").toString().split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
    QCOMPARE(errorLines.first().toUtf8(), QByteArrayLiteral("Die Website ist nicht erreichbar"));

    QLocale::setDefault(QLocale("en"));
    QScopedPointer<QQuickWebEngineView> viewEN(newWebEngineView());
    viewEN->setUrl(url);
    QVERIFY(waitForLoadFailed(viewEN.data()));

    QTRY_VERIFY(!evaluateJavaScriptSync(viewEN.data(), "document.body").isNull());
    QTRY_VERIFY(!evaluateJavaScriptSync(viewEN.data(), "document.body.innerText").isNull());
    errorLines = evaluateJavaScriptSync(viewEN.data(), "document.body.innerText").toString().split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
    QCOMPARE(errorLines.first().toUtf8(), QByteArrayLiteral("This site can\xE2\x80\x99t be reached"));

    // Reset error page
    viewDE->setUrl(QUrl("about:blank"));
    QVERIFY(waitForLoadSucceeded(viewDE.data()));

    // Check whether an existing QWebEngineView keeps the language settings after changing the default locale
    viewDE->setUrl(url);
    QVERIFY(waitForLoadFailed(viewDE.data()));

    QTRY_VERIFY(!evaluateJavaScriptSync(viewDE.data(), "document.body").isNull());
    QTRY_VERIFY(!evaluateJavaScriptSync(viewDE.data(), "document.body.innerText").isNull());
    errorLines = evaluateJavaScriptSync(viewDE.data(), "document.body.innerText").toString().split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
    QCOMPARE(errorLines.first().toUtf8(), QByteArrayLiteral("Die Website ist nicht erreichbar"));
}

void tst_QQuickWebEngineView::userScripts()
{
    QScopedPointer<QQuickWebEngineView> webEngineView1(newWebEngineView());
    webEngineView1->setParentItem(m_window->contentItem());
    QScopedPointer<QQuickWebEngineView> webEngineView2(newWebEngineView());
    webEngineView2->setParentItem(m_window->contentItem());

    QQuickWebEngineScriptCollection *collection = webEngineView1->profile()->userScripts();
    QWebEngineScript script;
    script.setSourceCode("document.title = 'New title';");
    collection->insert(script);

    webEngineView1->setUrl(urlFromTestPath("html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView1.data()));
    QTRY_COMPARE(webEngineView1->title(), QStringLiteral("New title"));

    webEngineView2->setUrl(urlFromTestPath("html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView2.data()));
    QTRY_COMPARE(webEngineView2->title(), QStringLiteral("New title"));

    collection->clear();
}

void tst_QQuickWebEngineView::javascriptClipboard_data()
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

void tst_QQuickWebEngineView::javascriptClipboard()
{
    QFETCH(bool, javascriptCanAccessClipboard);
    QFETCH(bool, javascriptCanPaste);
    QFETCH(bool, copyResult);
    QFETCH(bool, pasteResult);

    // check defaults
    QCOMPARE(webEngineView()->settings()->javascriptCanAccessClipboard(), false);
    QCOMPARE(webEngineView()->settings()->javascriptCanPaste(), false);

    // check accessors
    webEngineView()->settings()->setJavascriptCanAccessClipboard(javascriptCanAccessClipboard);
    webEngineView()->settings()->setJavascriptCanPaste(javascriptCanPaste);
    QCOMPARE(webEngineView()->settings()->javascriptCanAccessClipboard(),
             javascriptCanAccessClipboard);
    QCOMPARE(webEngineView()->settings()->javascriptCanPaste(), javascriptCanPaste);

    QQuickWebEngineView *view = webEngineView();
    view->loadHtml("<html><body>"
                   "<input type='text' value='OriginalText' id='myInput'/>"
                   "</body></html>");
    QVERIFY(waitForLoadSucceeded(view));

    // make sure that 'OriginalText' is selected
    evaluateJavaScriptSync(view, "document.getElementById('myInput').select()");
    QCOMPARE(evaluateJavaScriptSync(view, "window.getSelection().toString()").toString(),
             QStringLiteral("OriginalText"));

    // Check that the actual settings work by the
    // - return value of queryCommandEnabled and
    // - return value of execCommand
    // - comparing the clipboard / input field
    QGuiApplication::clipboard()->setText(QString());
    QCOMPARE(evaluateJavaScriptSync(view, "document.queryCommandEnabled('copy')").toBool(),
             copyResult);
    QCOMPARE(evaluateJavaScriptSync(view, "document.execCommand('copy')").toBool(), copyResult);
    QCOMPARE(QGuiApplication::clipboard()->text(),
             (copyResult ? QString("OriginalText") : QString()));

    QGuiApplication::clipboard()->setText("AnotherText");
    QCOMPARE(evaluateJavaScriptSync(view, "document.queryCommandEnabled('paste')").toBool(),
             pasteResult);
    QCOMPARE(evaluateJavaScriptSync(view, "document.execCommand('paste')").toBool(), pasteResult);
    QCOMPARE(evaluateJavaScriptSync(view, "document.getElementById('myInput').value").toString(),
                (pasteResult ? QString("AnotherText") : QString("OriginalText")));

    // Test settings on clipboard permissions
    evaluateJavaScriptSync(view,
        QStringLiteral(
            "var accessGranted = false;"
            "var accessDenied = false;"
            "var accessPrompt = false;"
            "navigator.permissions.query({name:'clipboard-write'})"
            ".then(result => {"
                "if (result.state == 'granted') accessGranted = true;"
                "if (result.state == 'denied') accessDenied = true;"
                "if (result.state == 'prompt') accessPrompt = true;"
            "})"));

    QTRY_COMPARE(evaluateJavaScriptSync(view, "accessGranted").toBool(), copyResult);
    QTRY_COMPARE(evaluateJavaScriptSync(view, "accessDenied").toBool(), !javascriptCanAccessClipboard);
    QTRY_COMPARE(evaluateJavaScriptSync(view, "accessPrompt").toBool(), false);

    evaluateJavaScriptSync(view,
        QStringLiteral(
            "accessGranted = false;"
            "accessDenied = false;"
            "accessPrompt = false;"
            "navigator.permissions.query({name:'clipboard-read'})"
            ".then(result => {"
                "if (result.state == 'granted') accessGranted = true;"
                "if (result.state == 'denied') accessDenied = true;"
                "if (result.state == 'prompt') accessPrompt = true;"
            "})"));

    QTRY_COMPARE(evaluateJavaScriptSync(view, "accessGranted").toBool(), pasteResult);
    QTRY_COMPARE(evaluateJavaScriptSync(view, "accessDenied").toBool(), !javascriptCanAccessClipboard || !javascriptCanPaste);
    QTRY_COMPARE(evaluateJavaScriptSync(view, "accessPrompt").toBool(), false);
}

void tst_QQuickWebEngineView::setProfile() {
    QSignalSpy loadSpy(webEngineView(), SIGNAL(loadingChanged(QWebEngineLoadingInfo)));
    webEngineView()->setUrl(urlFromTestPath("html/basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView()));
    QCOMPARE(loadSpy.size(), 2);
    webEngineView()->setUrl(urlFromTestPath("html/basic_page2.html"));
    QVERIFY(waitForLoadSucceeded(webEngineView()));
    QCOMPARE(loadSpy.size(), 4);
    QQuickWebEngineProfile *profile = new QQuickWebEngineProfile();
    auto oldProfile = webEngineView()->profile();
    auto sc = qScopeGuard([&] () { webEngineView()->setProfile(oldProfile); delete profile; });
    webEngineView()->setProfile(profile);
    QTRY_COMPARE(webEngineView()->url() ,urlFromTestPath("html/basic_page2.html"));
}

#if QT_CONFIG(accessibility)
void tst_QQuickWebEngineView::focusChild_data()
{
    QTest::addColumn<QString>("interfaceName");
    QTest::addColumn<QList<QAccessible::Role>>("ancestorRoles");

    QTest::newRow("QQuickWebEngineView") << QString("QQuickWebEngineView") << QList<QAccessible::Role>({QAccessible::Client});
    QTest::newRow("RenderWidgetHostViewQtDelegate") << QString("RenderWidgetHostViewQtDelegate") << QList<QAccessible::Role>({QAccessible::Client});
    QTest::newRow("QQuickView") << QString("QQuickView") << QList<QAccessible::Role>({QAccessible::Window, QAccessible::Client /* view */});
}

void tst_QQuickWebEngineView::focusChild()
{
    auto traverseToWebDocumentAccessibleInterface = [](QAccessibleInterface *iface) -> QAccessibleInterface * {
        QFETCH(QList<QAccessible::Role>, ancestorRoles);
        for (int i = 0; i < ancestorRoles.size(); ++i) {
            if (iface->childCount() == 0 || iface->role() != ancestorRoles[i])
                return nullptr;
            iface = iface->child(0);
        }

        if (iface->role() != QAccessible::WebDocument)
            return nullptr;

        return iface;
    };

    QQuickWebEngineView *view = webEngineView();
    m_window->show();
    view->settings()->setFocusOnNavigationEnabled(true);
    view->setSize(QSizeF(640, 480));
    view->loadHtml("<html><body>"
                   "<input id='input1' type='text'>"
                   "</body></html>");
    QVERIFY(waitForLoadSucceeded(view));

    QAccessibleInterface *iface = nullptr;
    QFETCH(QString, interfaceName);
    if (interfaceName == "QQuickWebEngineView")
        iface = QAccessible::queryAccessibleInterface(view);
    else if (interfaceName == "RenderWidgetHostViewQtDelegate")
        iface = QAccessible::queryAccessibleInterface(m_window->focusObject());
    else if (interfaceName == "QQuickView")
        iface = QAccessible::queryAccessibleInterface(m_window.data());
    QVERIFY(iface);

    // Make sure the input field does not have the focus.
    runJavaScript("document.getElementById('input1').blur();");
    QTRY_VERIFY(evaluateJavaScriptSync(view, "document.activeElement.id").toString().isEmpty());

    QVERIFY(iface->focusChild());
    QTRY_COMPARE(iface->focusChild()->role(), QAccessible::WebDocument);
    QCOMPARE(traverseToWebDocumentAccessibleInterface(iface), iface->focusChild());

    // Set active focus on the input field.
    runJavaScript("document.getElementById('input1').focus();");
    QTRY_COMPARE(evaluateJavaScriptSync(view, "document.activeElement.id").toString(), QStringLiteral("input1"));

    QVERIFY(iface->focusChild());
    QTRY_COMPARE(iface->focusChild()->role(), QAccessible::EditableText);
    // <html> -> <body> -> <input>
    QCOMPARE(traverseToWebDocumentAccessibleInterface(iface)->child(0)->child(0), iface->focusChild());
}
#endif // QT_CONFIG(accessibility)

void tst_QQuickWebEngineView::htmlSelectPopup()
{
    m_window->show();
    QQuickWebEngineView &view = *webEngineView();
    view.settings()->setFocusOnNavigationEnabled(true);
    view.setSize(QSizeF(640, 480));
    view.loadHtml("<html><body>"
                   "<select id='select' onchange='console.log(\"option changed to: \" + this.value)'>"
                   "<option value='O1'>O1</option><option value='O2'>O2</option><option value='O3'>O3</option></select>"
                   "</body></html>");
    QVERIFY(waitForLoadSucceeded(&view));

    auto makeTouch = [this] (QWindow *w, const QPoint &p) {
        QTest::touchEvent(w, touchDevice()).press(1, p);
        QTest::touchEvent(w, touchDevice()).release(1, p);
    };

    makeTouch(view.window(), elementCenter(&view, "select"));
    QPointer<QQuickWindow> popup;
    QTRY_VERIFY((popup = m_window->findChild<QQuickWindow *>()));
    QCOMPARE(activeElementId(&view), QStringLiteral("select"));

    makeTouch(popup, QPoint(popup->width() / 2, popup->height() / 2));
    QTRY_VERIFY(!popup);
    QCOMPARE(evaluateJavaScriptSync(&view, "document.getElementById('select').value").toString(), QStringLiteral("O2"));
}

#if QT_CONFIG(accessibility)
static QByteArrayList params = QByteArrayList()
    << "--force-renderer-accessibility";
#else
static QByteArrayList params;
#endif

W_QTEST_MAIN(tst_QQuickWebEngineView, params)
#include "tst_qquickwebengineview.moc"
#include "moc_quickutil.cpp"
