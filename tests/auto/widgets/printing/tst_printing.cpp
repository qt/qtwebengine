/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QWebEngineView>
#include <QTemporaryDir>
#include <QTest>
#include <QSignalSpy>
#include <util.h>

#if defined(POPPLER_CPP)
#include <poppler-document.h>
#include <poppler-page.h>
#endif

class tst_Printing : public QObject
{
    Q_OBJECT
private slots:
    void printToPdfBasic();
    void printRequest();
#if defined(POPPLER_CPP) && defined(Q_OS_LINUX) && defined(__GLIBCXX__)
    void printToPdfPoppler();
#endif
};

void tst_Printing::printToPdfBasic()
{
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_qwebengineview-XXXXXX");
    QVERIFY(tempDir.isValid());
    QWebEngineView view;
    QSignalSpy spy(&view, &QWebEngineView::loadFinished);
    view.load(QUrl("qrc:///resources/basic_printing_page.html"));
    QTRY_VERIFY(spy.count() == 1);

    QSignalSpy savePdfSpy(view.page(), &QWebEnginePage::pdfPrintingFinished);
    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(0.0, 0.0, 0.0, 0.0));
    QString path = tempDir.path() + "/print_1_success.pdf";
    view.page()->printToPdf(path, layout);
    QTRY_VERIFY2(savePdfSpy.count() == 1, "Printing to PDF file failed without signal");

    QList<QVariant> successArguments = savePdfSpy.takeFirst();
    QVERIFY2(successArguments.at(0).toString() == path, "File path for first saved PDF does not match arguments");
    QVERIFY2(successArguments.at(1).toBool() == true, "Printing to PDF file failed though it should succeed");

#if !defined(Q_OS_WIN)
    path = tempDir.path() + "/print_//2_failed.pdf";
#else
    path = tempDir.path() + "/print_|2_failed.pdf";
#endif
    view.page()->printToPdf(path, QPageLayout());
    QTRY_VERIFY2(savePdfSpy.count() == 1, "Printing to PDF file failed without signal");

    QList<QVariant> failedArguments = savePdfSpy.takeFirst();
    QVERIFY2(failedArguments.at(0).toString() == path, "File path for second saved PDF does not match arguments");
    QVERIFY2(failedArguments.at(1).toBool() == false, "Printing to PDF file succeeded though it should fail");

    CallbackSpy<QByteArray> successfulSpy;
    view.page()->printToPdf(successfulSpy.ref(), layout);
    QVERIFY(successfulSpy.waitForResult().length() > 0);

    CallbackSpy<QByteArray> failedInvalidLayoutSpy;
    view.page()->printToPdf(failedInvalidLayoutSpy.ref(), QPageLayout());
    QCOMPARE(failedInvalidLayoutSpy.waitForResult().length(), 0);
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
     QTRY_VERIFY(loadFinishedSpy.count() == 1);
     view.page()->runJavaScript("window.print()");
     QTRY_VERIFY(printRequestedSpy.count() == 1);
     QVERIFY(printRequestedSpy2.count() == 1);
     //check if printing still works
     view.printToPdf(resultSpy.ref(), layout);
     const QByteArray data = resultSpy.waitForResult();
     QVERIFY(data.length() > 0);
}

#if defined(POPPLER_CPP) && defined(Q_OS_LINUX) && defined(__GLIBCXX__)
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


QTEST_MAIN(tst_Printing)
#include "tst_printing.moc"
