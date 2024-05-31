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
    void isMainFrame();
    void runJavaScript();
    void printRequestedByFrame();
    void printToPdfFile();
    void printToPdfFileFailures();
    void printToPdfFunction();

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

void tst_QWebEngineFrame::isMainFrame()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/frameset.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto frame = page.mainFrame();
    QVERIFY(frame.isMainFrame());
    for (auto child : frame.children()) {
        QVERIFY(!child.isMainFrame());
    }
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

void tst_QWebEngineFrame::printRequestedByFrame()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    QSignalSpy printRequestedSpy{ &page, SIGNAL(printRequestedByFrame(QWebEngineFrame)) };

    page.load(QUrl("qrc:/resources/nesting-iframe.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    auto oFrame2 = page.findFrameByName("test-main-frame");
    QVERIFY(oFrame2.has_value());
    CallbackSpy<QVariant> spy;
    oFrame2->runJavaScript("window.print()", spy.ref());
    spy.waitForResult();
    QCOMPARE(printRequestedSpy.size(), 1);
    auto *framePtr = get_if<QWebEngineFrame>(&printRequestedSpy[0][0]);
    QCOMPARE(*framePtr, *oFrame2);
}

void tst_QWebEngineFrame::printToPdfFile()
{
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_qwebengineframe-XXXXXX");
    QVERIFY(tempDir.isValid());

    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/printing-outer-document.html"));
    QTRY_COMPARE(loadSpy.size(), 1);

    auto outerFrame = page.mainFrame();
    auto maybeInnerFrame = page.findFrameByName("inner");
    QVERIFY(maybeInnerFrame);
    auto innerFrame = *maybeInnerFrame;

    QSignalSpy savePdfSpy{ &page, SIGNAL(pdfPrintingFinished(QString, bool)) };

    QString outerPath = tempDir.path() + "/outer.pdf";
    outerFrame.printToPdf(outerPath);
    QTRY_COMPARE(savePdfSpy.size(), 1);

    QList<QVariant> outerArgs = savePdfSpy.takeFirst();
    QCOMPARE(outerArgs.at(0).toString(), outerPath);
    QVERIFY(outerArgs.at(1).toBool());

    QString innerPath = tempDir.path() + "/inner.pdf";
    innerFrame.printToPdf(innerPath);
    QTRY_COMPARE(savePdfSpy.size(), 1);

    QList<QVariant> innerArgs = savePdfSpy.takeFirst();
    QCOMPARE(innerArgs.at(0).toString(), innerPath);
    QVERIFY(innerArgs.at(1).toBool());

    // The outer document encompasses more elements so its PDF should be larger. This is a
    // roundabout way to check that we aren't just printing the same document twice.
    auto outerSize = QFileInfo(outerPath).size();
    auto innerSize = QFileInfo(innerPath).size();
    QCOMPARE_GT(outerSize, innerSize);
    QCOMPARE_GT(innerSize, 0);
}

void tst_QWebEngineFrame::printToPdfFileFailures()
{
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_qwebengineframe-XXXXXX");
    QVERIFY(tempDir.isValid());

    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/printing-outer-document.html"));
    QTRY_COMPARE(loadSpy.size(), 1);

    auto maybeInnerFrame = page.findFrameByName("inner");
    QVERIFY(maybeInnerFrame);
    auto innerFrame = *maybeInnerFrame;

    QSignalSpy savePdfSpy{ &page, SIGNAL(pdfPrintingFinished(QString, bool)) };

#if !defined(Q_OS_WIN)
    auto badPath = tempDir.path() + "/print_//2_failed.pdf";
#else
    auto badPath = tempDir.path() + "/print_|2_failed.pdf";
#endif
    innerFrame.printToPdf(badPath);
    QTRY_COMPARE(savePdfSpy.size(), 1);

    QList<QVariant> badPathArgs = savePdfSpy.takeFirst();
    QCOMPARE(badPathArgs.at(0).toString(), badPath);
    QVERIFY(!badPathArgs.at(1).toBool());

    page.triggerAction(QWebEnginePage::WebAction::Reload);
    QTRY_COMPARE(loadSpy.size(), 2);

    QVERIFY(!innerFrame.isValid());
    QString invalidFramePath = tempDir.path() + "/invalidFrame.pdf";
    innerFrame.printToPdf(invalidFramePath);
    QTRY_COMPARE(savePdfSpy.size(), 1);

    QList<QVariant> invalidFrameArgs = savePdfSpy.takeFirst();
    QCOMPARE(invalidFrameArgs.at(0).toString(), invalidFramePath);
    QVERIFY(!invalidFrameArgs.at(1).toBool());
}

void tst_QWebEngineFrame::printToPdfFunction()
{
    QWebEnginePage page;
    QSignalSpy loadSpy{ &page, SIGNAL(loadFinished(bool)) };
    page.load(QUrl("qrc:/resources/printing-outer-document.html"));
    QTRY_COMPARE(loadSpy.size(), 1);

    auto outerFrame = page.mainFrame();
    auto maybeInnerFrame = page.findFrameByName("inner");
    QVERIFY(maybeInnerFrame);
    auto innerFrame = *maybeInnerFrame;

    CallbackSpy<QByteArray> outerSpy;
    outerFrame.printToPdf(outerSpy.ref());
    auto outerPdfData = outerSpy.waitForResult();
    QCOMPARE_GT(outerPdfData.size(), 0);

    CallbackSpy<QByteArray> innerSpy;
    innerFrame.printToPdf(innerSpy.ref());
    auto innerPdfData = innerSpy.waitForResult();
    QCOMPARE_GT(innerPdfData.size(), 0);
    QCOMPARE_GT(outerPdfData.size(), innerPdfData.size());

    page.triggerAction(QWebEnginePage::WebAction::Reload);
    QTRY_COMPARE(loadSpy.size(), 2);
    QVERIFY(!innerFrame.isValid());

    CallbackSpy<QByteArray> invalidSpy;
    innerFrame.printToPdf(invalidSpy.ref());
    auto invalidPdfData = invalidSpy.waitForResult();
    QVERIFY(invalidSpy.wasCalled());
    QCOMPARE(invalidPdfData.size(), 0);
}

QTEST_MAIN(tst_QWebEngineFrame)

#include "tst_qwebengineframe.moc"
