/*
    Copyright (C) 2024 The Qt Company Ltd.

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

#include <QtWebEngineCore/qwebengineframe.h>

class tst_QWebEngineFrame : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void mainFrame();
    void findFrameByName();
    void isValid();
    void name();
    void htmlName();
    void children();
    void childrenOfInvalidFrame();
    void url();
    void size();
    void runJavaScript();

private:
};

void tst_QWebEngineFrame::mainFrame()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/frameset.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto frame = page.mainFrame();
    QVERIFY(frame.isValid());
}

void tst_QWebEngineFrame::findFrameByName()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/iframes.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto maybeFrame = page.findFrameByName("test-subframe0");
    QVERIFY(maybeFrame.has_value());
    QCOMPARE(maybeFrame->name(), "test-subframe0");
    QVERIFY(!page.findFrameByName("foobar").has_value());
}

void tst_QWebEngineFrame::isValid()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/iframes.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto firstPageSubframe = page.findFrameByName("test-subframe0");
    QVERIFY(firstPageSubframe && firstPageSubframe->isValid());

    page.load(QUrl("qrc:/resources/frameset.html"));
    QTRY_COMPARE(loadSpy.size(), 2);
    QVERIFY(!firstPageSubframe->isValid());
}

void tst_QWebEngineFrame::name()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/frameset.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(page.mainFrame().name(), "test-main-frame");
    auto children = page.mainFrame().children();
    QCOMPARE(children.at(0).name(), "test-subframe0");
    QCOMPARE(children.at(1).name(), "test-subframe1");
    QCOMPARE(children.at(2).name(), "test-subframe2");

    page.load(QUrl("qrc:/resources/iframes.html"));
    QTRY_COMPARE(loadSpy.size(), 2);
    QVERIFY(!children.at(0).isValid());
    QCOMPARE(children.at(0).name(), QString());
}

void tst_QWebEngineFrame::htmlName()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/iframes.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto children = page.mainFrame().children();
    QCOMPARE(children.at(0).name(), "test-subframe0");
    QCOMPARE(children.at(0).htmlName(), "iframe0-300x200");
    QCOMPARE(children.at(1).name(), "test-subframe1");
    QCOMPARE(children.at(1).htmlName(), "iframe1-350x250");

    page.load(QUrl("qrc:/resources/frameset.html"));
    QTRY_COMPARE(loadSpy.size(), 2);
    QVERIFY(!children.at(0).isValid());
    QCOMPARE(children.at(0).htmlName(), QString());
}

void tst_QWebEngineFrame::children()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/frameset.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto frame = page.mainFrame();
    auto children = frame.children();
    QCOMPARE(children.size(), 3);
    for (auto child : children) {
        QVERIFY(child.isValid());
    }
}

void tst_QWebEngineFrame::childrenOfInvalidFrame()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/nesting-iframe.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto nestedFrame = page.mainFrame().children().at(0);
    QCOMPARE(nestedFrame.children().size(), 2);

    page.load(QUrl("qrc:/resources/frameset.html"));
    QTRY_COMPARE(loadSpy.size(), 2);
    QVERIFY(!nestedFrame.isValid());
    QVERIFY(nestedFrame.children().empty());
}

void tst_QWebEngineFrame::url()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/nesting-iframe.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto children = page.mainFrame().children();
    QCOMPARE(children.at(0).url(), QUrl("qrc:/resources/iframes.html"));

    page.load(QUrl("qrc:/resources/frameset.html"));
    QTRY_COMPARE(loadSpy.size(), 2);
    QVERIFY(!children.at(0).isValid());
    QCOMPARE(children.at(0).url(), QUrl());
}

void tst_QWebEngineFrame::size()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/iframes.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto frame1 = *page.findFrameByName("test-subframe0");
    auto size1 = frame1.size();
    auto size2 = page.findFrameByName("test-subframe1")->size();
    QCOMPARE(size1, QSizeF(300, 200));
    QCOMPARE(size2, QSizeF(350, 250));

    page.load(QUrl("qrc:/resources/frameset.html"));
    QTRY_COMPARE(loadSpy.size(), 2);
    QVERIFY(!frame1.isValid());
    QCOMPARE(frame1.size(), QSizeF());
}

void tst_QWebEngineFrame::runJavaScript()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/iframes.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto children = page.mainFrame().children();
    CallbackSpy<QVariant> spy;
    children[0].runJavaScript("window.name", spy.ref());
    auto result = spy.waitForResult();
    QCOMPARE(result, QString("test-subframe0"));
}

QTEST_MAIN(tst_QWebEngineFrame)

#include "tst_qwebengineframe.moc"
