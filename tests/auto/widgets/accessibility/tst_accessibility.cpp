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

#include <qtest.h>
#include "../util.h"

#include <QHBoxLayout>
#include <QMainWindow>

#include <qaccessible.h>
#include <qwebengineview.h>
#include <qwebenginepage.h>
#include <qwebenginesettings.h>
#include <qwidget.h>

class tst_Accessibility : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void noPage();
    void hierarchy();
    void focusChild();
    void focusChild_data();
    void text();
    void value();
    void roles_data();
    void roles();
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_Accessibility::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_Accessibility::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_Accessibility::init()
{
}

// This will be called after every test function.
void tst_Accessibility::cleanup()
{
}

void tst_Accessibility::noPage()
{
    QWebEngineView webView;
    webView.show();

    QTest::qWait(1000);
    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);
    QVERIFY(view);
    QCOMPARE(view->role(), QAccessible::Client);
    QCOMPARE(view->childCount(), 1);
    QAccessibleInterface *document = view->child(0);
    QCOMPARE(document->role(), QAccessible::WebDocument);
    QCOMPARE(document->parent(), view);
    QCOMPARE(document->childCount(), 0);
}

void tst_Accessibility::hierarchy()
{
    QWebEngineView webView;
    webView.setHtml("<html><body>" \
        "Hello world" \
        "<input type='text' value='some text'></input>" \
        "</body></html>");
    webView.show();
    QSignalSpy spyFinished(&webView, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);
    QVERIFY(view);
    QCOMPARE(view->role(), QAccessible::Client);
    QCOMPARE(view->childCount(), 1);
    // Wait for accessibility to be fully initialized
    QTRY_VERIFY(view->child(0)->childCount() == 1);
    QAccessibleInterface *document = view->child(0);
    QCOMPARE(document->role(), QAccessible::WebDocument);
    QCOMPARE(document->parent(), view);
    QCOMPARE(view->indexOfChild(document), 0);
    QCOMPARE(document->childCount(), 1);
    QAccessibleInterface *grouping = document->child(0);
    QVERIFY(grouping);
    QCOMPARE(grouping->parent(), document);
    QCOMPARE(document->indexOfChild(grouping), 0);
    QCOMPARE(grouping->childCount(), 2);

    QAccessibleInterface *text = grouping->child(0);
    QCOMPARE(text->role(), QAccessible::StaticText);
    QCOMPARE(text->parent(), grouping);
    QCOMPARE(grouping->indexOfChild(text), 0);
    QCOMPARE(text->childCount(), 0);
    QCOMPARE(text->text(QAccessible::Name), QStringLiteral("Hello world"));
    QCOMPARE(text->text(QAccessible::Description), QString());
    QCOMPARE(text->text(QAccessible::Value), QString());

    QAccessibleInterface *input = grouping->child(1);
    QCOMPARE(input->role(), QAccessible::EditableText);
    QCOMPARE(input->parent(), grouping);
    QCOMPARE(grouping->indexOfChild(input), 1);
    QCOMPARE(input->childCount(), 0);
    QCOMPARE(input->text(QAccessible::Name), QString());
    QCOMPARE(input->text(QAccessible::Description), QString());
    QCOMPARE(input->text(QAccessible::Value), QStringLiteral("some text"));

    QRect windowRect = webView.geometry();
    QRect inputRect = input->rect();
    QVERIFY(!inputRect.isEmpty());
    QVERIFY(windowRect.contains(inputRect));
    QPoint inputCenter = inputRect.center();
    QAccessibleInterface *hitTest = view;
    QAccessibleInterface *child = nullptr;
    while (hitTest) {
        child = hitTest;
        hitTest = hitTest->childAt(inputCenter.x(), inputCenter.y());
    }
    QCOMPARE(input, child);
}

void tst_Accessibility::focusChild_data()
{
    QTest::addColumn<QString>("interfaceName");
    QTest::addColumn<QVector<QAccessible::Role>>("ancestorRoles");

    QTest::newRow("QWebEngineView") << QString("QWebEngineView") << QVector<QAccessible::Role>({QAccessible::Client});
    QTest::newRow("RenderWidgetHostViewQtDelegate") << QString("RenderWidgetHostViewQtDelegate") << QVector<QAccessible::Role>({QAccessible::Client});
    QTest::newRow("QMainWindow") << QString("QMainWindow") << QVector<QAccessible::Role>({QAccessible::Window, QAccessible::Client /* central widget */, QAccessible::Client /* view */});
}

void tst_Accessibility::focusChild()
{
    auto traverseToWebDocumentAccessibleInterface = [](QAccessibleInterface *iface) -> QAccessibleInterface * {
        QFETCH(QVector<QAccessible::Role>, ancestorRoles);
        for (int i = 0; i < ancestorRoles.size(); ++i) {
            if (iface->childCount() == 0 || iface->role() != ancestorRoles[i])
                return nullptr;
            iface = iface->child(0);
        }

        if (iface->role() != QAccessible::WebDocument)
            return nullptr;

        return iface;
    };

    QMainWindow mainWindow;
    QWebEngineView *webView = new QWebEngineView;
    QWidget *centralWidget = new QWidget;
    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralWidget->setLayout(centralLayout);
    mainWindow.setCentralWidget(centralWidget);
    centralLayout->addWidget(webView);

    mainWindow.show();
    QVERIFY(QTest::qWaitForWindowExposed(&mainWindow));

    webView->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    webView->setHtml("<html><body>" \
        "<input id='input1' type='text' value='some text'/>" \
        "</body></html>");
    webView->show();
    QSignalSpy spyFinished(webView, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QVERIFY(webView->focusWidget());
    QAccessibleInterface *iface = nullptr;
    QFETCH(QString, interfaceName);
    if (interfaceName == "QWebEngineView")
        iface = QAccessible::queryAccessibleInterface(webView);
    else if (interfaceName == "RenderWidgetHostViewQtDelegate")
        iface = QAccessible::queryAccessibleInterface(webView->focusWidget());
    else if (interfaceName == "QMainWindow")
        iface = QAccessible::queryAccessibleInterface(&mainWindow);
    QVERIFY(iface);

    // Make sure the input field does not have the focus.
    evaluateJavaScriptSync(webView->page(), "document.getElementById('input1').blur()");
    QTRY_VERIFY(evaluateJavaScriptSync(webView->page(), "document.activeElement.id").toString().isEmpty());

    QVERIFY(iface->focusChild());
    QTRY_COMPARE(iface->focusChild()->role(), QAccessible::WebDocument);
    QCOMPARE(traverseToWebDocumentAccessibleInterface(iface), iface->focusChild());

    // Set active focus on the input field.
    evaluateJavaScriptSync(webView->page(), "document.getElementById('input1').focus()");
    QTRY_COMPARE(evaluateJavaScriptSync(webView->page(), "document.activeElement.id").toString(), QStringLiteral("input1"));

    QVERIFY(iface->focusChild());
    QTRY_COMPARE(iface->focusChild()->role(), QAccessible::EditableText);
    // <html> -> <body> -> <input>
    QCOMPARE(traverseToWebDocumentAccessibleInterface(iface)->child(0)->child(0), iface->focusChild());
}

void tst_Accessibility::text()
{
    QWebEngineView webView;
    webView.setHtml("<html><body>" \
        "<input type='text' value='Good morning!'></input>" \
        "<p id='labelName'>Enter your name here:</p>" \
        "<input type='text' value='my name' aria-labelledby='labelName' aria-describedby='explanation'></input>" \
        "<p id='explanation'>Provide both first and last name.</p>" \
        "<input type='text' value='Good day!' placeholder='day'></input>" \
        "</body></html>");
    webView.show();
    QSignalSpy spyFinished(&webView, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);
    // Wait for accessibility to be fully initialized

    QTRY_VERIFY(view->child(0)->childCount() == 5);
    QAccessibleInterface *document = view->child(0);
    QVERIFY(document);

    // Good morning! [edit]
    QAccessibleInterface *grouping1 = document->child(0);
    QAccessibleInterface *input1 = grouping1;
    QCOMPARE(input1->role(), QAccessible::EditableText);
    QCOMPARE(input1->text(QAccessible::Name), QString());
    QCOMPARE(input1->text(QAccessible::Description), QString());
    QCOMPARE(input1->text(QAccessible::Value), QStringLiteral("Good morning!"));

    QAccessibleTextInterface *textInterface1 = input1->textInterface();
    QVERIFY(textInterface1);
    QCOMPARE(textInterface1->characterCount(), 13);
    QCOMPARE(textInterface1->selectionCount(), 0);
    QCOMPARE(textInterface1->text(2, 9), QStringLiteral("od morn"));
    int start = -1;
    int end = -1;
    QCOMPARE(textInterface1->textAtOffset(8, QAccessible::WordBoundary, &start, &end), QStringLiteral("morning"));

    // Enter your name here:
    // my name [edit]
    // Provide both first and last name here.
    QAccessibleInterface *grouping2 = document->child(1);
    QAccessibleInterface *label1 = grouping2->child(0);
    QCOMPARE(label1->role(), QAccessible::StaticText);
    QCOMPARE(label1->text(QAccessible::Name), QStringLiteral("Enter your name here:"));
    QCOMPARE(label1->text(QAccessible::Description), QString());
    QCOMPARE(label1->text(QAccessible::Value), QString());
    QAccessibleInterface *grouping3 = document->child(2);
    QAccessibleInterface *input2 = grouping3;
    QCOMPARE(input2->role(), QAccessible::EditableText);
    QCOMPARE(input2->text(QAccessible::Name), QStringLiteral("Enter your name here:"));
    QCOMPARE(input2->text(QAccessible::Description), QStringLiteral("Provide both first and last name."));
    QCOMPARE(input2->text(QAccessible::Value), QStringLiteral("my name"));
    QAccessibleInterface *grouping4 = document->child(3);
    QAccessibleInterface *label2 = grouping4->child(0);
    QCOMPARE(label2->role(), QAccessible::StaticText);
    QCOMPARE(label2->text(QAccessible::Name), QStringLiteral("Provide both first and last name."));
    QCOMPARE(label2->text(QAccessible::Description), QString());
    QCOMPARE(label2->text(QAccessible::Value), QString());

    // Good day! [edit]
    QAccessibleInterface *grouping5 = document->child(4);
    QAccessibleInterface *input3 = grouping5;
    QCOMPARE(input3->role(), QAccessible::EditableText);
    QCOMPARE(input3->text(QAccessible::Name), QStringLiteral("day"));
    QCOMPARE(input3->text(QAccessible::Description), QString());
    QCOMPARE(input3->text(QAccessible::Value), QStringLiteral("Good day!"));
}

void tst_Accessibility::value()
{
    QWebEngineView webView;
    webView.setHtml("<html><body>" \
        "<div role='slider' aria-valuenow='4' aria-valuemin='1' aria-valuemax='10'></div>" \
        "<div class='progress' role='progressbar' aria-valuenow='77' aria-valuemin='22' aria-valuemax='99'></div>" \
        "</body></html>");
    webView.show();
    QSignalSpy spyFinished(&webView, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);
    QTRY_COMPARE(view->child(0)->childCount(), 2);
    QAccessibleInterface *document = view->child(0);
    QCOMPARE(document->childCount(), 2);

    QAccessibleInterface *slider = document->child(0);
    QCOMPARE(slider->role(), QAccessible::Slider);
    QCOMPARE(slider->text(QAccessible::Name), QString());
    QCOMPARE(slider->text(QAccessible::Description), QString());
    QCOMPARE(slider->text(QAccessible::Value), QString());
    QAccessibleValueInterface *valueInterface = slider->valueInterface();
    QVERIFY(valueInterface);
    QCOMPARE(valueInterface->currentValue().toInt(), 4);
    QCOMPARE(valueInterface->minimumValue().toInt(), 1);
    QCOMPARE(valueInterface->maximumValue().toInt(), 10);

    QAccessibleInterface *progressBar = document->child(1);
    QCOMPARE(progressBar->role(), QAccessible::ProgressBar);
    QCOMPARE(progressBar->text(QAccessible::Name), QString());
    QCOMPARE(progressBar->text(QAccessible::Description), QString());
    QCOMPARE(progressBar->text(QAccessible::Value), QString());
    QAccessibleValueInterface *progressBarValueInterface = progressBar->valueInterface();
    QVERIFY(progressBarValueInterface);
    QCOMPARE(progressBarValueInterface->currentValue().toInt(), 77);
    QCOMPARE(progressBarValueInterface->minimumValue().toInt(), 22);
    QCOMPARE(progressBarValueInterface->maximumValue().toInt(), 99);
}

void tst_Accessibility::roles_data()
{
    QTest::addColumn<QString>("html");
    QTest::addColumn<int>("nested");
    QTest::addColumn<QAccessible::Role>("role");

    QTest::newRow("ax::mojom::Role::kAbbr") << QString("<abbr>a</abbr>") << 1 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kAlert") << QString("<div role='alert'>alert</div>") << 0 << QAccessible::AlertMessage;
    QTest::newRow("ax::mojom::Role::kAlertDialog") << QString("<div role='alertdialog'>alert</div>") << 0 << QAccessible::AlertMessage;
    QTest::newRow("ax::mojom::Role::kAnchor") << QString("<a id='a'>Chapter a</a>") << 1 << QAccessible::Link;
    QTest::newRow("ax::mojom::Role::kApplication") << QString("<div role='application'>landmark</div>") << 0 << QAccessible::Document;
    QTest::newRow("ax::mojom::Role::kArticle") << QString("<article>a</article>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kAudio") << QString("<audio controls><source src='test.mp3' type='audio/mpeg'></audio>") << 1 << QAccessible::Sound;
    QTest::newRow("ax::mojom::Role::kBanner") << QString("<div role='banner'>a</div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kBlockquote") << QString("<blockquote>a</blockquote>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kButton") << QString("<button>a</button>") << 1 << QAccessible::Button;
    //QTest::newRow("ax::mojom::Role::kCanvas") << QString("<canvas width='10' height='10'></canvas>") << 0 << QAccessible::Canvas; // FIXME: The test case might be wrong (see AXLayoutObject.cpp)
    QTest::newRow("ax::mojom::Role::kCaption") << QString("<table><caption>a</caption></table>") << 1 << QAccessible::Heading;
    //QTest::newRow("ax::mojom::Role::kCaret"); // No mapping to ARIA role
    QTest::newRow("ax::mojom::Role::kCell") << QString("<table role=table><tr><td>a</td></tr></table>") << 2 << QAccessible::Cell;
    QTest::newRow("ax::mojom::Role::kCheckBox") << QString("<input type='checkbox'>a</input>") << 1 << QAccessible::CheckBox;
    QTest::newRow("ax::mojom::Role::kClient") << QString("") << 0 << QAccessible::Client;
    QTest::newRow("ax::mojom::Role::kCode") << QString("<code>a</code>") << 1 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kColorWell") << QString("<input type='color'>a</input>") << 1 << QAccessible::ColorChooser;
    //QTest::newRow("ax::mojom::Role::kColumn") << QString("<table><tr><td>a</td></tr></table>") << 0 << QAccessible::Column; // FIXME: The test case might be wrong (see AXTableColumn.h)
    QTest::newRow("ax::mojom::Role::kColumnHeader") << QString("<table role=table><tr><th>a</th></tr><tr><td>a</td></tr></table>") << 2 << QAccessible::ColumnHeader;
    QTest::newRow("ax::mojom::Role::kComboBoxGrouping") << QString("<div role='combobox'><input></div>") << 0 << QAccessible::ComboBox;
    QTest::newRow("ax::mojom::Role::kComboBoxMenuButton") << QString("<div tabindex=0 role='combobox'>Select</div>") << 0 << QAccessible::ComboBox;
    QTest::newRow("ax::mojom::Role::kTextFieldWithComboBox") << QString("<input role='combobox'>") << 1 << QAccessible::ComboBox;
    QTest::newRow("ax::mojom::Role::kComplementary") << QString("<aside>a</aside>") << 0 << QAccessible::ComplementaryContent;
    QTest::newRow("ax::mojom::Role::kComment") << QString("<div role='comment'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kContentDeletion") << QString("<div role='deletion'></div>") << 0 << QAccessible::Grouping;
    QTest::newRow("ax::mojom::Role::kContentInsertion") << QString("<div role='insertion'></div>") << 0 << QAccessible::Grouping;
    QTest::newRow("ax::mojom::Role::kContentInfo") << QString("<div role='contentinfo'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kData") << QString("<input type='date'></input>") << 1 << QAccessible::Clock;
    QTest::newRow("ax::mojom::Role::kDateTime") << QString("<input type='datetime-local'></input>") << 1 << QAccessible::Clock;
    QTest::newRow("ax::mojom::Role::kDefinition") << QString("<div role='definition'>landmark</div>") << 0 << QAccessible::Paragraph;
    QTest::newRow("ax::mojom::Role::kDescriptionList") << QString("<dl>a</dl>") << 0 << QAccessible::List;
    QTest::newRow("ax::mojom::Role::kDescriptionListDetail") << QString("<dd>a</dd>") << 0 << QAccessible::Paragraph;
    QTest::newRow("ax::mojom::Role::kDescriptionListTerm") << QString("<dt>a</dt>") << 0 << QAccessible::ListItem;
    QTest::newRow("ax::mojom::Role::kDetails") << QString("<details>a</details>") << 0 << QAccessible::Grouping;
    //QTest::newRow("ax::mojom::Role::kDesktop"); // No mapping to ARIA role
    QTest::newRow("ax::mojom::Role::kDialog") << QString("<div role='dialog'></div>") << 0 << QAccessible::Dialog;
    //QTest::newRow("ax::mojom::Role::kDirectory") << QString("<ul role='directory'></ul>") << 0 << QAccessible::List; // FIXME: Aria role 'directory' should work
    QTest::newRow("ax::mojom::Role::kDisclosureTriangle") << QString("<details><summary>a</summary></details>") << 1 << QAccessible::Button;
    QTest::newRow("ax::mojom::Role::kGenericContainer") << QString("<div>a</div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocCover") << QString("<div role='doc-cover'></div>") << 0 << QAccessible::Graphic;
    QTest::newRow("ax::mojom::Role::kDocBackLink") << QString("<div role='doc-backlink'></div>") << 0 << QAccessible::Link;
    QTest::newRow("ax::mojom::Role::kDocBiblioRef") << QString("<div role='doc-biblioref'></div>") << 0 << QAccessible::Link;
    QTest::newRow("ax::mojom::Role::kDocGlossRef") << QString("<div role='doc-glossref'></div>") << 0 << QAccessible::Link;
    QTest::newRow("ax::mojom::Role::kDocNoteRef") << QString("<div role='doc-noteref'></div>") << 0 << QAccessible::Link;
    QTest::newRow("ax::mojom::Role::kDocBiblioEntry") << QString("<div role='doc-biblioentry'></div>") << 0 << QAccessible::ListItem;
    QTest::newRow("ax::mojom::Role::kDocEndnote") << QString("<div role='doc-endnote'></div>") << 0 << QAccessible::ListItem;
    QTest::newRow("ax::mojom::Role::kDocFootnote") << QString("<div role='doc-footnote'></div>") << 0 << QAccessible::ListItem;
    QTest::newRow("ax::mojom::Role::kDocPageBreak") << QString("<div role='doc-pagebreak'></div>") << 0 << QAccessible::Separator;
    QTest::newRow("ax::mojom::Role::kDocAbstract") << QString("<div role='doc-abstract'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocAcknowledgements") << QString("<div role='doc-acknowledgments'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocAfterword") << QString("<div role='doc-afterword'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocAppendix") << QString("<div role='doc-appendix'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocBibliography") << QString("<div role='doc-bibliography'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocChapter") << QString("<div role='doc-chapter'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocColophon") << QString("<div role='doc-colophon'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocConclusion") << QString("<div role='doc-conclusion'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocCredit") << QString("<div role='doc-credit'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocCredits") << QString("<div role='doc-credits'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocDedication") << QString("<div role='doc-dedication'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocEndnotes") << QString("<div role='doc-endnotes'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocEpigraph") << QString("<div role='doc-epigraph'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocEpilogue") << QString("<div role='doc-epilogue'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocErrata") << QString("<div role='doc-errata'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocExample") << QString("<div role='doc-example'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocForeword") << QString("<div role='doc-foreword'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocGlossary") << QString("<div role='doc-glossary'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocIndex") << QString("<div role='doc-index'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocIntroduction") << QString("<div role='doc-introduction'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocNotice") << QString("<div role='doc-notice'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocPageList") << QString("<div role='doc-pagelist'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocPart") << QString("<div role='doc-part'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocPreface") << QString("<div role='doc-preface'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocPrologue") << QString("<div role='doc-prologue'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocPullquote") << QString("<div role='doc-pullquote'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocQna") << QString("<div role='doc-qna'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocSubtitle") << QString("<div role='doc-subtitle'></div>") << 0 << QAccessible::Heading;
    QTest::newRow("ax::mojom::Role::kDocTip") << QString("<div role='doc-tip'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocToc") << QString("<div role='doc-toc'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kDocument") << QString("<div role='document'>a</div>") << 0 << QAccessible::Document;
    QTest::newRow("ax::mojom::Role::kEmbeddedObject") << QString("<object width='10' height='10'></object>") << 1 << QAccessible::Grouping;
    QTest::newRow("ax::mojom::Role::kEmphasis") << QString("<em>a</em>") << 1 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kFeed") << QString("<div role='feed'>a</div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kFigcaption") << QString("<figcaption>a</figcaption>") << 0 << QAccessible::Heading;
    QTest::newRow("ax::mojom::Role::kFigure") << QString("<figure>a</figure>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kFooter") << QString("<footer>a</footer>") << 0 << QAccessible::Footer;
    QTest::newRow("ax::mojom::Role::kFooterAsNonLandmark") << QString("<article><footer>a</footer><article>") << 1 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kForm") << QString("<form></form>") << 0 << QAccessible::Form;
    QTest::newRow("ax::mojom::Role::kGraphicsDocument") << QString("<div role='graphics-document'></div>") << 0 << QAccessible::Document;
    QTest::newRow("ax::mojom::Role::kGraphicsObject") << QString("<div role='graphics-object'></div>") << 0 << QAccessible::Pane;
    QTest::newRow("ax::mojom::Role::kGraphicsSymbol") << QString("<div role='graphics-symbol'></div>") << 0 << QAccessible::Graphic;
    QTest::newRow("ax::mojom::Role::kGrid") << QString("<div role='grid'></div>") << 0 << QAccessible::Table;
    QTest::newRow("ax::mojom::Role::kGroup") << QString("<fieldset></fieldset>") << 0 << QAccessible::Grouping;
    QTest::newRow("ax::mojom::Role::Header") << QString("<header>a</header>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::HeaderAsNonLandMark") << QString("<article><header>a</header><article>") << 1 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kHeading") << QString("<h1>a</h1>") << 0 << QAccessible::Heading;
    QTest::newRow("ax::mojom::Role::kIframe") << QString("<iframe>a</iframe>") << 1 << QAccessible::WebDocument;
    QTest::newRow("ax::mojom::Role::kIframePresentational") << QString("<iframe role='presentation'>a</iframe>") << 1 << QAccessible::Grouping;
    //QTest::newRow("ax::mojom::Role::kIgnored") << QString("<tag>a</tag>") << 0 << QAccessible::NoRole; // FIXME: The HTML element should not be exposed as an element (see AXNodeObject.cpp)
    QTest::newRow("ax::mojom::Role::kImage") << QString("<img>") << 1 << QAccessible::Graphic;
    //QTest::newRow("ax::mojom::Role::kImageMap") << QString("<img usemap='map'>") << 0 << QAccessible::Document; // FIXME: AXLayoutObject::DetermineAccessiblityRole returns kImageMap but something overrides it
    //QTest::newRow("ax::mojom::Role::kInlineTextBox"); // No mapping to ARIA role
    QTest::newRow("ax::mojom::Role::kInputTime") << QString("<input type='time'></input>") << 1 << QAccessible::SpinBox;
    //QTest::newRow("ax::mojom::Role::kKeyboard"); // No mapping to ARIA role
    QTest::newRow("ax::mojom::Role::kLabelText") << QString("<label>a</label>") << 1 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kLayoutTable") << QString("<table><tr><td></td></tr></table>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kLayoutTableCell") << QString("<table><tr><td></td></tr></table>") << 2 << QAccessible::Section;
    //QTest::newRow("ax::mojom::Role::kLayoutTableColumn") << QString("<table><tr></tr></table>") << 1 << QAccessible::Section; // FIXME: The test case might be wrong
    QTest::newRow("ax::mojom::Role::kLayoutTableRow") << QString("<table><tr><td></td></tr></table>") << 1 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kLegend") << QString("<legend>a</legend>") << 0 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kLineBreak") << QString("<br>") << 1 << QAccessible::Separator;
    QTest::newRow("ax::mojom::Role::kLink") << QString("<a href=''>link</a>") << 1 << QAccessible::Link;
    QTest::newRow("ax::mojom::Role::kList") << QString("<ul></ul>") << 0 << QAccessible::List;
    QTest::newRow("ax::mojom::Role::kListBox") << QString("<select multiple></select>") << 1 << QAccessible::ComboBox;
    QTest::newRow("ax::mojom::Role::kListBoxOption") << QString("<option>a</option>") << 0 << QAccessible::ListItem;
    QTest::newRow("ax::mojom::Role::kListItem") << QString("<li>a</li>") << 0 << QAccessible::ListItem;
    //QTest::newRow("ax::mojom::Role::kListGrid"); // No mapping to ARIA role
    QTest::newRow("ax::mojom::Role::kListMarker") << QString("<li><ul></ul></li>") << 1 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kLog") << QString("<div role='log'>a</div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kMain") << QString("<main>a</main>") << 0 << QAccessible::Grouping;
    QTest::newRow("ax::mojom::Role::kMark") << QString("<mark>a</mark>") << 1 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kMarquee") << QString("<div role='marquee'>a</div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kMath") << QString("<math>a</math>") << 1 << QAccessible::Equation;
    QTest::newRow("ax::mojom::Role::kMenu") << QString("<div role='menu'>a</div>") << 0 << QAccessible::PopupMenu;
    QTest::newRow("ax::mojom::Role::kMenuBar") << QString("<div role='menubar'>a</div>") << 0 << QAccessible::MenuBar;
    QTest::newRow("ax::mojom::Role::kMenuItem") << QString("<menu role='menu'><input type='button' /></menu>") << 1 << QAccessible::MenuItem;
    QTest::newRow("ax::mojom::Role::kMenuItemCheckBox") << QString("<menu role='menu'><input type='checkbox'></input></menu>") << 1 << QAccessible::CheckBox;
    QTest::newRow("ax::mojom::Role::kMenuItemRadio") << QString("<menu role='menu'><input type='radio'></input></menu>") << 1 << QAccessible::RadioButton;
    QTest::newRow("ax::mojom::Role::kMenuButton") << QString("<menu role='group'><div role='menuitem'>a</div></menu>") << 1 << QAccessible::MenuItem;
    QTest::newRow("ax::mojom::Role::kMenuListOption") << QString("<select role='menu'><option>a</option></select>") << 3 << QAccessible::MenuItem;
    QTest::newRow("ax::mojom::Role::kMenuListPopup") << QString("<select role='menu'><option>a</option></select>") << 2 << QAccessible::PopupMenu;
    QTest::newRow("ax::mojom::Role::kMeter") << QString("<meter>a</meter>") << 1 << QAccessible::Chart;
    QTest::newRow("ax::mojom::Role::kNavigation") << QString("<nav>a</nav>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kNote") << QString("<div role='note'>a</div>") << 0 << QAccessible::Note;
    //QTest::newRow("ax::mojom::Role::kPane"); // No mapping to ARIA role
    QTest::newRow("ax::mojom::Role::kParagraph") << QString("<p>a</p>") << 0 << QAccessible::Paragraph;
    QTest::newRow("ax::mojom::Role::kPopUpButton") << QString("<select><option>a</option></select>") << 1 << QAccessible::ComboBox;
    QTest::newRow("ax::mojom::Role::kPre") << QString("<pre>a</pre>") << 0 << QAccessible::Section;
    //QTest::newRow("ax::mojom::Role::kPresentational") << QString("<div role='presentation'>a</div>") << 0 << QAccessible::NoRole; // FIXME: Aria role 'presentation' should work
    QTest::newRow("ax::mojom::Role::kProgressIndicator") << QString("<div role='progressbar' aria-valuenow='77' aria-valuemin='22' aria-valuemax='99'></div>") << 0 << QAccessible::ProgressBar;
    QTest::newRow("ax::mojom::Role::kRadioButton") << QString("<input type='radio'></input>") << 1 << QAccessible::RadioButton;
    QTest::newRow("ax::mojom::Role::kRadioGroup") << QString("<fieldset role='radiogroup'></fieldset>") << 0 << QAccessible::Grouping;
    QTest::newRow("ax::mojom::Role::kRegion") << QString("<div role='region'>a</div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kRow") << QString("<table role=table><tr><td>a</td></tr></table>") << 1 << QAccessible::Row;
    QTest::newRow("ax::mojom::Role::kRowGroup") << QString("<table role=table><tbody role=rowgroup><tr><td>a</td></tr></tbody></table>") << 1 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kRowHeader") << QString("<table role=table><tr><th>a</td><td>b</td></tr></table>") << 2 << QAccessible::RowHeader;
    QTest::newRow("ax::mojom::Role::kRuby") << QString("<ruby>a</ruby>") << 1 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kRubyAnnotation") << QString("<ruby><rt>a</rt></ruby>") << 2 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kScrollBar") << QString("<div role='scrollbar'>a</a>") << 0 << QAccessible::ScrollBar;
    //QTest::newRow("ax::mojom::Role::kScrollView"); // No mapping to ARIA role
    QTest::newRow("ax::mojom::Role::kSearch") << QString("<div role='search'>landmark</div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kSearchBox") << QString("<input type='search'></input>") << 1 << QAccessible::EditableText;
    QTest::newRow("ax::mojom::Role::kSection") << QString("<section></section>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kSlider") << QString("<input type='range'>") << 1 << QAccessible::Slider;
    //QTest::newRow("ax::mojom::Role::kSliderThumb") << QString("<input type='range'>") << 1 << QAccessible::Slider; // TODO: blink/renderer/modules/accessibility/ax_slider.cc
    QTest::newRow("ax::mojom::Role::kSpinButton") << QString("<input type='number'></input>") << 1 << QAccessible::SpinBox;
    QTest::newRow("ax::mojom::Role::kSplitter") << QString("<hr>") << 0 << QAccessible::Splitter;
    QTest::newRow("ax::mojom::Role::kStaticText") << QString("a") << 1 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kStatus") << QString("<output>a</output>") << 1 << QAccessible::Indicator;
    QTest::newRow("ax::mojom::Role::kStrong") << QString("<strong>a</strong>") << 1 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kSuggestion") << QString("<div role='suggestion'></div>") << 0 << QAccessible::Section;
    QTest::newRow("ax::mojom::Role::kSvgRoot") << QString("<svg width='10' height='10'></svg>") << 1 << QAccessible::Graphic;
    QTest::newRow("ax::mojom::Role::kSwitch") << QString("<button aria-checked='false'>a</button>") << 1 << QAccessible::Button;
    QTest::newRow("ax::mojom::Role::kTable") << QString("<table role=table><td>a</td></table>") << 0 << QAccessible::Table;
    //QTest::newRow("ax::mojom::Role::kTableHeaderContainer"); // No mapping to ARIA role
    QTest::newRow("ax::mojom::Role::kTab") << QString("<div role='tab'>a</div>") << 0 << QAccessible::PageTab;
    QTest::newRow("ax::mojom::Role::kTabList") << QString("<div role='tablist'>a</div>") << 0 << QAccessible::PageTabList;
    QTest::newRow("ax::mojom::Role::kTabPanel") << QString("<div role='tabpanel'>a</div>") << 0 << QAccessible::Pane;
    QTest::newRow("ax::mojom::Role::kTerm") << QString("<div role='term'>a</div>") << 0 << QAccessible::StaticText;
    QTest::newRow("ax::mojom::Role::kTextField") << QString("<input type='text'></input>") << 1 << QAccessible::EditableText;
    QTest::newRow("ax::mojom::Role::kTime") << QString("<time>a</time>") << 1 << QAccessible::Clock;
    QTest::newRow("ax::mojom::Role::kTimer") << QString("<div role='timer'>a</div>") << 0 << QAccessible::Clock;
    //QTest::newRow("ax::mojom::Role::kTitleBar"); // No mapping to ARIA role
    QTest::newRow("ax::mojom::Role::kToggleButton") << QString("<button aria-pressed='false'>a</button>") << 1 << QAccessible::Button;
    QTest::newRow("ax::mojom::Role::kToolbar") << QString("<div role='toolbar'>a</div>") << 0 << QAccessible::ToolBar;
    QTest::newRow("ax::mojom::Role::kToolTip") << QString("<div role='tooltip'>a</div>") << 0 << QAccessible::ToolTip;
    QTest::newRow("ax::mojom::Role::kTree") << QString("<div role='tree'>a</div>") << 0 << QAccessible::Tree;
    QTest::newRow("ax::mojom::Role::kTreeGrid") << QString("<div role='treegrid'>a</div>") << 0 << QAccessible::Tree;
    QTest::newRow("ax::mojom::Role::kTreeItem") << QString("<div role='treeitem'>a</div>") << 0 << QAccessible::TreeItem;
    QTest::newRow("ax::mojom::Role::kVideo") << QString("<video><source src='test.mp4' type='video/mp4'></video>") << 1 << QAccessible::Animation;
    //QTest::newRow("ax::mojom::Role::kWindow"); // No mapping to ARIA role
}

void tst_Accessibility::roles()
{
    QFETCH(QString, html);
    QFETCH(int, nested);
    QFETCH(QAccessible::Role, role);

    QWebEngineView webView;
    webView.setHtml("<html><body>" + html + "</body></html>");
    webView.show();
    QSignalSpy spyFinished(&webView, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);

    // Corner case for Client role
    if (html.isEmpty()) {
        QCOMPARE(view->role(), QAccessible::Client);
        return;
    }

    QTRY_COMPARE(view->child(0)->childCount(), 1);
    QAccessibleInterface *document = view->child(0);
    QAccessibleInterface *element = document->child(0);

    while (nested--) {
        QTRY_VERIFY(element->child(0));
        element = element->child(0);
    }

    QCOMPARE(element->role(), role);
}

static QByteArrayList params = QByteArrayList()
    << "--force-renderer-accessibility"
    << "--enable-features=AccessibilityExposeARIAAnnotations";

W_QTEST_MAIN(tst_Accessibility, params)
#include "tst_accessibility.moc"
