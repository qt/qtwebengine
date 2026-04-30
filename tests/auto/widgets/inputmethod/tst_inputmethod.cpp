// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

#include <QtGui/private/qinputmethod_p.h>
#include <QtGui/qpa/qplatforminputcontext.h>
#include <QtGui/qtextformat.h>
#include <QtTest/qtest.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineCore/qwebenginesettings.h>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QtWidgets/qstyle.h>

#include <util.h>
#include <visualutil.h>

using namespace Qt::StringLiterals;

#define VERIFY_INPUT_VALUE(view, elementId, expect)                                                \
    do {                                                                                           \
        QTRY_COMPARE(evaluateJavaScriptSync(                                                       \
                             view.page(), u"document.getElementById('%1').value"_s.arg(elementId)) \
                             .toString(),                                                          \
                     expect);                                                                      \
    } while (false)

#define VERIFY_INPUT_METHOD_HINTS(view, expect)                                                    \
    do {                                                                                           \
        QCOMPARE(view.focusProxy()->inputMethodHints(),                                            \
                 (expect | Qt::ImhNoPredictiveText | Qt::ImhNoTextHandles | Qt::ImhNoEditMenu));   \
    } while (false)

#define VERIFY_VIRTUAL_KEYBOARD_ENABLED(view)                                                      \
    do {                                                                                           \
        QTRY_VERIFY(view.focusProxy()->inputMethodQuery(Qt::ImEnabled).toBool());                  \
    } while (false)

#define VERIFY_VIRTUAL_KEYBOARD_DISABLED(view)                                                     \
    do {                                                                                           \
        QTRY_VERIFY(!view.focusProxy()->inputMethodQuery(Qt::ImEnabled).toBool());                 \
    } while (false)

#define CLICK_INPUT_TO_FOCUS(view, elementId)                                                      \
    do {                                                                                           \
        QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {},                                   \
                          elementCenter(view.page(), elementId));                                  \
        QTRY_COMPARE(                                                                              \
                evaluateJavaScriptSync(view.page(), u"document.activeElement.id"_s).toString(),    \
                elementId);                                                                        \
    } while (false)

#define CLEAR_INPUT(view, elementId)                                                               \
    do {                                                                                           \
        evaluateJavaScriptSync(view.page(),                                                        \
                               u"document.getElementById('%1').value = ''"_s.arg(elementId));      \
        QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),        \
                     u""_s);                                                                       \
    } while (false)

class tst_InputMethod : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void microFocusCoordinates();
    void inputTypes_data();
    void inputTypes();
    void inputTypePassword();
    void inputModes_data();
    void inputModes();
    void inputModeNone();
    void inputMethodsTextFormat_data();
    void inputMethodsTextFormat();
    void softwareInputPanel();
    void inputContextQueryInput();
    void inputMethods();
    void textSelectionInInputField();
    void textSelectionOutOfInputField();
    void emptyInputMethodEvent();
    void imeComposition();
    void imeCompositionQueryEvent_data();
    void imeCompositionQueryEvent();
    void newlineInTextarea();
    void imeJSInputEvents();
};

void tst_InputMethod::initTestCase() { }
void tst_InputMethod::cleanupTestCase() { }
void tst_InputMethod::init() { }
void tst_InputMethod::cleanup() { }

void tst_InputMethod::microFocusCoordinates()
{
    QWebEngineView view;
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy scrollSpy(view.page(), SIGNAL(scrollPositionChanged(QPointF)));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.page()->setHtml(u"<html><body>"
                         "<input type='text' id='input' value='' maxlength='20'/><br>"
                         "<canvas id='canvas1' width='500' height='500'></canvas>"
                         "<input type='password'/><br>"
                         "<canvas id='canvas2' width='500' height='500'></canvas>"
                         "</body></html>"_s);
    QVERIFY(loadFinishedSpy.wait());
    CLICK_INPUT_TO_FOCUS(view, u"input"_s);

    QTRY_VERIFY(view.focusProxy()->inputMethodQuery(Qt::ImCursorRectangle).isValid());
    QVariant initialMicroFocus = view.focusProxy()->inputMethodQuery(Qt::ImCursorRectangle);

    evaluateJavaScriptSync(view.page(), u"window.scrollBy(0, 50)"_s);
    QTRY_VERIFY(scrollSpy.size() > 0);

    QTRY_VERIFY(view.focusProxy()->inputMethodQuery(Qt::ImCursorRectangle).isValid());
    QVariant currentMicroFocus = view.focusProxy()->inputMethodQuery(Qt::ImCursorRectangle);

    QCOMPARE(initialMicroFocus.toRect().translated(QPoint(0,-50)), currentMicroFocus.toRect());
}

// clang-format off
void tst_InputMethod::inputTypes_data()
{
    QTest::addColumn<QString>("id");
    QTest::addColumn<Qt::InputMethodHints>("hints");
    QTest::newRow("text")
            << u"textInput"_s
            << Qt::InputMethodHints({Qt::ImhPreferLowercase});
    QTest::newRow("password")
            << u"passwordInput"_s
            << Qt::InputMethodHints({Qt::ImhSensitiveData, Qt::ImhNoAutoUppercase,
                                     Qt::ImhHiddenText});
    QTest::newRow("search")
            << u"searchInput"_s
            << Qt::InputMethodHints({Qt::ImhPreferLowercase, Qt::ImhNoAutoUppercase});
    QTest::newRow("email")
            << u"emailInput"_s
            << Qt::InputMethodHints({Qt::ImhEmailCharactersOnly});
    QTest::newRow("number")
            << u"numberInput"_s
            << Qt::InputMethodHints({Qt::ImhFormattedNumbersOnly});
    QTest::newRow("tel")
            << u"telInput"_s
            << Qt::InputMethodHints({Qt::ImhDialableCharactersOnly});
    QTest::newRow("url")
            << u"urlInput"_s
            << Qt::InputMethodHints({Qt::ImhUrlCharactersOnly, Qt::ImhNoAutoUppercase});
    QTest::newRow("textarea")
            << u"textArea"_s
            << Qt::InputMethodHints({Qt::ImhMultiLine, Qt::ImhPreferLowercase});
    QTest::newRow("contenteditable")
            << u"contentEditable"_s
            << Qt::InputMethodHints({Qt::ImhMultiLine, Qt::ImhPreferLowercase});
}
// clang-format on

void tst_InputMethod::inputTypes()
{
    QFETCH(QString, id);
    QFETCH(Qt::InputMethodHints, hints);

    QWebEngineView view;
    view.resize(200, 600);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.load(QUrl(u"qrc:///resources/input_types.html"_s));
    QVERIFY(loadFinishedSpy.wait());

    CLICK_INPUT_TO_FOCUS(view, id);
    VERIFY_INPUT_METHOD_HINTS(view, hints);
    VERIFY_VIRTUAL_KEYBOARD_ENABLED(view);
}

void tst_InputMethod::inputTypePassword()
{
    const QPlatformInputContext *platformInputContext =
            QGuiApplicationPrivate::platformIntegration()->inputContext();
    const bool imeHasHiddenTextCapability = platformInputContext
            && platformInputContext->hasCapability(QPlatformInputContext::HiddenTextCapability);

    QWebEngineView view;
    view.resize(200, 600);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.load(QUrl(u"qrc:///resources/input_types.html"_s));
    QVERIFY(loadFinishedSpy.wait());

    // Test password field.
    CLICK_INPUT_TO_FOCUS(view, u"passwordInput"_s);
    QVERIFY(view.focusProxy()->inputMethodHints() & Qt::ImhHiddenText);
    VERIFY_VIRTUAL_KEYBOARD_ENABLED(view);
    QVERIFY(!view.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_COMPARE(platformInputContext->inputMethodAccepted(), imeHasHiddenTextCapability);

    // Test if attributes update after focus change.
    CLICK_INPUT_TO_FOCUS(view, u"textInput"_s);
    QVERIFY(!(view.focusProxy()->inputMethodHints() & Qt::ImhHiddenText));
    VERIFY_VIRTUAL_KEYBOARD_ENABLED(view);
    QVERIFY(view.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));

    QTRY_VERIFY(platformInputContext->inputMethodAccepted());

    // Test password field again after re-focus.
    CLICK_INPUT_TO_FOCUS(view, u"passwordInput"_s);
    QVERIFY(view.focusProxy()->inputMethodHints() & Qt::ImhHiddenText);
    VERIFY_VIRTUAL_KEYBOARD_ENABLED(view);
    QVERIFY(!view.focusProxy()->testAttribute(Qt::WA_InputMethodEnabled));
    QTRY_COMPARE(platformInputContext->inputMethodAccepted(), imeHasHiddenTextCapability);
}

// clang-format off
void tst_InputMethod::inputModes_data()
{
    QTest::addColumn<QString>("id");
    QTest::addColumn<Qt::InputMethodHints>("hints");
    QTest::newRow("text")
            << u"textMode"_s
            << Qt::InputMethodHints({Qt::ImhPreferLowercase});
    QTest::newRow("tel")
            << u"telMode"_s
            << Qt::InputMethodHints({Qt::ImhDialableCharactersOnly});
    QTest::newRow("url")
            << u"urlMode"_s
            << Qt::InputMethodHints({Qt::ImhUrlCharactersOnly, Qt::ImhNoAutoUppercase});
    QTest::newRow("email")
            << u"emailMode"_s
            << Qt::InputMethodHints({Qt::ImhEmailCharactersOnly});
    QTest::newRow("numeric")
            << u"numericMode"_s
            << Qt::InputMethodHints({Qt::ImhDigitsOnly});
    QTest::newRow("decimal")
            << u"decimalMode"_s
            << Qt::InputMethodHints({Qt::ImhFormattedNumbersOnly});
    QTest::newRow("search")
            << u"searchMode"_s
            << Qt::InputMethodHints({Qt::ImhPreferLowercase, Qt::ImhNoAutoUppercase});
    // inputmode=numeric overrides <input type="text">
    QTest::newRow("override")
            << u"override"_s
            << Qt::InputMethodHints({Qt::ImhDigitsOnly});
}
// clang-format on

void tst_InputMethod::inputModes()
{
    QFETCH(QString, id);
    QFETCH(Qt::InputMethodHints, hints);

    QWebEngineView view;
    view.resize(200, 600);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.load(QUrl(u"qrc:///resources/input_modes.html"_s));
    QVERIFY(loadFinishedSpy.wait());

    CLICK_INPUT_TO_FOCUS(view, id);
    VERIFY_INPUT_METHOD_HINTS(view, hints);
    VERIFY_VIRTUAL_KEYBOARD_ENABLED(view);
}

void tst_InputMethod::inputModeNone()
{
    QWebEngineView view;
    view.resize(200, 600);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.load(QUrl(u"qrc:///resources/input_modes.html"_s));
    QVERIFY(loadFinishedSpy.wait());

    // Should not trigger the virtual keyboard.
    CLICK_INPUT_TO_FOCUS(view, u"noneMode"_s);
    VERIFY_VIRTUAL_KEYBOARD_DISABLED(view);

    // Trigger the virtual keyboard
    CLICK_INPUT_TO_FOCUS(view, u"textMode"_s);
    VERIFY_VIRTUAL_KEYBOARD_ENABLED(view);

    // Should hide the virtual keyboard.
    CLICK_INPUT_TO_FOCUS(view, u"noneMode"_s);
    VERIFY_VIRTUAL_KEYBOARD_DISABLED(view);
}

// clang-format off
void tst_InputMethod::inputMethodsTextFormat_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("length");
    QTest::addColumn<QTextCharFormat::UnderlineStyle>("underlineStyle");
    QTest::addColumn<QColor>("underlineColor");
    QTest::addColumn<QColor>("backgroundColor");

    // Tag format: s<start>l<length><s|n><r|g|n><r|g|n>
    //  - underlineStyle: s = SingleUnderline, n = NoUnderline
    //  - underlineColor: r = red, g = green, n = none
    //  - backgroundColor: r = red, g = green, n = none
    QTest::newRow("empty s0l0srn") << u""_s << 0 << 0
                                   << QTextCharFormat::SingleUnderline
                                   << QColor(Qt::red) << QColor();
    QTest::newRow("Q s0l1srn") << u"Q"_s << 0 << 1
                               << QTextCharFormat::SingleUnderline
                               << QColor(Qt::red) << QColor();
    QTest::newRow("Qt s0l1srn") << u"Qt"_s << 0 << 1
                                << QTextCharFormat::SingleUnderline
                                << QColor(Qt::red) << QColor();
    QTest::newRow("Qt s0l2srn") << u"Qt"_s << 0 << 2
                                << QTextCharFormat::SingleUnderline
                                << QColor(Qt::red) << QColor();
    QTest::newRow("Qt s1l1srn") << u"Qt"_s << 1 << 1
                                << QTextCharFormat::SingleUnderline
                                << QColor(Qt::red) << QColor();
    QTest::newRow("Qt_ s0l1srn") << u"Qt "_s << 0 << 1
                                 << QTextCharFormat::SingleUnderline
                                 << QColor(Qt::red) << QColor();
    QTest::newRow("Qt_ s1l1srn") << u"Qt "_s << 1 << 1
                                 << QTextCharFormat::SingleUnderline
                                 << QColor(Qt::red) << QColor();
    QTest::newRow("Qt_ s2l1srn") << u"Qt "_s << 2 << 1
                                 << QTextCharFormat::SingleUnderline
                                 << QColor(Qt::red) << QColor();
    QTest::newRow("Qt_ s2l-1srn") << u"Qt "_s << 2 << -1
                                  << QTextCharFormat::SingleUnderline
                                  << QColor(Qt::red) << QColor();
    QTest::newRow("Qt_ s-2l3srn") << u"Qt "_s << -2 << 3
                                  << QTextCharFormat::SingleUnderline
                                  << QColor(Qt::red) << QColor();
    QTest::newRow("Qt_ s-1l-1srn") << u"Qt "_s << -1 << -1
                                   << QTextCharFormat::SingleUnderline
                                   << QColor(Qt::red) << QColor();
    QTest::newRow("Qt_ s0l3srn") << u"Qt "_s << 0 << 3
                                 << QTextCharFormat::SingleUnderline
                                 << QColor(Qt::red) << QColor();
    QTest::newRow("The_Qt s0l1srn") << u"The Qt"_s << 0 << 1
                                    << QTextCharFormat::SingleUnderline
                                    << QColor(Qt::red) << QColor();
    QTest::newRow("The_Qt_Company s0l1srn") << u"The Qt Company"_s << 0 << 1
                                            << QTextCharFormat::SingleUnderline
                                            << QColor(Qt::red) << QColor();
    QTest::newRow("The_Qt_Company s0l3sgn") << u"The Qt Company"_s << 0 << 3
                                            << QTextCharFormat::SingleUnderline
                                            << QColor(Qt::green) << QColor();
    QTest::newRow("The_Qt_Company s4l2sgr") << u"The Qt Company"_s << 4 << 2
                                            << QTextCharFormat::SingleUnderline
                                            << QColor(Qt::green) << QColor(Qt::red);
    QTest::newRow("The_Qt_Company s7l7ngr") << u"The Qt Company"_s << 7 << 7
                                            << QTextCharFormat::NoUnderline
                                            << QColor(Qt::green) << QColor(Qt::red);
    QTest::newRow("The_Qt_Company s7l7nnr") << u"The Qt Company"_s << 7 << 7
                                            << QTextCharFormat::NoUnderline
                                            << QColor() << QColor(Qt::red);
}
// clang-format on

void tst_InputMethod::inputMethodsTextFormat()
{
    QWebEnginePage page;
    QWebEngineView view(&page);
    page.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));

    page.setHtml(
            u"<html><body>"
            " <input type='text' id='input' style='font-family: serif' value='' maxlength='20'/>"
            "</body></html>"_s);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 10000);

    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    evaluateJavaScriptSync(&page, u"document.getElementById('input').focus()"_s);

    QFETCH(QString, string);
    QFETCH(int, start);
    QFETCH(int, length);
    QFETCH(QTextCharFormat::UnderlineStyle, underlineStyle);
    QFETCH(QColor, underlineColor);
    QFETCH(QColor, backgroundColor);

    QTextCharFormat format;
    format.setUnderlineStyle(static_cast<QTextCharFormat::UnderlineStyle>(underlineStyle));
    format.setUnderlineColor(underlineColor);

    // Setting background color is disabled for Qt WebEngine because some IME manager
    // sets background color to black and there is no API for setting the foreground color.
    // This may result black text on black background. However, we still test it to ensure
    // changing background color doesn't cause any crash.
    if (backgroundColor.isValid())
        format.setBackground(QBrush(backgroundColor));

    QList<QInputMethodEvent::Attribute> attributes;
    attributes.append(
            QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, start, length, format));

    QInputMethodEvent im(string, attributes);
    QApplication::sendEvent(view.focusProxy(), &im);

    if (QSysInfo::productType() == QLatin1String("rhel") && QSysInfo::productVersion() == QLatin1String("10.0"))
        QSKIP("Fails on RHEL 10 Wayland and flaky with Ubuntu 24.04 Wayland. Therefore QSKIP RHEL 10.0 is used as we cannot blacklist Wayland or rhel in general - QTBUG-144178");

    QTRY_COMPARE_WITH_TIMEOUT(
            evaluateJavaScriptSync(&page, u"document.getElementById('input').value"_s).toString(),
            string, 20000);
}

class TestInputContext : public QPlatformInputContext
{
public:
    struct InputMethodInfo
    {
        int cursorPosition;
        int anchorPosition;
        QString surroundingText;
        QString selectedText;
    };

    TestInputContext() : m_visible(false)
    {
        QInputMethodPrivate *inputMethodPrivate = QInputMethodPrivate::get(qApp->inputMethod());
        inputMethodPrivate->testContext = this;
    }

    ~TestInputContext()
    {
        QInputMethodPrivate *inputMethodPrivate = QInputMethodPrivate::get(qApp->inputMethod());
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

        infos.append({ cursorPosition, anchorPosition, surroundingText, selectedText });
    }

    bool m_visible;
    QList<InputMethodInfo> infos;
};

void tst_InputMethod::softwareInputPanel()
{
    TestInputContext testContext;
    QWebEngineView view;
    view.resize(640, 480);
    view.show();

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(u"<html><body>"
                 "  <input type='text' id='input' value='' size='50'/>"
                 "</body></html>"_s);
    QVERIFY(loadFinishedSpy.wait());
    CLICK_INPUT_TO_FOCUS(view, u"input"_s);

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

    CLICK_INPUT_TO_FOCUS(view, u"input"_s);
    QTRY_VERIFY(testContext.isInputPanelVisible());

    view.setHtml(u"<html><body><p id='para'>nothing to input here</p></body></html>"_s);
    QVERIFY(loadFinishedSpy.wait());
    testContext.hideInputPanel();

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, elementCenter(view.page(), u"para"_s));
    QVERIFY(!testContext.isInputPanelVisible());

    // Check sending RequestSoftwareInputPanel event
    view.page()->setHtml(
            u"<html><body>"
            "  <input type='text' id='input' value='QtWebEngine inputMethod'/>"
            "  <div id='btnDiv' onclick='i=document.getElementById(&quot;input&quot;); "
            "i.focus();'>abc</div>"
            "</body></html>"_s);
    QVERIFY(loadFinishedSpy.wait());

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {},
                      elementCenter(view.page(), u"btnDiv"_s));
    QVERIFY(!testContext.isInputPanelVisible());
}

void tst_InputMethod::inputContextQueryInput()
{
    SKIP_IF_NO_WINDOW_ACTIVATION();

    QWebEngineView view;
    view.resize(640, 480);
    view.show();
    view.window()->windowHandle()->requestActivate();

    // testContext will be destroyed before the view, so no events are sent accidentally
    // when the view is destroyed.
    TestInputContext testContext;

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(u"<html><body>"
                 "  <input type='text' id='input' value='' size='50'/>"
                 "</body></html>"_s);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 10000);
    QVERIFY(QTest::qWaitForWindowActive(&view));
    QCOMPARE(testContext.infos.size(), 0);

    // Set focus on an input field.
    CLICK_INPUT_TO_FOCUS(view, u"input"_s);
    QTRY_COMPARE(testContext.infos.size(), 2);
    for (const auto &info : std::as_const(testContext.infos)) {
        QCOMPARE(info.cursorPosition, 0);
        QCOMPARE(info.anchorPosition, 0);
        QCOMPARE(info.surroundingText, u""_s);
        QCOMPARE(info.selectedText, u""_s);
    }
    testContext.infos.clear();

    // Change content of an input field from JavaScript.
    evaluateJavaScriptSync(view.page(), u"document.getElementById('input').value='QtWebEngine';"_s);
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 11);
    QCOMPARE(testContext.infos[0].anchorPosition, 11);
    QCOMPARE(testContext.infos[0].surroundingText, u"QtWebEngine"_s);
    QCOMPARE(testContext.infos[0].selectedText, u""_s);
    testContext.infos.clear();

    // Change content of an input field by key press.
    QTest::keyClick(view.focusProxy(), Qt::Key_Exclam);
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 12);
    QCOMPARE(testContext.infos[0].anchorPosition, 12);
    QCOMPARE(testContext.infos[0].surroundingText, u"QtWebEngine!"_s);
    QCOMPARE(testContext.infos[0].selectedText, u""_s);
    testContext.infos.clear();

    // Change cursor position.
    QTest::keyClick(view.focusProxy(), Qt::Key_Left);
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 11);
    QCOMPARE(testContext.infos[0].anchorPosition, 11);
    QCOMPARE(testContext.infos[0].surroundingText, u"QtWebEngine!"_s);
    QCOMPARE(testContext.infos[0].selectedText, u""_s);
    testContext.infos.clear();

    // Selection by IME.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent::Attribute newSelection(QInputMethodEvent::Selection, 2, 12, QVariant());
        attributes.append(newSelection);
        QInputMethodEvent event(u""_s, attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 2);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);

    // As a first step, Chromium moves the cursor to the start of the selection.
    // We don't filter this in QtWebEngine because we don't know yet if this is part of a selection.
    QCOMPARE(testContext.infos[0].cursorPosition, 2);
    QCOMPARE(testContext.infos[0].anchorPosition, 2);
    QCOMPARE(testContext.infos[0].surroundingText, u"QtWebEngine!"_s);
    QCOMPARE(testContext.infos[0].selectedText, u""_s);

    // The update of the selection.
    QCOMPARE(testContext.infos[1].cursorPosition, 12);
    QCOMPARE(testContext.infos[1].anchorPosition, 2);
    QCOMPARE(testContext.infos[1].surroundingText, u"QtWebEngine!"_s);
    QCOMPARE(testContext.infos[1].selectedText, u"WebEngine!"_s);
    testContext.infos.clear();
    selectionChangedSpy.clear();

    // Clear selection by IME.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent::Attribute newSelection(QInputMethodEvent::Selection, 0, 0, QVariant());
        attributes.append(newSelection);
        QInputMethodEvent event(u""_s, attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 1);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 0);
    QCOMPARE(testContext.infos[0].anchorPosition, 0);
    QCOMPARE(testContext.infos[0].surroundingText, u"QtWebEngine!"_s);
    QCOMPARE(testContext.infos[0].selectedText, u""_s);
    testContext.infos.clear();
    selectionChangedSpy.clear();

    // Compose text.
    {
        QInputMethodEvent event(u"123"_s, {});
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 0);
    QCOMPARE(testContext.infos[0].anchorPosition, 0);
    QCOMPARE(testContext.infos[0].surroundingText, u"QtWebEngine!"_s);
    QCOMPARE(testContext.infos[0].selectedText, u""_s);
    VERIFY_INPUT_VALUE(view, u"input"_s, u"123QtWebEngine!"_s);
    testContext.infos.clear();

    // Cancel composition.
    {
        QInputMethodEvent event(u""_s, {});
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 2);
    for (const auto &info : std::as_const(testContext.infos)) {
        QCOMPARE(info.cursorPosition, 0);
        QCOMPARE(info.anchorPosition, 0);
        QCOMPARE(info.surroundingText, u"QtWebEngine!"_s);
        QCOMPARE(info.selectedText, u""_s);
    }
    VERIFY_INPUT_VALUE(view, u"input"_s, u"QtWebEngine!"_s);
    testContext.infos.clear();

    // Commit text.
    {
        QInputMethodEvent event(u""_s, {});
        event.setCommitString(u"123"_s, 0, 0);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(testContext.infos.size(), 1);
    QCOMPARE(testContext.infos[0].cursorPosition, 3);
    QCOMPARE(testContext.infos[0].anchorPosition, 3);
    QCOMPARE(testContext.infos[0].surroundingText, u"123QtWebEngine!"_s);
    QCOMPARE(testContext.infos[0].selectedText, u""_s);
    VERIFY_INPUT_VALUE(view, u"input"_s, u"123QtWebEngine!"_s);
    testContext.infos.clear();

    // Focus out.
    QTest::keyPress(view.focusProxy(), Qt::Key_Tab);
    QTRY_COMPARE(testContext.infos.size(), 1);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), u"document.activeElement.id"_s).toString(),
                 u""_s);
    testContext.infos.clear();
}

void tst_InputMethod::inputMethods()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.settings()->setFontFamily(QWebEngineSettings::SerifFont,
                                   view.settings()->fontFamily(QWebEngineSettings::FixedFont));
    view.setHtml(u"<html><body>"
                 "  <input type='text' id='input' style='font-family: serif' value='' "
                 "maxlength='20' size='50'/>"
                 "</body></html>"_s);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 10000);
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    CLICK_INPUT_TO_FOCUS(view, u"input"_s);

    // ImCursorRectangle
    QVariant variant = view.focusProxy()->inputMethodQuery(Qt::ImCursorRectangle);
    QVERIFY(elementGeometry(view.page(), u"input"_s).contains(variant.toRect().topLeft()));

    // We assigned the serif font family to be the same as the fixed font family.
    // Then test ImFont on a serif styled element, we should get our fixed font family.
    variant = view.focusProxy()->inputMethodQuery(Qt::ImFont);
    QFont font = variant.value<QFont>();
    QEXPECT_FAIL("", "UNIMPLEMENTED: RenderWidgetHostViewQt::inputMethodQuery(Qt::ImFont)",
                 Continue);
    QCOMPARE(view.settings()->fontFamily(QWebEngineSettings::FixedFont), font.family());

    // Insert text
    {
        QString text = u"QtWebEngine"_s;
        QInputMethodEvent eventText(text, {});
        QApplication::sendEvent(view.focusProxy(), &eventText);
        VERIFY_INPUT_VALUE(view, u"input"_s, text);
        QCOMPARE(selectionChangedSpy.size(), 0);
    }

    {
        QString text = u"QtWebEngine"_s;
        QInputMethodEvent eventText(u""_s, {});
        eventText.setCommitString(text, 0, 0);
        QApplication::sendEvent(view.focusProxy(), &eventText);
        VERIFY_INPUT_VALUE(view, u"input"_s, text);
        QCOMPARE(selectionChangedSpy.size(), 0);
    }

    // ImMaximumTextLength
    QEXPECT_FAIL("",
                 "UNIMPLEMENTED: RenderWidgetHostViewQt::inputMethodQuery(Qt::ImMaximumTextLength)",
                 Continue);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImMaximumTextLength).toInt(), 20);

    // Set selection
    {
        QList<QInputMethodEvent::Attribute> attributes;
        attributes << QInputMethodEvent::Attribute(QInputMethodEvent::Selection, 3, 2, QVariant());
        QInputMethodEvent eventSelection(u""_s, attributes);
        QApplication::sendEvent(view.focusProxy(), &eventSelection);
        QTRY_COMPARE(selectionChangedSpy.size(), 1);

        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 3);
        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 5);
        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u"eb"_s);
        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"QtWebEngine"_s);
    }

    // Set selection with negative length
    {
        QList<QInputMethodEvent::Attribute> attributes;
        attributes << QInputMethodEvent::Attribute(QInputMethodEvent::Selection, 6, -5, QVariant());
        QInputMethodEvent eventSelection(u""_s, attributes);
        QApplication::sendEvent(view.focusProxy(), &eventSelection);
        QTRY_COMPARE(selectionChangedSpy.size(), 2);

        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 1);
        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 6);
        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(),
                 u"tWebE"_s);
        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"QtWebEngine"_s);
    }

    // Clear the selection, so the next test does not clear any contents.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        attributes << QInputMethodEvent::Attribute(QInputMethodEvent::Selection, 0, 0, QVariant());
        QInputMethodEvent eventComposition(u"composition"_s, attributes);
        QApplication::sendEvent(view.focusProxy(), &eventComposition);
        QTRY_COMPARE(selectionChangedSpy.size(), 3);

        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
        // An ongoing composition should not change the surrounding text before it is committed.
        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"QtWebEngine"_s);
    }

    // Cancel current composition.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        attributes << QInputMethodEvent::Attribute(QInputMethodEvent::Selection, 0, 0, QVariant());
        QInputMethodEvent eventSelection(u""_s, attributes);
        QApplication::sendEvent(view.focusProxy(), &eventSelection);

        // Cancelling composition should not clear the surrounding text.
        QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"QtWebEngine"_s);
    }
}

void tst_InputMethod::textSelectionInInputField()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(u"<html><body>"
                 "  <input type='text' id='input' value='QtWebEngine' size='50'/>"
                 "</body></html>"_s);
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    // Tests for Selection when the Editor is NOT in Composition mode

    // LEFT to RIGHT selection
    // Mouse click event moves the current cursor to the end of the text
    CLICK_INPUT_TO_FOCUS(view, u"input"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 11);
    // There was no selection to be changed by the click
    QCOMPARE(selectionChangedSpy.size(), 0);

    QInputMethodEvent event(u""_s, {});
    event.setCommitString(u"XXX"_s, 0, 0);
    QApplication::sendEvent(view.focusProxy(), &event);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"QtWebEngineXXX"_s);
    QCOMPARE(selectionChangedSpy.size(), 0);

    event.setCommitString(u""_s, -2, 2); // Erase two characters.
    QApplication::sendEvent(view.focusProxy(), &event);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"QtWebEngineX"_s);
    QCOMPARE(selectionChangedSpy.size(), 0);

    event.setCommitString(u""_s, -1, 1); // Erase one character.
    QApplication::sendEvent(view.focusProxy(), &event);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"QtWebEngine"_s);
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
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(),
             u"WebEngine"_s);

    // RIGHT to LEFT selection
    // Deselect the selection (this moves the current cursor to the end of the text)
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {},
                      elementCenter(view.page(), u"input"_s));
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 2);

    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);

    // Move 2 characters LEFT
    for (int i = 0; i < 2; ++i)
        QTest::keyClick(view.focusProxy(), Qt::Key_Left);

    // Select to the start of the line
    QTest::keyClick(view.focusProxy(), Qt::Key_Home, Qt::ShiftModifier);
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 3);

    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 9);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(),
             u"QtWebEngi"_s);
}

void tst_InputMethod::textSelectionOutOfInputField()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(u"<html><body>"
                 "  This is a text"
                 "</body></html>"_s);
    QTRY_VERIFY_WITH_TIMEOUT(loadFinishedSpy.size(), 10000);
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QCOMPARE(selectionChangedSpy.size(), 0);
    QVERIFY(!view.hasSelection());
    QCOMPARE(view.page()->selectedText(), u""_s);

    // Simple click should not update text selection, however it updates selection bounds in
    // Chromium
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, view.geometry().center());
    QCOMPARE(selectionChangedSpy.size(), 0);
    QVERIFY(!view.hasSelection());
    QCOMPARE(view.page()->selectedText(), u""_s);

    // Select text by ctrl+a
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QVERIFY(view.hasSelection());
    QCOMPARE(view.page()->selectedText(), u"This is a text"_s);

    // Deselect text by mouse click
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, view.geometry().center());
    QTRY_COMPARE(selectionChangedSpy.size(), 2);
    QVERIFY(!view.hasSelection());
    QCOMPARE(view.page()->selectedText(), u""_s);

    // Select text by ctrl+a
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QTRY_COMPARE(selectionChangedSpy.size(), 3);
    QVERIFY(view.hasSelection());
    QCOMPARE(view.page()->selectedText(), u"This is a text"_s);

    // Deselect text via discard+undiscard
    view.hide();
    view.page()->setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    view.show();
    QVERIFY(loadFinishedSpy.wait());
    QTRY_COMPARE(selectionChangedSpy.size(), 4);
    QVERIFY(!view.hasSelection());
    QCOMPARE(view.page()->selectedText(), u""_s);

    selectionChangedSpy.clear();
    view.setHtml(u"<html><body>"
                 "  This is a text"
                 "  <br>"
                 "  <input type='text' id='input' value='QtWebEngine' size='50'/>"
                 "</body></html>"_s);
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QCOMPARE(selectionChangedSpy.size(), 0);
    QVERIFY(!view.hasSelection());
    QCOMPARE(view.page()->selectedText(), u""_s);

    // Make sure the input field does not have the focus
    evaluateJavaScriptSync(view.page(), u"document.getElementById('input').blur()"_s);
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), u"document.activeElement.id"_s).toString(),
                 u""_s);

    // Select the whole page by ctrl+a
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QVERIFY(view.hasSelection());
    QVERIFY(view.page()->selectedText().startsWith("This is a text"_L1));

    // Remove selection by clicking into an input field
    CLICK_INPUT_TO_FOCUS(view, u"input"_s);
    QTRY_COMPARE(selectionChangedSpy.size(), 2);
    QVERIFY(!view.hasSelection());
    QCOMPARE(view.page()->selectedText(), u""_s);

    // Select the content of the input field by ctrl+a
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QTRY_COMPARE(selectionChangedSpy.size(), 3);
    QVERIFY(view.hasSelection());
    QCOMPARE(view.page()->selectedText(), u"QtWebEngine"_s);

    // Deselect input field's text by mouse click
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, view.geometry().center());
    QTRY_COMPARE(selectionChangedSpy.size(), 4);
    QVERIFY(!view.hasSelection());
    QCOMPARE(view.page()->selectedText(), u""_s);
}

void tst_InputMethod::emptyInputMethodEvent()
{
    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(u"<html><body>"
                 "  <input type='text' id='input' value='QtWebEngine'/>"
                 "</body></html>"_s);
    QTRY_VERIFY_WITH_TIMEOUT(loadFinishedSpy.size(), 10000);
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    evaluateJavaScriptSync(view.page(),
                           u"var element = document.getElementById('input');"
                           "element.focus();"
                           "element.select();"_s);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);

    // 1. Empty input method event does not clear text
    QInputMethodEvent emptyEvent;
    QVERIFY(QApplication::sendEvent(view.focusProxy(), &emptyEvent));
    qApp->processEvents();
    VERIFY_INPUT_VALUE(view, u"input"_s, u"QtWebEngine"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"QtWebEngine"_s);

    // Reset: clear input field
    CLEAR_INPUT(view, u"input"_s);

    // 2. Cancel IME composition with empty input method event
    // Start IME composition
    QInputMethodEvent eventComposition(u"a"_s, {});
    QVERIFY(QApplication::sendEvent(view.focusProxy(), &eventComposition));
    VERIFY_INPUT_VALUE(view, u"input"_s, u"a"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), u""_s);

    // Cancel IME composition
    QVERIFY(QApplication::sendEvent(view.focusProxy(), &emptyEvent));
    VERIFY_INPUT_VALUE(view, u"input"_s, u""_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), u""_s);

    // Try key press after cancelled IME composition
    QTest::keyClick(view.focusProxy(), Qt::Key_B);
    VERIFY_INPUT_VALUE(view, u"input"_s, u"b"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), u"b"_s);
}

void tst_InputMethod::imeComposition()
{
    SKIP_IF_NO_WINDOW_ACTIVATION();

    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();
    view.window()->windowHandle()->requestActivate();

    QSignalSpy selectionChangedSpy(&view, SIGNAL(selectionChanged()));
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(u"<html><body>"
                 "  <input type='text' id='input' value='QtWebEngine inputMethod'/>"
                 "</body></html>"_s);
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowActive(&view));

    evaluateJavaScriptSync(view.page(),
                           u"var element = document.getElementById('input');"
                           "element.focus();"
                           "element.select();"_s);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);

    // Clear the selection, also cancel the ongoing composition if there is one.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent::Attribute newSelection(QInputMethodEvent::Selection, 0, 0, QVariant());
        attributes.append(newSelection);
        QInputMethodEvent event(u""_s, attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
        selectionChangedSpy.wait();
        QCOMPARE(selectionChangedSpy.size(), 2);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
             u"QtWebEngine inputMethod"_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);

    selectionChangedSpy.clear();

    // 1. Insert a character to the beginning of the line.
    // Send temporary text, which makes the editor has composition 'm'.
    {
        QInputMethodEvent event("m", {});
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
             u"QtWebEngine inputMethod"_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
    QCOMPARE(selectionChangedSpy.size(), 0);

    // Send temporary text, which makes the editor has composition 'n'.
    {
        QInputMethodEvent event(u"n"_s, {});
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
             u"QtWebEngine inputMethod"_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
    QCOMPARE(selectionChangedSpy.size(), 0);

    // Send commit text, which makes the editor conforms composition.
    {
        QInputMethodEvent event(u""_s, {});
        event.setCommitString(u"o"_s);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"oQtWebEngine inputMethod"_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 1);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 1);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
    QCOMPARE(selectionChangedSpy.size(), 0);

    // 2. insert a character to the middle of the line.
    // Send temporary text, which makes the editor has composition 'd'.
    {
        QInputMethodEvent event(u"d"_s, {});
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
             u"oQtWebEngine inputMethod"_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 1);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 1);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
    QCOMPARE(selectionChangedSpy.size(), 0);

    // Send commit text, which makes the editor conforms composition.
    {
        QInputMethodEvent event(u""_s, {});
        event.setCommitString(u"e"_s);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"oeQtWebEngine inputMethod"_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 2);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 2);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
    QCOMPARE(selectionChangedSpy.size(), 0);

    // 3. Insert a character to the end of the line.
    QTest::keyClick(view.focusProxy(), Qt::Key_End);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 25);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 25);

    // Send temporary text, which makes the editor has composition 't'.
    {
        QInputMethodEvent event(u"t"_s, {});
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
             u"oeQtWebEngine inputMethod"_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 25);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 25);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
    QCOMPARE(selectionChangedSpy.size(), 0);

    // Send commit text, which makes the editor conforms composition.
    {
        QInputMethodEvent event(u""_s, {});
        event.setCommitString(u"t"_s);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"oeQtWebEngine inputMethodt"_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 26);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 26);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
    QCOMPARE(selectionChangedSpy.size(), 0);

    // 4. Replace the selection.
#ifndef Q_OS_MACOS
    QTest::keyClick(view.focusProxy(), Qt::Key_Left, Qt::ShiftModifier | Qt::ControlModifier);
#else
    QTest::keyClick(view.focusProxy(), Qt::Key_Left, Qt::ShiftModifier | Qt::AltModifier);
#endif
    QVERIFY(selectionChangedSpy.wait());
    QCOMPARE(selectionChangedSpy.size(), 1);

    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
             u"oeQtWebEngine inputMethodt"_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 14);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 26);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(),
             u"inputMethodt"_s);

    // Send temporary text, which makes the editor has composition 'w'.
    {
        QInputMethodEvent event(u"w"_s, {});
        QApplication::sendEvent(view.focusProxy(), &event);
        // The new composition should clear the previous selection
        QVERIFY(selectionChangedSpy.wait());
        QCOMPARE(selectionChangedSpy.size(), 2);
    }
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
             u"oeQtWebEngine "_s);
    // The cursor should be positioned at the end of the composition text
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 15);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 15);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);

    // Send commit text, which makes the editor conforms composition.
    {
        QInputMethodEvent event(u""_s, {});
        event.setCommitString(u"2"_s);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    // There is no text selection to be changed at this point thus we can't wait for
    // selectionChanged signal.
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"oeQtWebEngine 2"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 15);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 15);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
    QCOMPARE(selectionChangedSpy.size(), 2);
    selectionChangedSpy.clear();

    // 5. Mimic behavior of QtVirtualKeyboard with enabled text prediction.
    evaluateJavaScriptSync(view.page(), u"document.getElementById('input').value='QtWebEngine';"_s);
    VERIFY_INPUT_VALUE(view, u"input"_s, u"QtWebEngine"_s);

    // Move cursor into position.
    QTest::keyClick(view.focusProxy(), Qt::Key_Home);
    for (int j = 0; j < 2; ++j)
        QTest::keyClick(view.focusProxy(), Qt::Key_Right);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 2);

    // Turn text into composition by using negative start position.
    {
        int replaceFrom = -1 * view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt();
        int replaceLength =
                view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString().size();

        QInputMethodEvent event(u"QtWebEngine"_s, {});
        event.setCommitString(u""_s, replaceFrom, replaceLength);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(), u""_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
    VERIFY_INPUT_VALUE(view, u"input"_s, u"QtWebEngine"_s);

    // Commit.
    {
        QInputMethodEvent event(u""_s, {});
        event.setCommitString(u"QtWebEngine"_s, 0, 0);
        QApplication::sendEvent(view.focusProxy(), &event);
    }
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"QtWebEngine"_s);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 11);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCurrentSelection).toString(), u""_s);
    VERIFY_INPUT_VALUE(view, u"input"_s, u"QtWebEngine"_s);
    QCOMPARE(selectionChangedSpy.size(), 0);
}

void tst_InputMethod::newlineInTextarea()
{
    SKIP_IF_NO_WINDOW_ACTIVATION();

    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();
    view.window()->windowHandle()->requestActivate();

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.page()->setHtml(u"<html><body>"
                         "  <textarea rows='5' cols='1' id='input'></textarea>"
                         "</body></html>"_s);
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowActive(&view));
    CLICK_INPUT_TO_FOCUS(view, u"input"_s);

    // Enter Key without key text
    CLEAR_INPUT(view, u"input"_s);

    QKeyEvent keyPressEnter(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    QKeyEvent keyReleaseEnter(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);
    QApplication::sendEvent(view.focusProxy(), &keyPressEnter);
    QApplication::sendEvent(view.focusProxy(), &keyReleaseEnter);

    QInputMethodEvent eventText(u""_s, {});
    eventText.setCommitString(u"\n"_s);
    QApplication::sendEvent(view.focusProxy(), &eventText);

    QInputMethodEvent eventText2(u""_s, {});
    eventText2.setCommitString(u"third line"_s);
    QApplication::sendEvent(view.focusProxy(), &eventText2);

    qApp->processEvents();
    VERIFY_INPUT_VALUE(view, u"input"_s, u"\n\nthird line"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"\n\nthird line"_s);

    // Enter Key with key text '\r'
    CLEAR_INPUT(view, u"input"_s);

    QKeyEvent keyPressEnterWithCarriageReturn(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier,
                                              u"\r"_s);
    QKeyEvent keyReleaseEnterWithCarriageReturn(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);
    QApplication::sendEvent(view.focusProxy(), &keyPressEnterWithCarriageReturn);
    QApplication::sendEvent(view.focusProxy(), &keyReleaseEnterWithCarriageReturn);

    QApplication::sendEvent(view.focusProxy(), &eventText);
    QApplication::sendEvent(view.focusProxy(), &eventText2);

    qApp->processEvents();
    VERIFY_INPUT_VALUE(view, u"input"_s, u"\n\nthird line"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"\n\nthird line"_s);

    // Enter Key with key text '\n'
    CLEAR_INPUT(view, u"input"_s);

    QKeyEvent keyPressEnterWithLineFeed(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier, u"\n"_s);
    QKeyEvent keyReleaseEnterWithLineFeed(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier,
                                          u"\n"_s);
    QApplication::sendEvent(view.focusProxy(), &keyPressEnterWithLineFeed);
    QApplication::sendEvent(view.focusProxy(), &keyReleaseEnterWithLineFeed);

    QApplication::sendEvent(view.focusProxy(), &eventText);
    QApplication::sendEvent(view.focusProxy(), &eventText2);

    qApp->processEvents();
    VERIFY_INPUT_VALUE(view, u"input"_s, u"\n\nthird line"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"\n\nthird line"_s);

    // Enter Key with key text "\n\r"
    CLEAR_INPUT(view, u"input"_s);

    QKeyEvent keyPressEnterWithLFCR(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier, u"\n\r"_s);
    QKeyEvent keyReleaseEnterWithLFCR(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier, u"\n\r"_s);
    QApplication::sendEvent(view.focusProxy(), &keyPressEnterWithLFCR);
    QApplication::sendEvent(view.focusProxy(), &keyReleaseEnterWithLFCR);

    QApplication::sendEvent(view.focusProxy(), &eventText);
    QApplication::sendEvent(view.focusProxy(), &eventText2);

    qApp->processEvents();
    VERIFY_INPUT_VALUE(view, u"input"_s, u"\n\nthird line"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"\n\nthird line"_s);

    // Return Key without key text
    CLEAR_INPUT(view, u"input"_s);

    QKeyEvent keyPressReturn(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    QKeyEvent keyReleaseReturn(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);
    QApplication::sendEvent(view.focusProxy(), &keyPressReturn);
    QApplication::sendEvent(view.focusProxy(), &keyReleaseReturn);

    QApplication::sendEvent(view.focusProxy(), &eventText);
    QApplication::sendEvent(view.focusProxy(), &eventText2);

    qApp->processEvents();
    VERIFY_INPUT_VALUE(view, u"input"_s, u"\n\nthird line"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"\n\nthird line"_s);
}

void tst_InputMethod::imeJSInputEvents()
{
#define CLEAR_LOG_AND_INPUT()                                                                      \
    evaluateJavaScriptSync(view.page(), u"log.textContent = ''; input.textContent = '';"_s);

    QWebEngineView view;
    view.resize(640, 480);
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.show();

    auto logLines = [&view](QStringList *log) -> size_t {
        *log = evaluateJavaScriptSync(view.page(), u"log.textContent"_s)
                       .toString()
                       .split(u'\n')
                       .filter(QRegularExpression(u".+"_s));
        return log->size();
    };

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.page()->setHtml(
            u"<html>"
            "<head><script>"
            "  var input, log;"
            "  function verboseEvent(ev) {"
            "      log.textContent += ev + ' ' + ev.type + ' ' + ev.data + '\\n';"
            "  }"
            "  function init() {"
            "      input = document.getElementById('input');"
            "      log = document.getElementById('log');"
            "      events = [ 'textInput', 'beforeinput', 'input', 'compositionstart', "
            "'compositionupdate', 'compositionend' ];"
            "      for (var e in events)"
            "          input.addEventListener(events[e], verboseEvent);"
            "  }"
            "</script></head>"
            "<body onload='init()'>"
            "  <div id='input' contenteditable='true' style='border-style: solid;'></div>"
            "  <pre id='log'></pre>"
            "</body></html>"_s);
    QTRY_VERIFY_WITH_TIMEOUT(loadFinishedSpy.size(), 10000);
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    CLICK_INPUT_TO_FOCUS(view, u"input"_s);
    QStringList log;

    // 1. Commit text (this is how dead keys work on Linux).
    {
        QInputMethodEvent event(u""_s, {});
        event.setCommitString(u"commit"_s);
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    // Simply committing text should not trigger any JS composition event.
    QTRY_COMPARE(logLines(&log), 3);
    QCOMPARE(log[0], u"[object InputEvent] beforeinput commit"_s);
    QCOMPARE(log[1], u"[object TextEvent] textInput commit"_s);
    QCOMPARE(log[2], u"[object InputEvent] input commit"_s);
    CLEAR_LOG_AND_INPUT();

    // 2. Start composition then commit text (this is how dead keys work on macOS).
    {
        QInputMethodEvent event(u"preedit"_s, {});
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(logLines(&log), 4);
    QCOMPARE(log[0], u"[object CompositionEvent] compositionstart "_s);
    QCOMPARE(log[1], u"[object CompositionEvent] compositionupdate preedit"_s);
    QCOMPARE(log[2], u"[object InputEvent] beforeinput preedit"_s);
    QCOMPARE(log[3], u"[object InputEvent] input preedit"_s);

    {
        QInputMethodEvent event(u""_s, {});
        event.setCommitString(u"commit"_s);
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(logLines(&log), 9);
    QCOMPARE(log[4], u"[object CompositionEvent] compositionupdate commit"_s);
    QCOMPARE(log[5], u"[object InputEvent] beforeinput commit"_s);
    QCOMPARE(log[6], u"[object TextEvent] textInput commit"_s);
    QCOMPARE(log[7], u"[object InputEvent] input commit"_s);
    QCOMPARE(log[8], u"[object CompositionEvent] compositionend commit"_s);
    CLEAR_LOG_AND_INPUT();

    // 3. Start composition then cancel it with an empty IME event.
    {
        QInputMethodEvent event(u"preedit"_s, {});
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(logLines(&log), 4);
    QCOMPARE(log[0], u"[object CompositionEvent] compositionstart "_s);
    QCOMPARE(log[1], u"[object CompositionEvent] compositionupdate preedit"_s);
    QCOMPARE(log[2], u"[object InputEvent] beforeinput preedit"_s);
    QCOMPARE(log[3], u"[object InputEvent] input preedit"_s);

    {
        QInputMethodEvent event(u""_s, {});
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(logLines(&log), 9);
    QCOMPARE(log[4], u"[object CompositionEvent] compositionupdate "_s);
    QCOMPARE(log[5], u"[object InputEvent] beforeinput "_s);
    QCOMPARE(log[6], u"[object TextEvent] textInput "_s);
    QCOMPARE(log[7], u"[object InputEvent] input null"_s);
    QCOMPARE(log[8], u"[object CompositionEvent] compositionend "_s);
    CLEAR_LOG_AND_INPUT();

    // 4. Send empty IME event.
    {
        QInputMethodEvent event(u""_s, {});
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    // No JS event is expected.
    QTest::qWait(100);
    QCOMPARE(logLines(&log), 0);

#undef CLEAR_LOG_AND_INPUT
}

void tst_InputMethod::imeCompositionQueryEvent_data()
{
    QTest::addColumn<QString>("receiverObjectName");
    QTest::newRow("focusObject") << u"focusObject"_s;
    QTest::newRow("focusProxy") << u"focusProxy"_s;
    QTest::newRow("focusWidget") << u"focusWidget"_s;
}

void tst_InputMethod::imeCompositionQueryEvent()
{
    SKIP_IF_NO_WINDOW_ACTIVATION();

    QWebEngineView view;
    view.resize(640, 480);
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);

    view.show();
    view.window()->windowHandle()->requestActivate();

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(u"<html><body>"
                 "  <input type='text' id='input' />"
                 "</body></html>"_s);
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowActive(&view));
    CLICK_INPUT_TO_FOCUS(view, u"input"_s);

    QObject *input = nullptr;

    QFETCH(QString, receiverObjectName);
    if (receiverObjectName == "focusObject"_L1) {
        QTRY_VERIFY(qApp->focusObject());
        input = qApp->focusObject();
    } else if (receiverObjectName == "focusProxy"_L1) {
        QTRY_VERIFY(view.focusProxy());
        input = view.focusProxy();
    } else if (receiverObjectName == "focusWidget"_L1) {
        QTRY_VERIFY(view.focusWidget());
        input = view.focusWidget();
    }

    QInputMethodQueryEvent srrndTextQuery(Qt::ImSurroundingText);
    QInputMethodQueryEvent absolutePosQuery(Qt::ImAbsolutePosition);
    QInputMethodQueryEvent cursorPosQuery(Qt::ImCursorPosition);
    QInputMethodQueryEvent anchorPosQuery(Qt::ImAnchorPosition);

    // Set composition
    {
        QInputMethodEvent event(u"composition"_s, {});
        QApplication::sendEvent(input, &event);
        qApp->processEvents();
    }
    VERIFY_INPUT_VALUE(view, u"input"_s, u"composition"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);

    QApplication::sendEvent(input, &srrndTextQuery);
    QApplication::sendEvent(input, &absolutePosQuery);
    QApplication::sendEvent(input, &cursorPosQuery);
    QApplication::sendEvent(input, &anchorPosQuery);
    qApp->processEvents();

    QTRY_COMPARE(srrndTextQuery.value(Qt::ImSurroundingText).toString(), u""_s);
    QTRY_COMPARE(absolutePosQuery.value(Qt::ImAbsolutePosition).toInt(), 0);
    QTRY_COMPARE(cursorPosQuery.value(Qt::ImCursorPosition).toInt(), 0);
    QTRY_COMPARE(anchorPosQuery.value(Qt::ImAnchorPosition).toInt(), 0);

    // Send commit
    {
        QInputMethodEvent event(u""_s, {});
        event.setCommitString(u"composition"_s);
        QApplication::sendEvent(input, &event);
        qApp->processEvents();
    }
    VERIFY_INPUT_VALUE(view, u"input"_s, u"composition"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImSurroundingText).toString(),
                 u"composition"_s);

    QApplication::sendEvent(input, &srrndTextQuery);
    QApplication::sendEvent(input, &absolutePosQuery);
    QApplication::sendEvent(input, &cursorPosQuery);
    QApplication::sendEvent(input, &anchorPosQuery);
    qApp->processEvents();

    QTRY_COMPARE(srrndTextQuery.value(Qt::ImSurroundingText).toString(), u"composition"_s);
    QTRY_COMPARE(absolutePosQuery.value(Qt::ImAbsolutePosition).toInt(), 11);
    QTRY_COMPARE(cursorPosQuery.value(Qt::ImCursorPosition).toInt(), 11);
    QTRY_COMPARE(anchorPosQuery.value(Qt::ImAnchorPosition).toInt(), 11);

    // Test another composition to ensure that the cursor position is set correctly.
    // In this case cursor will be at position 11 during input composition.
    {
        QInputMethodEvent event(u"123"_s, {});
        QApplication::sendEvent(input, &event);
        qApp->processEvents();
    }
    VERIFY_INPUT_VALUE(view, u"input"_s, u"composition123"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);

    QApplication::sendEvent(input, &srrndTextQuery);
    QApplication::sendEvent(input, &absolutePosQuery);
    QApplication::sendEvent(input, &cursorPosQuery);
    QApplication::sendEvent(input, &anchorPosQuery);
    qApp->processEvents();

    QTRY_COMPARE(srrndTextQuery.value(Qt::ImSurroundingText).toString(), u"composition"_s);
    QTRY_COMPARE(absolutePosQuery.value(Qt::ImAbsolutePosition).toInt(), 11);
    QTRY_COMPARE(cursorPosQuery.value(Qt::ImCursorPosition).toInt(), 11);
    QTRY_COMPARE(anchorPosQuery.value(Qt::ImAnchorPosition).toInt(), 11);

    // Send commit
    {
        QInputMethodEvent event(u""_s, {});
        event.setCommitString(u"123"_s);
        QApplication::sendEvent(input, &event);
        qApp->processEvents();
    }

    VERIFY_INPUT_VALUE(view, u"input"_s, u"composition123"_s);
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 14);

    QApplication::sendEvent(input, &srrndTextQuery);
    QApplication::sendEvent(input, &absolutePosQuery);
    QApplication::sendEvent(input, &cursorPosQuery);
    QApplication::sendEvent(input, &anchorPosQuery);
    qApp->processEvents();

    QTRY_COMPARE(srrndTextQuery.value(Qt::ImSurroundingText).toString(), u"composition123"_s);
    QTRY_COMPARE(absolutePosQuery.value(Qt::ImAbsolutePosition).toInt(), 14);
    QTRY_COMPARE(cursorPosQuery.value(Qt::ImCursorPosition).toInt(), 14);
    QTRY_COMPARE(anchorPosQuery.value(Qt::ImAnchorPosition).toInt(), 14);
}

QTEST_MAIN(tst_InputMethod)
#include "tst_inputmethod.moc"
