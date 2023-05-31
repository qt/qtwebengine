// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QtWebEngineCore/qtwebenginecore-config.h>
#include <QWebEngineView>
#include <QTemporaryDir>
#include <QTest>
#include <QSignalSpy>
#include <util.h>

#if QT_CONFIG(webengine_system_poppler)
#include <poppler-document.h>
#include <poppler-page.h>
#endif

class tst_Printing : public QObject
{
    Q_OBJECT
private slots:
    void printToPdfBasic();
    void printRequest();
#if QT_CONFIG(webengine_system_poppler)
    void printToPdfPoppler();
#endif
    void interruptPrinting();
};

void tst_Printing::printToPdfBasic()
{
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_qwebengineview-XXXXXX");
    QVERIFY(tempDir.isValid());
    QWebEngineView view;
    QSignalSpy spy(&view, &QWebEngineView::loadFinished);
    view.load(QUrl("qrc:///resources/basic_printing_page.html"));
    QTRY_VERIFY(spy.size() == 1);

    QSignalSpy savePdfSpy(view.page(), &QWebEnginePage::pdfPrintingFinished);
    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(0.0, 0.0, 0.0, 0.0));
    QString path = tempDir.path() + "/print_1_success.pdf";
    view.page()->printToPdf(path, layout);
    QTRY_VERIFY2(savePdfSpy.size() == 1, "Printing to PDF file failed without signal");

    QList<QVariant> successArguments = savePdfSpy.takeFirst();
    QVERIFY2(successArguments.at(0).toString() == path, "File path for first saved PDF does not match arguments");
    QVERIFY2(successArguments.at(1).toBool() == true, "Printing to PDF file failed though it should succeed");

#if !defined(Q_OS_WIN)
    path = tempDir.path() + "/print_//2_failed.pdf";
#else
    path = tempDir.path() + "/print_|2_failed.pdf";
#endif
    view.page()->printToPdf(path, QPageLayout());
    QTRY_VERIFY2(savePdfSpy.size() == 1, "Printing to PDF file failed without signal");

    QList<QVariant> failedArguments = savePdfSpy.takeFirst();
    QVERIFY2(failedArguments.at(0).toString() == path, "File path for second saved PDF does not match arguments");
    QVERIFY2(failedArguments.at(1).toBool() == false, "Printing to PDF file succeeded though it should fail");

    CallbackSpy<QByteArray> successfulSpy;
    view.page()->printToPdf(successfulSpy.ref(), layout);
    QVERIFY(successfulSpy.waitForResult().size() > 0);

    CallbackSpy<QByteArray> failedInvalidLayoutSpy;
    view.page()->printToPdf(failedInvalidLayoutSpy.ref(), QPageLayout());
    QCOMPARE(failedInvalidLayoutSpy.waitForResult().size(), 0);
}

void tst_Printing::printRequest()
{
     QWebEngineView view;
     QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(0.0, 0.0, 0.0, 0.0));
     QSignalSpy loadFinishedSpy(&view, &QWebEngineView::loadFinished);
     QSignalSpy printRequestedSpy(&view, &QWebEngineView::printRequested);
     QSignalSpy printRequestedSpy2(view.page(), &QWebEnginePage::printRequested);
     QSignalSpy savePdfSpy(&view, &QWebEngineView::pdfPrintingFinished);
     CallbackSpy<QByteArray> resultSpy;

     view.load(QUrl("qrc:///resources/basic_printing_page.html"));
     QTRY_VERIFY(loadFinishedSpy.size() == 1);
     view.page()->runJavaScript("window.print()");
     QTRY_VERIFY(printRequestedSpy.size() == 1);
     QVERIFY(printRequestedSpy2.size() == 1);
     //check if printing still works
     view.printToPdf(resultSpy.ref(), layout);
     const QByteArray data = resultSpy.waitForResult();
     QVERIFY(data.size() > 0);
}

#if QT_CONFIG(webengine_system_poppler)
void tst_Printing::printToPdfPoppler()
{
    // check if generated pdf is correct by searching for a know string on the page
    using namespace poppler;
    QWebEngineView view;
    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(0.0, 0.0, 0.0, 0.0));

    QSignalSpy spy(&view, &QWebEngineView::loadFinished);
    QSignalSpy savePdfSpy(&view, &QWebEngineView::pdfPrintingFinished);
    CallbackSpy<QByteArray> resultSpy;

    view.load(QUrl("qrc:///resources/basic_printing_page.html"));
    QTRY_VERIFY(spy.count() == 1);
    view.printToPdf(resultSpy.ref(), layout);
    const QByteArray data = resultSpy.waitForResult();
    QVERIFY(data.length() > 0);

    QScopedPointer<document> pdf(document::load_from_raw_data(data.constData(), data.length()));
    QVERIFY(pdf);

    const int pages = pdf->pages();
    QVERIFY(pages == 1);

    QScopedPointer<page> pdfPage(pdf->create_page(0));
    rectf rect;
    QVERIFY2(pdfPage->search(ustring::from_latin1("Hello Paper World"), rect, page::search_from_top,
                     case_sensitive ), "Could not find text");
}
#endif

void tst_Printing::interruptPrinting()
{
    QWebEngineView view;
    QSignalSpy spy(&view, &QWebEngineView::loadFinished);
    view.load(QUrl("qrc:///resources/basic_printing_page.html"));
    QTRY_VERIFY(spy.size() == 1);

    QTemporaryDir tempDir(QDir::tempPath() + "/tst_qwebengineview-XXXXXX");
    QVERIFY(tempDir.isValid());
    view.page()->printToPdf(tempDir.path() + "/file.pdf");
    // Navigation stop interrupts print job, preferably do this without crash/assert
    view.page()->triggerAction(QWebEnginePage::Stop);
}

QTEST_MAIN(tst_Printing)
#include "tst_printing.moc"
