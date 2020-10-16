/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QSignalSpy>
#include <QTest>
#include <QTouchDevice>
#include <QWebEngineSettings>
#include <QWebEngineView>

static QTouchDevice* s_touchDevice = nullptr;

class TouchInputTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void touchTap();
    void touchTapAndHold();
    void touchTapAndHoldCancelled();

private:
    QWebEngineView view;
    QSignalSpy loadSpy { &view, &QWebEngineView::loadFinished };
    QPoint notextCenter, textCenter, inputCenter;

    QString activeElement() { return evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(); }
};

void TouchInputTest::initTestCase()
{
    s_touchDevice = QTest::createTouchDevice();

    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);

    view.show(); view.resize(480, 320);
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    view.setHtml("<html><body>"
                 "<p id='text' style='width: 150px;'>The Qt Company</p>"
                 "<div id='notext' style='width: 150px; height: 100px; background-color: #f00;'></div>"
                 "<form><input id='input' width='150px' type='text' value='The Qt Company2' /></form>"
                 "</body></html>");
    QVERIFY(loadSpy.wait() && loadSpy.first().first().toBool());

    notextCenter = elementCenter(view.page(), "notext");
    textCenter = elementCenter(view.page(), "text");
    inputCenter = elementCenter(view.page(), "input");
}

void TouchInputTest::init()
{
    QCOMPARE(activeElement(), QString());
}

void TouchInputTest::cleanup()
{
    evaluateJavaScriptSync(view.page(), "if (document.activeElement) document.activeElement.blur()");
}

void TouchInputTest::touchTap()
{
    auto singleTap = [target = view.focusProxy()] (const QPoint& tapCoords) -> void {
        QTest::touchEvent(target, s_touchDevice).press(1, tapCoords, target);
        QTest::touchEvent(target, s_touchDevice).stationary(1);
        QTest::touchEvent(target, s_touchDevice).release(1, tapCoords, target);
    };

    // Single tap on text doesn't trigger a selection
    singleTap(textCenter);
    QTRY_COMPARE(activeElement(), QString());
    QTRY_VERIFY(!view.hasSelection());

    // Single tap inside the input field focuses it without selecting the text
    singleTap(inputCenter);
    QTRY_COMPARE(activeElement(), QStringLiteral("input"));
    QTRY_VERIFY(!view.hasSelection());

    // Single tap on the div clears the input field focus
    singleTap(notextCenter);
    QTRY_COMPARE(activeElement(), QString());

    // Double tap on text still doesn't trigger a selection
    singleTap(textCenter);
    singleTap(textCenter);
    QTRY_COMPARE(activeElement(), QString());
    QTRY_VERIFY(!view.hasSelection());

    // Double tap inside the input field focuses it and selects the word under it
    singleTap(inputCenter);
    singleTap(inputCenter);
    QTRY_COMPARE(activeElement(), QStringLiteral("input"));
    QTRY_COMPARE(view.selectedText(), QStringLiteral("Company2"));

    // Double tap outside the input field behaves like a single tap: clears its focus and selection
    singleTap(notextCenter);
    singleTap(notextCenter);
    QTRY_COMPARE(activeElement(), QString());
    QTRY_VERIFY(!view.hasSelection());
}

void TouchInputTest::touchTapAndHold()
{
    auto tapAndHold = [target = view.focusProxy()] (const QPoint& tapCoords) -> void {
        QTest::touchEvent(target, s_touchDevice).press(1, tapCoords, target);
        QTest::touchEvent(target, s_touchDevice).stationary(1);
        QTest::qWait(1000);
        QTest::touchEvent(target, s_touchDevice).release(1, tapCoords, target);
    };

    // Tap-and-hold on text selects the word under it
    tapAndHold(textCenter);
    QTRY_COMPARE(activeElement(), QString());
    QTRY_COMPARE(view.selectedText(), QStringLiteral("Company"));

    // Tap-and-hold inside the input field focuses it and selects the word under it
    tapAndHold(inputCenter);
    QTRY_COMPARE(activeElement(), QStringLiteral("input"));
    QTRY_COMPARE(view.selectedText(), QStringLiteral("Company2"));

    // Only test the page context menu on Windows, as Linux doesn't handle context menus consistently
    // and other non-desktop platforms like Android may not even support context menus at all
#if defined(Q_OS_WIN)
    // Tap-and-hold clears the text selection and shows the page's context menu
    QVERIFY(QApplication::activePopupWidget() == nullptr);
    tapAndHold(notextCenter);
    QTRY_COMPARE(activeElement(), QString());
    QTRY_VERIFY(!view.hasSelection());
    QTRY_VERIFY(QApplication::activePopupWidget() != nullptr);

    QApplication::activePopupWidget()->close();
    QVERIFY(QApplication::activePopupWidget() == nullptr);
#endif
}

void TouchInputTest::touchTapAndHoldCancelled()
{
    auto cancelledTapAndHold = [target = view.focusProxy()] (const QPoint& tapCoords) -> void {
        QTest::touchEvent(target, s_touchDevice).press(1, tapCoords, target);
        QTest::touchEvent(target, s_touchDevice).stationary(1);
        QTest::qWait(1000);
        QWindowSystemInterface::handleTouchCancelEvent(target->windowHandle(), s_touchDevice);
    };

    // A cancelled tap-and-hold should cancel text selection, but currently doesn't
    cancelledTapAndHold(textCenter);
    QEXPECT_FAIL("", "Incorrect Chromium selection behavior when cancelling tap-and-hold on text", Continue);
    QTRY_VERIFY_WITH_TIMEOUT(!view.hasSelection(), 100);

    // A cancelled tap-and-hold should cancel input field focusing and selection, but currently doesn't
    cancelledTapAndHold(inputCenter);
    QEXPECT_FAIL("", "Incorrect Chromium selection behavior when cancelling tap-and-hold on input field", Continue);
    QTRY_VERIFY_WITH_TIMEOUT(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString().isEmpty(), 100);
    QEXPECT_FAIL("", "Incorrect Chromium focus behavior when cancelling tap-and-hold on input field", Continue);
    QTRY_VERIFY_WITH_TIMEOUT(!view.hasSelection(), 100);

    // Only test the page context menu on Windows, as Linux doesn't handle context menus consistently
    // and other non-desktop platforms like Android may not even support context menus at all
#if defined(Q_OS_WIN)
    // A cancelled tap-and-hold cancels the context menu
    QVERIFY(QApplication::activePopupWidget() == nullptr);
    cancelledTapAndHold(notextCenter);
    QVERIFY(QApplication::activePopupWidget() == nullptr);
#endif
}

QTEST_MAIN(TouchInputTest)
#include "tst_touchinput.moc"
