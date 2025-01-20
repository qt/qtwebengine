// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWebEngineCore/qtwebenginecore-config.h>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include <QTemporaryDir>
#include <QTest>
#include <QSignalSpy>
#include <util.h>

#ifdef QTPDF_SUPPORT
#include <QBuffer>
#include <QPdfDocument>
#include <QPdfSearchModel>
#endif

class tst_Printing : public QObject
{
    Q_OBJECT
private slots:
    void printToPdfBasic();
    void printRequest();
    void pdfContent();
    void printFromPdfViewer();
    void printHeaderAndFooter_data();
    void printHeaderAndFooter();
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

void tst_Printing::pdfContent()
{
#if !defined(QTPDF_SUPPORT)
    QSKIP("QtPdf is required, but missing");
#else
    // check if generated pdf is correct by searching for a know string on the page
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

    QPdfDocument document;
    QSignalSpy statusChangedSpy(&document, &QPdfDocument::statusChanged);

    QBuffer buffer;
    buffer.setData((data));
    QVERIFY2(buffer.open(QBuffer::ReadWrite), qPrintable(buffer.errorString()));
    document.load(&buffer);
    QTRY_COMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Ready);
    QCOMPARE(document.pageCount(), 1);

    QPdfSearchModel searchModel;
    searchModel.setDocument(&document);
    searchModel.setSearchString("Hello Paper World");
    QTRY_COMPARE(searchModel.count(), 1);
#endif
}

void tst_Printing::printFromPdfViewer()
{
#if !defined(QTPDF_SUPPORT)
    QSKIP("QtPdf is required, but missing");
#else
    QWebEngineView view;
    view.page()->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    view.page()->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);

    // Load a basic HTML
    QSignalSpy spy(&view, &QWebEngineView::loadFinished);
    view.load(QUrl("qrc:///resources/basic_printing_page.html"));
    QTRY_COMPARE(spy.size(), 1);

    // Create a PDF
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_printing-XXXXXX");
    QVERIFY(tempDir.isValid());
    QString path = tempDir.path() + "/basic_page.pdf";
    QSignalSpy savePdfSpy(view.page(), &QWebEnginePage::pdfPrintingFinished);
    view.page()->printToPdf(path);
    QTRY_COMPARE(savePdfSpy.size(), 1);

    // Open the new file with the PDF viewer plugin
    view.load(QUrl::fromLocalFile(path));
    QTRY_COMPARE(spy.size(), 2);

    // Print from the plugin
    // loadFinished signal is not reliable when loading a PDF file, because it has multiple phases.
    // Workaround: Try to print it a couple of times until the result matches the expected.
    CallbackSpy<QByteArray> resultSpy;
    bool ok = QTest::qWaitFor([&]() -> bool {
        view.printToPdf(resultSpy.ref());
        QByteArray data = resultSpy.waitForResult();

        // Check if the result contains text from the original basic HTML
        // This catches all the typical issues: empty result or printing the WebUI without PDF content.
        QPdfDocument document;
        QSignalSpy statusChangedSpy(&document, &QPdfDocument::statusChanged);

        QBuffer buffer;
        buffer.setData((data));
        if (!buffer.open(QBuffer::ReadWrite)) {
            qWarning("Failed to open buffer: %s", qPrintable(buffer.errorString()));
            return false;
        }
        document.load(&buffer);
        statusChangedSpy.wait(1000);
        if (document.status() != QPdfDocument::Status::Ready)
            return false;

        QPdfSearchModel searchModel;
        QSignalSpy countChangedSpy(&searchModel, &QPdfSearchModel::countChanged);
        searchModel.setDocument(&document);
        searchModel.setSearchString("Hello Paper World");
        countChangedSpy.wait(1000);
        if (searchModel.count() != 1)
            return false;

        return true;
    }, 30000);
    QVERIFY(ok);
#endif
}

void tst_Printing::printHeaderAndFooter_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::newRow("PrintHeaderAndFooter disabled") << false;
    QTest::newRow("PrintHeaderAndFooter enabled") << true;
}

void tst_Printing::printHeaderAndFooter()
{
#if !defined(QTPDF_SUPPORT)
    QSKIP("QtPdf is required, but missing");
#else
    QFETCH(bool, enabled);

    QWebEngineView view;
    QVERIFY(!view.page()->settings()->testAttribute(QWebEngineSettings::PrintHeaderAndFooter));
    view.page()->settings()->setAttribute(QWebEngineSettings::PrintHeaderAndFooter, enabled);

    QSignalSpy spy(&view, &QWebEngineView::loadFinished);
    view.load(QUrl("qrc:///resources/basic_printing_page.html"));
    QTRY_COMPARE(spy.size(), 1);

    CallbackSpy<QByteArray> resultSpy;
    QPageLayout pageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait,
                           QMarginsF(0, 30, 0, 30));
    view.printToPdf(resultSpy.ref(), pageLayout);
    const QByteArray data = resultSpy.waitForResult();
    QVERIFY(data.length() > 0);

    QPdfDocument document;
    QSignalSpy statusChangedSpy(&document, &QPdfDocument::statusChanged);

    QBuffer buffer;
    buffer.setData((data));
    QVERIFY2(buffer.open(QBuffer::ReadWrite), qPrintable(buffer.errorString()));
    document.load(&buffer);
    QTRY_COMPARE(document.status(), QPdfDocument::Status::Ready);

    QPdfSearchModel searchModel;
    QSignalSpy countChangedSpy(&searchModel, &QPdfSearchModel::countChanged);
    searchModel.setDocument(&document);
    searchModel.setSearchString("Basic Printing Page");
    countChangedSpy.wait(500);
    QTRY_COMPARE(searchModel.count() == 1, enabled);
#endif
}

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
