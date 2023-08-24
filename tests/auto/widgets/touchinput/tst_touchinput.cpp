// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <util.h>

#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QSignalSpy>
#include <QTest>
#include <QPointingDevice>
#include <QWebEngineSettings>
#include <QWebEngineView>

static QPointingDevice* s_touchDevice = nullptr;

struct Page : QWebEnginePage
{
    QStringList alerts;
    void javaScriptAlert(const QUrl &/*origin*/, const QString &msg) override { alerts.append(msg); }
};

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
    void scrolling();
    void pinchZoom_data();
    void pinchZoom();
    void complexSequence();
    void buttonClickHandler();
    void htmlSelectPopup();

private:
    Page page;
    QWebEngineView view;
    QSignalSpy loadSpy { &view, &QWebEngineView::loadFinished };
    QPoint notextCenter, textCenter, inputCenter;

    QString activeElement() { return evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(); }

    void makeTouch(QWindow *w, const QPoint &p) {
        QTest::touchEvent(w, s_touchDevice).press(1, p);
        QTest::touchEvent(w, s_touchDevice).release(1, p);
    }
    void makeTouch(const QPoint &p) { makeTouch(view.windowHandle(), p); }

    void gestureScroll(bool down) {
        auto target = view.focusProxy();
        QPoint p(target->width() / 2, target->height() / 4 * (down ? 3 : 1));

        QTest::touchEvent(target, s_touchDevice).press(1, p, target);

        QSignalSpy spy(view.page(), &QWebEnginePage::scrollPositionChanged);
        for (int i = 0; i < 3; ++i) {
            down ? p -= QPoint(5, 15) : p += QPoint(5, 15);
            QTest::qWait(100); // too fast and events are recognized as fling gesture
            QTest::touchEvent(target, s_touchDevice).move(1, p, target);
            spy.wait();
        }

        QTest::touchEvent(target, s_touchDevice).release(1, p, target);
    }

    void gesturePinch(bool zoomIn, bool tapOneByOne = false) {
        auto target = view.focusProxy();
        QPoint p(target->width() / 2, target->height() / 2);
        auto t1 = p - QPoint(zoomIn ? 50 : 150, 10), t2 = p + QPoint(zoomIn ? 50 : 150, 10);

        if (tapOneByOne) {
            QTest::touchEvent(target, s_touchDevice).press(0, t1, target);
            QTest::touchEvent(target, s_touchDevice).stationary(0).press(1, t2, target);
        } else {
            QTest::touchEvent(target, s_touchDevice).press(0, t1, target).press(1, t2, target);
        }

        for (int i = 0; i < 3; ++i) {
            if (zoomIn) {
                t1 -= QPoint(25, 5);
                t2 += QPoint(25, 5);
            } else {
                t1 += QPoint(35, 5);
                t2 -= QPoint(35, 5);
            }
            QTest::qWait(100); // too fast and events are recognized as fling gesture
            QTest::touchEvent(target, s_touchDevice).move(1, t1, target).move(0, t2, target);
        }

        if (tapOneByOne) {
            QTest::touchEvent(target, s_touchDevice).stationary(0).release(1, t2, target);
            QTest::touchEvent(target, s_touchDevice).release(0, t1, target);
        } else {
            QTest::touchEvent(target, s_touchDevice).release(0, t1, target).release(1, t2, target);
        }
    }

    int getScrollPosition(int *position = nullptr) {
        int p = evaluateJavaScriptSync(view.page(), "window.scrollY").toInt();
        return position ? (*position = p) : p;
    }

    int pageScrollPosition() {
        // this one is updated later in page in asynchronous way
        return qRound(view.page()->scrollPosition().y());
    }

    double getScaleFactor(double *scale = nullptr)  {
        double s = evaluateJavaScriptSync(view.page(), "window.visualViewport.scale").toDouble();
        return scale ? (*scale = s) : s;
    }
};

void TouchInputTest::initTestCase()
{
    s_touchDevice = QTest::createTouchDevice();

    view.setPage(&page);
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);

    view.show(); view.resize(480, 320);
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    view.setHtml("<html><head><style>.rect { min-width: 240px; min-height: 120px; }</style></head><body>"
                 "<p id='text' style='width: 150px;'>The Qt Company</p>"
                 "<div id='notext' style='width: 150px; height: 100px; background-color: #f00;'></div>"
                 "<form><input id='input' style='width: 150px;' type='text' value='The Qt Company2' /></form>"
                 "<button id='btn' type='button' onclick='alert(\"button clicked!\")'>Click Me!</button>"
                 "<select id='select' onchange='alert(\"option changed to: \" + this.value)'>"
                 "<option value='O1'>O1</option><option value='O2'>O2</option><option value='O3'>O3</option></select>"
                 "<table style='width: 100%; padding: 15px; text-align: center;'>"
                 "<tr><td>BEFORE</td><td><div class='rect' style='background-color: #00f;'></div></td><td>AFTER</td></tr>"
                 "<tr><td>BEFORE</td><td><div class='rect' style='background-color: #0f0;'></div></td><td>AFTER</td></tr>"
                 "<tr><td>BEFORE</td><td><div class='rect' style='background-color: #f00;'></div></td><td>AFTER</td></tr></table>"
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
    evaluateJavaScriptSync(view.page(), "window.scrollTo(0, 0)");
    QTRY_COMPARE(getScrollPosition(), 0);
    QTRY_COMPARE(pageScrollPosition(), 0);
    page.alerts.clear();
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

void TouchInputTest::scrolling()
{
    int p = getScrollPosition();
    QCOMPARE(p, 0);

    // scroll a bit down...
    for (int i = 0; i < 3; ++i) {
        gestureScroll(/* down = */true);
        int positionBefore = p;
        QTRY_VERIFY2(getScrollPosition(&p) > positionBefore, qPrintable(QString("i: %1, position: %2 -> %3").arg(i).arg(positionBefore).arg(p)));
    }

    // ... and then scroll page again but in opposite direction
    for (int i = 0; i < 3; ++i) {
        gestureScroll(/* down = */false);
        int positionBefore = p;
        QTRY_VERIFY2(getScrollPosition(&p) < positionBefore, qPrintable(QString("i: %1, position: %2 -> %3").arg(i).arg(positionBefore).arg(p)));
    }

    QTRY_COMPARE(getScrollPosition(), 0);
}

void TouchInputTest::pinchZoom_data()
{
    QTest::addColumn<bool>("tapOneByOne");
    QTest::addRow("sequential") << true;
    QTest::addRow("simultaneous") << false;
}

void TouchInputTest::pinchZoom()
{
    QFETCH(bool, tapOneByOne);
    double scale = getScaleFactor();
    QCOMPARE(scale, 1.0);

    for (int i = 0; i < 3; ++i) {
        gesturePinch(/* zoomIn = */true, tapOneByOne);
        QTRY_VERIFY2(getScaleFactor(&scale) > 1.5, qPrintable(QString("i: %1, scale: %2").arg(i).arg(scale)));
        gesturePinch(/* zoomIn = */false, tapOneByOne);
        QTRY_COMPARE(getScaleFactor(&scale), 1.0);
    }
}

void TouchInputTest::complexSequence()
{
    auto t = view.focusProxy();
    QPoint pc(view.width() / 2, view.height() / 2), p1 = pc - QPoint(50, 25), p2 = pc + QPoint(50, 25);

    for (int i = 0; i < 4; ++i) {
        QTest::touchEvent(t, s_touchDevice).press(42, p1, t); QTest::qWait(50);
        QTest::touchEvent(t, s_touchDevice).stationary(42).press(24, p2, t); QTest::qWait(50);
        QTest::touchEvent(t, s_touchDevice).release(42, p1, t).release(24, p2, t);

        // for additional variablity add zooming in on even steps and zooming out on odd steps
        // MEMO scroll position will always be 0 while viewport scale factor > 1.0, so do zoom in after scroll
        bool zoomIn = i % 2 == 0;

        if (!zoomIn) {
            gesturePinch(false);
            QTRY_COMPARE(getScaleFactor(), 1.0);
        }

        int p = getScrollPosition(), positionBefore = p;
        gestureScroll(true);
        QTRY_VERIFY2_WITH_TIMEOUT(getScrollPosition(&p) > positionBefore, qPrintable(QString("i: %1, position: %2 -> %3").arg(i).arg(positionBefore).arg(p)), 1000);

        if (zoomIn) {
            double s = getScaleFactor(), scaleBefore = s;
            gesturePinch(true);
            QTRY_VERIFY2(getScaleFactor(&s) > scaleBefore, qPrintable(QString("i: %1, scale: %2").arg(i).arg(s)));
        }
    }
}

void TouchInputTest::buttonClickHandler()
{
    auto buttonCenter = elementGeometry(view.page(), "btn").center();
    makeTouch(buttonCenter);
    QTRY_VERIFY(!page.alerts.isEmpty());
    QCOMPARE(page.alerts.first(), "button clicked!");
    QCOMPARE(page.alerts.size(), 1);
    QEXPECT_FAIL("", "Shouldn't trigger twice due to synthesized mouse events for touch", Continue);
    QTRY_VERIFY_WITH_TIMEOUT(page.alerts.size() == 2, 500);
}

void TouchInputTest::htmlSelectPopup()
{
    auto selectRect = elementGeometry(view.page(), "select");
    makeTouch(selectRect.center());
    QTRY_VERIFY(QApplication::activePopupWidget());
    QCOMPARE(activeElement(), QStringLiteral("select"));

    auto popup = QApplication::activePopupWidget();
    makeTouch(popup->windowHandle(), QPoint(popup->width() / 2, popup->height() / 2));
    QTRY_VERIFY(!QApplication::activePopupWidget());

    QTRY_VERIFY(!page.alerts.isEmpty());
    QCOMPARE(page.alerts.first(), "option changed to: O2");
    QEXPECT_FAIL("", "Shouldn't trigger twice due to synthesized mouse events for touch", Continue);
    QTRY_VERIFY_WITH_TIMEOUT(page.alerts.size() == 2, 500);
}

QTEST_MAIN(TouchInputTest)
#include "tst_touchinput.moc"
