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

#define VERIFY_INPUTMETHOD_HINTS(actual, expect) \
    QVERIFY(actual == (expect | Qt::ImhNoPredictiveText | Qt::ImhNoTextHandles | Qt::ImhNoEditMenu));

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
    void focusInputTypes();
    void inputModes();
    void inputMethodsTextFormat_data();
    void inputMethodsTextFormat();
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
};

void tst_InputMethod::initTestCase() { }
void tst_InputMethod::cleanupTestCase() { }
void tst_InputMethod::init() { }
void tst_InputMethod::cleanup() { }

void tst_InputMethod::microFocusCoordinates()
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

void tst_InputMethod::focusInputTypes()
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

void tst_InputMethod::inputModes()
{
    QWebEngineView webView;
    webView.resize(200, 600);
    webView.show();
    QVERIFY(QTest::qWaitForWindowExposed(&webView));

    QSignalSpy loadFinishedSpy(&webView, SIGNAL(loadFinished(bool)));
    webView.load(QUrl(u"qrc:///resources/input_modes.html"_s));
    QVERIFY(loadFinishedSpy.wait());

    auto inputMethodQuery = [&webView](Qt::InputMethodQuery query) {
        QInputMethodQueryEvent event(query);
        QApplication::sendEvent(webView.focusProxy(), &event);
        return event.value(query);
    };

    // inputmode=none
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, elementCenter(webView.page(), u"noneMode"_s));
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), u"document.activeElement.id"_s).toString(), u"noneMode"_s);
    // Should not trigger the virtual keyboard.
    QTRY_VERIFY(!inputMethodQuery(Qt::ImEnabled).toBool());

    // inputmode=text
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, elementCenter(webView.page(), u"textMode"_s));
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), u"document.activeElement.id"_s).toString(), u"textMode"_s);
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), Qt::ImhPreferLowercase);

    // inputmode=tel
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, elementCenter(webView.page(), u"telMode"_s));
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), u"document.activeElement.id"_s).toString(), u"telMode"_s);
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(),
                             Qt::ImhDialableCharactersOnly);

    // inputmode=url
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, elementCenter(webView.page(), u"urlMode"_s));
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), u"document.activeElement.id"_s).toString(), u"urlMode"_s);
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(),
                             Qt::ImhUrlCharactersOnly | Qt::ImhNoAutoUppercase);

    // inputmode=email
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, elementCenter(webView.page(), u"emailMode"_s));
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), u"document.activeElement.id"_s).toString(), u"emailMode"_s);
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), Qt::ImhEmailCharactersOnly);

    // inputmode=numeric
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, elementCenter(webView.page(), u"numericMode"_s));
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), u"document.activeElement.id"_s).toString(), u"numericMode"_s);
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), Qt::ImhDigitsOnly);

    // inputmode=decimal
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, elementCenter(webView.page(), u"decimalMode"_s));
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), u"document.activeElement.id"_s).toString(), u"decimalMode"_s);
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), Qt::ImhFormattedNumbersOnly);

    // inputmode=search
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, elementCenter(webView.page(), u"searchMode"_s));
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), u"document.activeElement.id"_s).toString(), u"searchMode"_s);
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(),
                             Qt::ImhPreferLowercase | Qt::ImhNoAutoUppercase);

    // inputmode=none
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, elementCenter(webView.page(), u"noneMode"_s));
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), u"document.activeElement.id"_s).toString(), u"noneMode"_s);
    // Should hide the virtual keyboard.
    QTRY_VERIFY(!inputMethodQuery(Qt::ImEnabled).toBool());

    // inputmode=numeric overrides <input type="text">
    QTest::mouseClick(webView.focusProxy(), Qt::LeftButton, {}, elementCenter(webView.page(), u"override"_s));
    QTRY_COMPARE(evaluateJavaScriptSync(webView.page(), u"document.activeElement.id"_s).toString(), u"override"_s);
    QTRY_VERIFY(inputMethodQuery(Qt::ImEnabled).toBool());
    VERIFY_INPUTMETHOD_HINTS(webView.focusProxy()->inputMethodHints(), Qt::ImhDigitsOnly);
}

void tst_InputMethod::inputMethodsTextFormat_data()
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

void tst_InputMethod::inputMethodsTextFormat()
{
    QWebEnginePage page;
    QWebEngineView view(&page);
    page.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));

    page.setHtml("<html><body>"
                 " <input type='text' id='input1' style='font-family: serif' value='' maxlength='20'/>"
                 "</body></html>");
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 10000);

    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    evaluateJavaScriptSync(&page, "document.getElementById('input1').focus()");

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
    QApplication::sendEvent(view.focusProxy(), &im);
    QTRY_COMPARE_WITH_TIMEOUT(evaluateJavaScriptSync(&page, "document.getElementById('input1').value").toString(), string, 20000);
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

void tst_InputMethod::softwareInputPanel()
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
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='' size='50'/>"
                 "</body></html>");
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 10000);
    QVERIFY(QTest::qWaitForWindowActive(&view));
    QCOMPARE(testContext.infos.size(), 0);

    // Set focus on an input field.
    QPoint textInputCenter = elementCenter(view.page(), "input1");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(testContext.infos.size(), 2);
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));
    for (const InputMethodInfo &info : std::as_const(testContext.infos)) {
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
    QCOMPARE(testContext.infos[0].cursorPosition, 0);
    QCOMPARE(testContext.infos[0].anchorPosition, 0);
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
    for (const InputMethodInfo &info : std::as_const(testContext.infos)) {
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

void tst_InputMethod::inputMethods()
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
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 10000);
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

void tst_InputMethod::textSelectionInInputField()
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

void tst_InputMethod::textSelectionOutOfInputField()
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
    QTRY_VERIFY_WITH_TIMEOUT(loadFinishedSpy.size(), 10000);
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
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QVERIFY(view.hasSelection());
    QCOMPARE(view.page()->selectedText(), QString("This is a text"));

    // Deselect text by mouse click
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, view.geometry().center());
    QTRY_COMPARE(selectionChangedSpy.size(), 2);
    QVERIFY(!view.hasSelection());
    QVERIFY(view.page()->selectedText().isEmpty());

    // Select text by ctrl+a
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QTRY_COMPARE(selectionChangedSpy.size(), 3);
    QVERIFY(view.hasSelection());
    QCOMPARE(view.page()->selectedText(), QString("This is a text"));

    // Deselect text via discard+undiscard
    view.hide();
    view.page()->setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    view.show();
    QVERIFY(loadFinishedSpy.wait());
    QTRY_COMPARE(selectionChangedSpy.size(), 4);
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
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QVERIFY(view.hasSelection());
    QVERIFY(view.page()->selectedText().startsWith(QString("This is a text")));

    // Remove selection by clicking into an input field
    QPoint textInputCenter = elementCenter(view.page(), "input1");
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, textInputCenter);
    QTRY_COMPARE(selectionChangedSpy.size(), 2);
    QCOMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("input1"));
    QVERIFY(!view.hasSelection());
    QVERIFY(view.page()->selectedText().isEmpty());

    // Select the content of the input field by ctrl+a
    QTest::keyClick(view.windowHandle(), Qt::Key_A, Qt::ControlModifier);
    QTRY_COMPARE(selectionChangedSpy.size(), 3);
    QVERIFY(view.hasSelection());
    QCOMPARE(view.page()->selectedText(), QString("QtWebEngine"));

    // Deselect input field's text by mouse click
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, view.geometry().center());
    QTRY_COMPARE(selectionChangedSpy.size(), 4);
    QVERIFY(!view.hasSelection());
    QVERIFY(view.page()->selectedText().isEmpty());
}

void tst_InputMethod::hiddenText()
{
    QWebEngineView view;
    view.resize(640, 480);
    view.show();

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='QtWebEngine' size='50'/><br>"
                 "  <input type='password' id='password1'/>"
                 "</body></html>");
    QTRY_VERIFY_WITH_TIMEOUT(loadFinishedSpy.size(), 10000);

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

void tst_InputMethod::emptyInputMethodEvent()
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
    QTRY_VERIFY_WITH_TIMEOUT(loadFinishedSpy.size(), 10000);
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
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='QtWebEngine inputMethod'/>"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowActive(&view));

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
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);
    QCOMPARE(view.focusProxy()->inputMethodQuery(Qt::ImAnchorPosition).toInt(), 0);
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

void tst_InputMethod::newlineInTextarea()
{
    SKIP_IF_NO_WINDOW_ACTIVATION();

    QWebEngineView view;
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    view.resize(640, 480);
    view.show();
    view.window()->windowHandle()->requestActivate();

    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.page()->setHtml("<html><body>"
                         "  <textarea rows='5' cols='1' id='input1'></textarea>"
                         "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowActive(&view));

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

void tst_InputMethod::imeJSInputEvents()
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
    QTRY_VERIFY_WITH_TIMEOUT(loadFinishedSpy.size(), 10000);
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
    QCOMPARE(logLines()[1], QStringLiteral("[object CompositionEvent] compositionupdate preedit"));
    QCOMPARE(logLines()[2], QStringLiteral("[object InputEvent] beforeinput preedit"));
    QCOMPARE(logLines()[3], QStringLiteral("[object InputEvent] input preedit"));

    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString("commit");
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(logLines().size(), 9);
    QCOMPARE(logLines()[4], QStringLiteral("[object CompositionEvent] compositionupdate commit"));
    QCOMPARE(logLines()[5], QStringLiteral("[object InputEvent] beforeinput commit"));
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
    QCOMPARE(logLines()[1], QStringLiteral("[object CompositionEvent] compositionupdate preedit"));
    QCOMPARE(logLines()[2], QStringLiteral("[object InputEvent] beforeinput preedit"));
    QCOMPARE(logLines()[3], QStringLiteral("[object InputEvent] input preedit"));

    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        QApplication::sendEvent(view.focusProxy(), &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(logLines().size(), 9);
    QCOMPARE(logLines()[4], QStringLiteral("[object CompositionEvent] compositionupdate "));
    QCOMPARE(logLines()[5], QStringLiteral("[object InputEvent] beforeinput "));
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

void tst_InputMethod::imeCompositionQueryEvent_data()
{
    QTest::addColumn<QString>("receiverObjectName");
    QTest::newRow("focusObject") << QString("focusObject");
    QTest::newRow("focusProxy") << QString("focusProxy");
    QTest::newRow("focusWidget") << QString("focusWidget");
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
    view.setHtml("<html><body>"
                 "  <input type='text' id='input1' />"
                 "</body></html>");
    QVERIFY(loadFinishedSpy.wait());
    QVERIFY(QTest::qWaitForWindowActive(&view));

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
    QInputMethodQueryEvent absolutePosQuery(Qt::ImAbsolutePosition);
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
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 0);

    QApplication::sendEvent(input, &srrndTextQuery);
    QApplication::sendEvent(input, &absolutePosQuery);
    QApplication::sendEvent(input, &cursorPosQuery);
    QApplication::sendEvent(input, &anchorPosQuery);
    qApp->processEvents();

    QTRY_COMPARE(srrndTextQuery.value(Qt::ImSurroundingText).toString(), QString(""));
    QTRY_COMPARE(absolutePosQuery.value(Qt::ImAbsolutePosition).toInt(), 0);
    QTRY_COMPARE(cursorPosQuery.value(Qt::ImCursorPosition).toInt(), 0);
    QTRY_COMPARE(anchorPosQuery.value(Qt::ImAnchorPosition).toInt(), 0);

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
    QApplication::sendEvent(input, &absolutePosQuery);
    QApplication::sendEvent(input, &cursorPosQuery);
    QApplication::sendEvent(input, &anchorPosQuery);
    qApp->processEvents();

    QTRY_COMPARE(srrndTextQuery.value(Qt::ImSurroundingText).toString(), QString("composition"));
    QTRY_COMPARE(absolutePosQuery.value(Qt::ImAbsolutePosition).toInt(), 11);
    QTRY_COMPARE(cursorPosQuery.value(Qt::ImCursorPosition).toInt(), 11);
    QTRY_COMPARE(anchorPosQuery.value(Qt::ImAnchorPosition).toInt(), 11);

    // Test another composition to ensure that the cursor position is set correctly.
    // In this case cursor will be at position 11 during input composition.
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("123", attributes);
        QApplication::sendEvent(input, &event);
        qApp->processEvents();
    }
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value")
                         .toString(),
                 QString("composition123"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 11);

    QApplication::sendEvent(input, &srrndTextQuery);
    QApplication::sendEvent(input, &absolutePosQuery);
    QApplication::sendEvent(input, &cursorPosQuery);
    QApplication::sendEvent(input, &anchorPosQuery);
    qApp->processEvents();

    QTRY_COMPARE(srrndTextQuery.value(Qt::ImSurroundingText).toString(), QString("composition"));
    QTRY_COMPARE(absolutePosQuery.value(Qt::ImAbsolutePosition).toInt(), 11);
    QTRY_COMPARE(cursorPosQuery.value(Qt::ImCursorPosition).toInt(), 11);
    QTRY_COMPARE(anchorPosQuery.value(Qt::ImAnchorPosition).toInt(), 11);

    // Send commit
    {
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent event("", attributes);
        event.setCommitString("123");
        QApplication::sendEvent(input, &event);
        qApp->processEvents();
    }

    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.getElementById('input1').value")
                         .toString(),
                 QString("composition123"));
    QTRY_COMPARE(view.focusProxy()->inputMethodQuery(Qt::ImCursorPosition).toInt(), 14);

    QApplication::sendEvent(input, &srrndTextQuery);
    QApplication::sendEvent(input, &absolutePosQuery);
    QApplication::sendEvent(input, &cursorPosQuery);
    QApplication::sendEvent(input, &anchorPosQuery);
    qApp->processEvents();

    QTRY_COMPARE(srrndTextQuery.value(Qt::ImSurroundingText).toString(), QString("composition123"));
    QTRY_COMPARE(absolutePosQuery.value(Qt::ImAbsolutePosition).toInt(), 14);
    QTRY_COMPARE(cursorPosQuery.value(Qt::ImCursorPosition).toInt(), 14);
    QTRY_COMPARE(anchorPosQuery.value(Qt::ImAnchorPosition).toInt(), 14);
}

QTEST_MAIN(tst_InputMethod)
#include "tst_inputmethod.moc"
