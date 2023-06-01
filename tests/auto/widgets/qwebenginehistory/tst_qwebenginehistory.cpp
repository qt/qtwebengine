/*
    Copyright (C) 2008 Holger Hans Peter Freyther

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
#include <QAction>

#include <util.h>
#include "qwebenginepage.h"
#include "qwebengineview.h"
#include "qwebenginehistory.h"
#include "qdebug.h"

class tst_QWebEngineHistory : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineHistory();
    virtual ~tst_QWebEngineHistory();

protected :
    void loadPage(int nr)
    {
        loadFinishedSpy->clear();
        page->load(QUrl("qrc:/resources/page" + QString::number(nr) + ".html"));
        QTRY_COMPARE(loadFinishedSpy->size(), 1);
        loadFinishedSpy->clear();
    }

public Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void title();
    void lastVisited();
    void iconUrl();
    void count();
    void back();
    void forward();
    void itemAt();
    void goToItem();
    void items();
    void backForwardItems();
    void serialize_1(); //QWebEngineHistory countity
    void serialize_2(); //QWebEngineHistory index
    void serialize_3(); //QWebEngineHistoryItem
    // Those tests shouldn't crash
    void saveAndRestore_crash_1();
    void saveAndRestore_crash_2();
    void saveAndRestore_crash_3();
    void saveAndRestore_crash_4();
    void saveAndRestore_InternalPage();

    void popPushState_data();
    void popPushState();
    void clear();
    void historyItemFromDeletedPage();
    void restoreIncompatibleVersion1();


private:
    QWebEnginePage* page;
    QWebEngineHistory* hist;
    QScopedPointer<QSignalSpy> loadFinishedSpy;
    int histsize;
};

tst_QWebEngineHistory::tst_QWebEngineHistory()
{
}

tst_QWebEngineHistory::~tst_QWebEngineHistory()
{
}

void tst_QWebEngineHistory::initTestCase()
{
}

void tst_QWebEngineHistory::init()
{
    page = new QWebEnginePage(this);
    loadFinishedSpy.reset(new QSignalSpy(page, SIGNAL(loadFinished(bool))));

    for (int i = 1;i < 6;i++) {
        loadPage(i);
    }
    hist = page->history();
    histsize = 5;
}

void tst_QWebEngineHistory::cleanup()
{
    loadFinishedSpy.reset();
    delete page;
}

/**
  * Check QWebEngineHistoryItem::title() method
  */
void tst_QWebEngineHistory::title()
{
    QTRY_COMPARE(hist->currentItem().title(), QString("page5"));
}

void tst_QWebEngineHistory::lastVisited()
{
    // Check that the conversion from Chromium's internal time format went well.
    QVERIFY(qAbs(hist->itemAt(0).lastVisited().secsTo(QDateTime::currentDateTime())) < 60);
}

void tst_QWebEngineHistory::iconUrl()
{
    QTRY_COMPARE(hist->currentItem().iconUrl(), QUrl("qrc:/qt-project.org/qmessagebox/images/qtlogo-64.png"));
}

/**
  * Check QWebEngineHistory::count() method
  */
void tst_QWebEngineHistory::count()
{
    QTRY_COMPARE(hist->count(), histsize);
}

/**
  * Check QWebEngineHistory::back() method
  */
void tst_QWebEngineHistory::back()
{
    QSignalSpy titleChangedSpy(page, SIGNAL(titleChanged(const QString&)));

    for (int i = histsize;i > 1;i--) {
        QTRY_COMPARE(toPlainTextSync(page), QString("page") + QString::number(i));
        hist->back();
        QTRY_COMPARE(loadFinishedSpy->size(), histsize-i+1);
        QTRY_COMPARE(titleChangedSpy.size(), histsize-i+1);
    }
    //try one more time (too many). crash test
    hist->back();
    QTRY_COMPARE(toPlainTextSync(page), QString("page1"));
}

/**
  * Check QWebEngineHistory::forward() method
  */
void tst_QWebEngineHistory::forward()
{
    //rewind history :-)
    int histBackCount = 0;
    while (hist->canGoBack()) {
        hist->back();
        histBackCount++;
        QTRY_COMPARE(loadFinishedSpy->size(), histBackCount);
    }

    QSignalSpy titleChangedSpy(page, SIGNAL(titleChanged(const QString&)));
    for (int i = 1;i < histsize;i++) {
        QTRY_COMPARE(toPlainTextSync(page), QString("page") + QString::number(i));
        hist->forward();
        QTRY_COMPARE(loadFinishedSpy->size(), i+histBackCount);
        QTRY_COMPARE(titleChangedSpy.size(), i);
    }
    //try one more time (too many). crash test
    hist->forward();
    QTRY_COMPARE(toPlainTextSync(page), QString("page") + QString::number(histsize));
}

/**
  * Check QWebEngineHistory::itemAt() method
  */
void tst_QWebEngineHistory::itemAt()
{
    for (int i = 1;i < histsize;i++) {
        QTRY_COMPARE(hist->itemAt(i - 1).title(), QString("page") + QString::number(i));
        QVERIFY(hist->itemAt(i - 1).isValid());
    }
    //check out of range values
    QVERIFY(!hist->itemAt(-1).isValid());
    QVERIFY(!hist->itemAt(histsize).isValid());
}

/**
  * Check QWebEngineHistory::goToItem() method
  */
void tst_QWebEngineHistory::goToItem()
{
    QWebEngineHistoryItem current = hist->currentItem();

    hist->back();
    QTRY_COMPARE(loadFinishedSpy->size(), 1);

    hist->back();
    QTRY_COMPARE(loadFinishedSpy->size(), 2);

    QVERIFY(hist->currentItem().title() != current.title());

    hist->goToItem(current);
    QTRY_COMPARE(loadFinishedSpy->size(), 2);

    QTRY_COMPARE(hist->currentItem().title(), current.title());
}

/**
  * Check QWebEngineHistory::items() method
  */
void tst_QWebEngineHistory::items()
{
    QList<QWebEngineHistoryItem> items = hist->items();
    //check count
    QTRY_COMPARE(histsize, items.size());

    //check order
    for (int i = 1;i <= histsize;i++) {
        QTRY_COMPARE(items.at(i - 1).title(), QString("page") + QString::number(i));
    }
}

void tst_QWebEngineHistory::backForwardItems()
{
    hist->back();
    QTRY_COMPARE(loadFinishedSpy->size(), 1);

    hist->back();
    QTRY_COMPARE(loadFinishedSpy->size(), 2);

    QTRY_COMPARE(hist->items().size(), 5);
    QTRY_COMPARE(hist->backItems(100).size(), 2);
    QTRY_COMPARE(hist->backItems(1).size(), 1);
    QTRY_COMPARE(hist->forwardItems(100).size(), 2);
    QTRY_COMPARE(hist->forwardItems(1).size(), 1);
}

/**
  * Check history state after serialization (pickle, persistent..) method
  * Checks history size, history order
  */
void tst_QWebEngineHistory::serialize_1()
{
    QByteArray tmp;  //buffer
    QDataStream save(&tmp, QIODevice::WriteOnly); //here data will be saved
    QDataStream load(&tmp, QIODevice::ReadOnly); //from here data will be loaded

    save << *hist;
    QVERIFY(save.status() == QDataStream::Ok);
    QTRY_COMPARE(hist->count(), histsize);

    //check size of history
    //load next page to find differences
    loadPage(6);
    QTRY_COMPARE(hist->count(), histsize + 1);
    load >> *hist;
    QVERIFY(load.status() == QDataStream::Ok);
    QTRY_COMPARE(hist->count(), histsize);

    //check order of historyItems
    QList<QWebEngineHistoryItem> items = hist->items();
    for (int i = 1;i <= histsize;i++) {
        QTRY_COMPARE(items.at(i - 1).title(), QString("page") + QString::number(i));
    }
}

/**
  * Check history state after serialization (pickle, persistent..) method
  * Checks history currentIndex value
  */
void tst_QWebEngineHistory::serialize_2()
{
    QByteArray tmp;  //buffer
    QDataStream save(&tmp, QIODevice::WriteOnly); //here data will be saved
    QDataStream load(&tmp, QIODevice::ReadOnly); //from here data will be loaded

    // Force a "same document" navigation.
    page->load(page->url().toString() + QLatin1String("#dummyAnchor"));
    // "same document" navigation doesn't trigger loadFinished signal.
    QTRY_COMPARE(evaluateJavaScriptSync(page, "location.hash").toString(), QStringLiteral("#dummyAnchor"));

    int initialCurrentIndex = hist->currentItemIndex();

    hist->back();
    QTRY_VERIFY(evaluateJavaScriptSync(page, "location.hash").toString().isEmpty());
    hist->back();
    QTRY_COMPARE(loadFinishedSpy->size(), 1);
    hist->back();
    QTRY_COMPARE(loadFinishedSpy->size(), 2);
    //check if current index was changed (make sure that it is not last item)
    QVERIFY(hist->currentItemIndex() != initialCurrentIndex);
    //save current index
    int oldCurrentIndex = hist->currentItemIndex();

    save << *hist;
    QVERIFY(save.status() == QDataStream::Ok);
    load >> *hist;
    QVERIFY(load.status() == QDataStream::Ok);
    // Restoring the history will trigger a load.
    QTRY_COMPARE(loadFinishedSpy->size(), 3);

    //check current index
    QTRY_COMPARE(hist->currentItemIndex(), oldCurrentIndex);

    hist->forward();
    QTRY_COMPARE(loadFinishedSpy->size(), 4);
    hist->forward();
    QTRY_COMPARE(loadFinishedSpy->size(), 5);
    hist->forward();
    // In-page navigation, the last url was the page5.html
    QTRY_COMPARE(loadFinishedSpy->size(), 5);
    QTRY_COMPARE(hist->currentItemIndex(), initialCurrentIndex);
}

/**
  * Check history state after serialization (pickle, persistent..) method
  * Checks QWebEngineHistoryItem public property after serialization
  */
void tst_QWebEngineHistory::serialize_3()
{
    QByteArray tmp;  //buffer
    QDataStream save(&tmp, QIODevice::WriteOnly); //here data will be saved
    QDataStream load(&tmp, QIODevice::ReadOnly); //from here data will be loaded

    //prepare two different history items
    QWebEngineHistoryItem a = hist->currentItem();

    //check properties BEFORE serialization
    QString title(a.title());
    QDateTime lastVisited(a.lastVisited());
    QUrl originalUrl(a.originalUrl());
    QUrl url(a.url());
    QUrl iconUrl(a.iconUrl());

    save << *hist;
    QVERIFY(save.status() == QDataStream::Ok);
    QVERIFY(!load.atEnd());
    hist->clear();
    QVERIFY(hist->count() == 1);
    load >> *hist;
    QVERIFY(load.status() == QDataStream::Ok);
    QWebEngineHistoryItem b = hist->currentItem();

    //check properties AFTER serialization
    QTRY_COMPARE(b.title(), title);
    QTRY_COMPARE(b.lastVisited(), lastVisited);
    QTRY_COMPARE(b.originalUrl(), originalUrl);
    QTRY_COMPARE(b.url(), url);
    QTRY_COMPARE(b.iconUrl(), iconUrl);

    //Check if all data was read
    QVERIFY(load.atEnd());
}

static void saveHistory(QWebEngineHistory* history, QByteArray* in)
{
    in->clear();
    QDataStream save(in, QIODevice::WriteOnly);
    save << *history;
}

static void restoreHistory(QWebEngineHistory* history, QByteArray* out)
{
    QDataStream load(out, QIODevice::ReadOnly);
    load >> *history;
}

void tst_QWebEngineHistory::saveAndRestore_crash_1()
{
    QByteArray buffer;
    saveHistory(hist, &buffer);
    for (unsigned i = 0; i < 5; i++) {
        restoreHistory(hist, &buffer);
        saveHistory(hist, &buffer);
    }
}

void tst_QWebEngineHistory::saveAndRestore_crash_2()
{
    QByteArray buffer;
    saveHistory(hist, &buffer);
    QWebEnginePage page2(this);
    QWebEngineHistory* hist2 = page2.history();
    for (unsigned i = 0; i < 5; i++) {
        restoreHistory(hist2, &buffer);
        saveHistory(hist2, &buffer);
    }
}

void tst_QWebEngineHistory::saveAndRestore_crash_3()
{
    QByteArray buffer;
    saveHistory(hist, &buffer);
    QWebEnginePage page2(this);
    QWebEngineHistory* hist1 = hist;
    QWebEngineHistory* hist2 = page2.history();
    for (unsigned i = 0; i < 5; i++) {
        restoreHistory(hist1, &buffer);
        restoreHistory(hist2, &buffer);
        QVERIFY(hist1->count() == hist2->count());
        QVERIFY(hist1->count() == histsize);
        hist2->back();
        saveHistory(hist2, &buffer);
        hist2->clear();
    }
}

void tst_QWebEngineHistory::saveAndRestore_crash_4()
{
    QByteArray buffer;
    saveHistory(hist, &buffer);

    QScopedPointer<QWebEnginePage> page2(new QWebEnginePage(this));

    // Load the history in a new page, waiting for the load to finish.
    QSignalSpy loadFinishedSpy2(page2.data(), SIGNAL(loadFinished(bool)));
    QDataStream load(&buffer, QIODevice::ReadOnly);
    load >> *page2->history();
    QTRY_COMPARE(loadFinishedSpy2.size(), 1);
}

void tst_QWebEngineHistory::saveAndRestore_InternalPage()
{
    QWebEngineView view;
    view.show();
    QSignalSpy loadFinishedSpy(&view, &QWebEngineView::loadFinished);
    view.load(QUrl("view-source:http://qt.io"));
    QTRY_LOOP_IMPL((loadFinishedSpy.size() == 1), 30000, 200)
    if (loadFinishedSpy.size() != 1 || !loadFinishedSpy.at(0).at(0).toBool())
         QSKIP("Couldn't load page from network, skipping test.");

    // get history
    QByteArray data;
    QDataStream stream1(&data, QIODevice::WriteOnly);
    stream1 << *view.history();

    // restore history - this should not crash. see QTBUG-57826
    QDataStream stream2(data);
    stream2 >> *view.history();
}

void tst_QWebEngineHistory::popPushState_data()
{
    QTest::addColumn<QString>("script");
    QTest::newRow("pushState") << "history.pushState(123, \"foo\");";
    QTest::newRow("replaceState") << "history.replaceState(\"a\", \"b\");";
    QTest::newRow("back") << "history.back();";
    QTest::newRow("forward") << "history.forward();";
}

/** Crash test, WebKit bug 38840 (https://bugs.webkit.org/show_bug.cgi?id=38840) */
void tst_QWebEngineHistory::popPushState()
{
    QFETCH(QString, script);
    QWebEnginePage page;
    QSignalSpy spyLoadFinished(&page, SIGNAL(loadFinished(bool)));
    page.setHtml("<html><body>long live Qt!</body></html>");
    QTRY_COMPARE(spyLoadFinished.size(), 1);
    evaluateJavaScriptSync(&page, script);
}

/** ::clear */
void tst_QWebEngineHistory::clear()
{
    QByteArray buffer;

    QAction* actionBack = page->action(QWebEnginePage::Back);
    QVERIFY(actionBack->isEnabled());
    saveHistory(hist, &buffer);
    QVERIFY(hist->count() > 1);
    hist->clear();
    QVERIFY(hist->count() == 1);  // Leave current item.
    QVERIFY(!actionBack->isEnabled());

    QWebEnginePage page2(this);
    QWebEngineHistory* hist2 = page2.history();
    QCOMPARE(hist2->count(), 1);
    hist2->clear();
    QCOMPARE(hist2->count(), 1); // Do not change anything.
}

void tst_QWebEngineHistory::historyItemFromDeletedPage()
{
    const QList<QWebEngineHistoryItem> items = page->history()->items();
    delete page;
    page = 0;

    for (const QWebEngineHistoryItem &item : items) {
        QVERIFY(!item.isValid());
        QTRY_COMPARE(item.originalUrl(), QUrl());
        QTRY_COMPARE(item.url(), QUrl());
        QTRY_COMPARE(item.title(), QString());
        QTRY_COMPARE(item.lastVisited(), QDateTime());
    }
}

// static void dumpCurrentVersion(QWebEngineHistory* history)
// {
//     QByteArray buffer;
//     saveHistory(history, &buffer);
//     printf("    static const char version1Dump[] = {");
//     for (int i = 0; i < buffer.size(); ++i) {
//         bool newLine = !(i % 15);
//         bool last = i == buffer.size() - 1;
//         printf("%s0x%.2x%s", newLine ? "\n        " : "", (unsigned char)buffer[i], last ? "" : ", ");
//     }
//     printf("};\n");
// }

void tst_QWebEngineHistory::restoreIncompatibleVersion1()
{
    // Uncomment this code to generate a dump similar to the one below with the current stream version.
    // dumpCurrentVersion(hist);
    static const unsigned char version1Dump[] = {
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x32, 0x00, 0x71, 0x00, 0x72, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x2f, 0x00, 0x72, 0x00, 0x65,
        0x00, 0x73, 0x00, 0x6f, 0x00, 0x75, 0x00, 0x72, 0x00, 0x63, 0x00, 0x65, 0x00, 0x73, 0x00,
        0x2f, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x68,
        0x00, 0x74, 0x00, 0x6d, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x70, 0x00, 0x61, 0x00,
        0x67, 0x00, 0x65, 0x00, 0x31, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x71, 0x00, 0x72, 0x00, 0x63, 0x00, 0x3a, 0x00,
        0x2f, 0x00, 0x72, 0x00, 0x65, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x75, 0x00, 0x72, 0x00, 0x63,
        0x00, 0x65, 0x00, 0x73, 0x00, 0x2f, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00,
        0x31, 0x00, 0x2e, 0x00, 0x68, 0x00, 0x74, 0x00, 0x6d, 0x00, 0x6c, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32,
        0x00, 0x71, 0x00, 0x72, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x2f, 0x00, 0x72, 0x00, 0x65, 0x00,
        0x73, 0x00, 0x6f, 0x00, 0x75, 0x00, 0x72, 0x00, 0x63, 0x00, 0x65, 0x00, 0x73, 0x00, 0x2f,
        0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x68, 0x00,
        0x74, 0x00, 0x6d, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67,
        0x00, 0x65, 0x00, 0x32, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x71, 0x00, 0x72, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x2f,
        0x00, 0x72, 0x00, 0x65, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x75, 0x00, 0x72, 0x00, 0x63, 0x00,
        0x65, 0x00, 0x73, 0x00, 0x2f, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x32,
        0x00, 0x2e, 0x00, 0x68, 0x00, 0x74, 0x00, 0x6d, 0x00, 0x6c, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00,
        0x71, 0x00, 0x72, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x2f, 0x00, 0x72, 0x00, 0x65, 0x00, 0x73,
        0x00, 0x6f, 0x00, 0x75, 0x00, 0x72, 0x00, 0x63, 0x00, 0x65, 0x00, 0x73, 0x00, 0x2f, 0x00,
        0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x33, 0x00, 0x2e, 0x00, 0x68, 0x00, 0x74,
        0x00, 0x6d, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00,
        0x65, 0x00, 0x33, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x32, 0x00, 0x71, 0x00, 0x72, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x2f, 0x00,
        0x72, 0x00, 0x65, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x75, 0x00, 0x72, 0x00, 0x63, 0x00, 0x65,
        0x00, 0x73, 0x00, 0x2f, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x33, 0x00,
        0x2e, 0x00, 0x68, 0x00, 0x74, 0x00, 0x6d, 0x00, 0x6c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f,
        0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x71,
        0x00, 0x72, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x2f, 0x00, 0x72, 0x00, 0x65, 0x00, 0x73, 0x00,
        0x6f, 0x00, 0x75, 0x00, 0x72, 0x00, 0x63, 0x00, 0x65, 0x00, 0x73, 0x00, 0x2f, 0x00, 0x70,
        0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x34, 0x00, 0x2e, 0x00, 0x68, 0x00, 0x74, 0x00,
        0x6d, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65,
        0x00, 0x34, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x32, 0x00, 0x71, 0x00, 0x72, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x2f, 0x00, 0x72,
        0x00, 0x65, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x75, 0x00, 0x72, 0x00, 0x63, 0x00, 0x65, 0x00,
        0x73, 0x00, 0x2f, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x34, 0x00, 0x2e,
        0x00, 0x68, 0x00, 0x74, 0x00, 0x6d, 0x00, 0x6c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x71, 0x00,
        0x72, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x2f, 0x00, 0x72, 0x00, 0x65, 0x00, 0x73, 0x00, 0x6f,
        0x00, 0x75, 0x00, 0x72, 0x00, 0x63, 0x00, 0x65, 0x00, 0x73, 0x00, 0x2f, 0x00, 0x70, 0x00,
        0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x68, 0x00, 0x74, 0x00, 0x6d,
        0x00, 0x6c, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00,
        0x35, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x32, 0x00, 0x71, 0x00, 0x72, 0x00, 0x63, 0x00, 0x3a, 0x00, 0x2f, 0x00, 0x72, 0x00,
        0x65, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x75, 0x00, 0x72, 0x00, 0x63, 0x00, 0x65, 0x00, 0x73,
        0x00, 0x2f, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x35, 0x00, 0x2e, 0x00,
        0x68, 0x00, 0x74, 0x00, 0x6d, 0x00, 0x6c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    QByteArray version1(reinterpret_cast<const char*>(version1Dump), sizeof(version1Dump));
    QDataStream stream(&version1, QIODevice::ReadOnly);

    // This should fail to load, the history should be cleared and the stream should be broken.
    stream >> *hist;
    QEXPECT_FAIL("", "Behavior change: A broken stream won't clear the history in QtWebEngine.", Continue);
    QVERIFY(!hist->canGoBack());
    QVERIFY(!hist->canGoForward());
    QVERIFY(stream.status() == QDataStream::ReadCorruptData);
}

QTEST_MAIN(tst_QWebEngineHistory)
#include "tst_qwebenginehistory.moc"
