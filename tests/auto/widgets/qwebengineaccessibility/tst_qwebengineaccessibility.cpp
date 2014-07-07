/*
    Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
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

#include <qtest.h>
#include "../util.h"

#include <qaccessible.h>
#include <qwebengineview.h>
#include <qwebenginepage.h>
#include <qwidget.h>

class tst_QWebEngineView : public QObject
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
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_QWebEngineView::initTestCase()
{
    QWebEngineWidgets::initialize();
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
}

void tst_QWebEngineView::noPage()
{
    QWebEngineView webView;
    webView.show();

    QTest::qWait(1000);
    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);
    QVERIFY(view);
    QCOMPARE(view->childCount(), 1);
    QAccessibleInterface *document = view->child(0);
    QCOMPARE(document->role(), QAccessible::Document);
    QCOMPARE(document->parent(), view);
    QCOMPARE(document->childCount(), 0);
}

void tst_QWebEngineView::hierarchy()
{
    QWebEngineView webView;
    QWebEnginePage page;
    page.setHtml("<html><body>" \
        "Hello world" \
        "<input type='text'/><br>" \
        "</body></html>");
    webView.setPage(&page);
    webView.show();

    // FIXME: don't use qWait but the loadFinished signal
    // I didn't get that to work for some reason
    QTest::qWait(2000);
    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);
    QVERIFY(view);
    QCOMPARE(view->childCount(), 1);
    QAccessibleInterface *document = view->child(0);
    QCOMPARE(document->role(), QAccessible::Document);
    QCOMPARE(document->parent(), view);
    QCOMPARE(document->childCount(), 1);
    QAccessibleInterface *grouping = document->child(0);
    QVERIFY(grouping);
    QCOMPARE(grouping->parent(), document);
    QCOMPARE(grouping->childCount(), 2);
    QAccessibleInterface *text = grouping->child(0);
    QCOMPARE(text->role(), QAccessible::StaticText);
    QCOMPARE(text->parent(), grouping);
    QEXPECT_FAIL("", "FIXME: static text should probably not have a child element", Continue);
    QCOMPARE(text->childCount(), 0);
    QAccessibleInterface *input = grouping->child(1);
    QCOMPARE(input->role(), QAccessible::EditableText);
    QCOMPARE(input->parent(), grouping);
    QCOMPARE(input->childCount(), 0);
}

QTEST_MAIN(tst_QWebEngineView)
#include "tst_qwebengineaccessibility.moc"
