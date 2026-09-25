// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtCore/QLoggingCategory>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>
#include <QtPdf/QPdfLink>
#include <QtPdf/QPdfSearchModel>
#include <QtPdf/private/qtpdfglobal_p.h>
#include <QtPdfQuick/private/qquickpdfpageimage_p.h>
#include <QtPdfQuick/private/qquickpdfsearchmodel_p.h>
#include "../shared/util.h"

using namespace Qt::StringLiterals;

Q_PDF_LOGGING_CATEGORY(lcTests, "qt.pdf.tests")

class tst_ScrollablePageView : public QQuickDataTest
{
    Q_OBJECT

private Q_SLOTS:
    void goToSearchResultKeepsZoom();
    void initialSearchResult_data();
    void initialSearchResult();
};

/*
    A search result does not ask for any particular zoom level: its link holds
    zero. Going to one used to set renderScale to zero all the same, and since
    the page image is sized in points times renderScale, the page then had no
    size at all and was not rendered.
*/
void tst_ScrollablePageView::goToSearchResultKeepsZoom() // QTBUG-150873
{
    QQuickView window;
    QVERIFY(showView(window, testFileUrl("scrollablePageView.qml")));
    QQuickItem *pdfView = window.rootObject();
    QVERIFY(pdfView);
    QVERIFY(pdfView->setProperty("source", testFileUrl("qpdfwriter.pdf")));
    QTRY_COMPARE(pdfView->property("status").toInt(), QQuickPdfPageImage::Ready);
    QCOMPARE(pdfView->property("renderScale").toReal(), 1);
    const int startingPage = pdfView->property("currentPage").toInt();

    QPdfSearchModel *searchModel = pdfView->property("searchModel").value<QPdfSearchModel *>();
    QVERIFY(searchModel);
    QVERIFY(pdfView->setProperty("searchString", u"resolution"_s));
    // wait for every page to be searched, so that the results below are all there
    QTRY_COMPARE(searchModel->count(), 10); // occurrences of "resolution" in this file

    int pageChanges = 0;
    for (int i = 0; i < searchModel->count(); ++i) {
        QVERIFY(searchModel->setProperty("currentResult", i));
        QTRY_COMPARE(pdfView->property("status").toInt(), QQuickPdfPageImage::Ready);
        const int page = pdfView->property("currentPage").toInt();
        qCDebug(lcTests) << "result" << i << "is on page" << page
                         << "renderScale" << pdfView->property("renderScale").toReal();
        if (page != startingPage)
            ++pageChanges;
        QCOMPARE(pdfView->property("renderScale").toReal(), 1);
    }
    // if no result was on another page, jumping was never exercised
    QCOMPARE_GT(pageChanges, 0);
}

void tst_ScrollablePageView::initialSearchResult_data()
{
    QTest::addColumn<QLatin1StringView>("methodName");
    QTest::addColumn<int>("expectedFirstStringResultIndex");
    QTest::addColumn<int>("expectedFirstStringResultPage");
    QTest::addColumn<int>("expectedSecondStringResultIndex");
    QTest::addColumn<int>("expectedSecondStringResultPage");

    QTest::newRow("forward") << "searchForward"_L1 << 1 << 0 << 1 << 0;
    QTest::newRow("back") << "searchBack"_L1 << 9 << 3 << 106 << 3;
}

/*
    currentResult is initialized to 0: index of the first result.
    As soon as the user types a search string that is found,
    the view should probably jump to the first result already, but so far it doesn't.
    Searching forward goes to the next result (index 1).
    Searching backwards from the first result goes to the last result on the last page.
*/
void tst_ScrollablePageView::initialSearchResult() // QTBUG-136476
{
    QFETCH(QLatin1StringView, methodName);
    QFETCH(int, expectedFirstStringResultIndex);
    QFETCH(int, expectedFirstStringResultPage);
    QFETCH(int, expectedSecondStringResultIndex);
    QFETCH(int, expectedSecondStringResultPage);

    QQuickView window;
    QVERIFY(showView(window, testFileUrl("scrollablePageView.qml")));
    QQuickItem *pdfView = window.rootObject();
    QVERIFY(pdfView);
    QVERIFY(pdfView->setProperty("source", testFileUrl("qpdfwriter.pdf")));
    QTRY_COMPARE(pdfView->property("status").toInt(), QQuickPdfPageImage::Ready);

    QQuickPdfSearchModel *searchModel = pdfView->property("searchModel").value<QQuickPdfSearchModel *>();
    QVERIFY(searchModel);
    QSignalSpy currentResultChangedSpy(searchModel, &QQuickPdfSearchModel::currentResultChanged);
    QSignalSpy currentResultLinkChangedSpy(searchModel, &QQuickPdfSearchModel::currentResultLinkChanged);
    QCOMPARE(searchModel->property("currentResult").toInt(), 0);

    QVERIFY(pdfView->setProperty("searchString", u"unicorn"_s));
    QCOMPARE(searchModel->count(), 0); // no unicorns in this file
    QCOMPARE(searchModel->property("currentResult").toInt(), 0);
    QCOMPARE(currentResultChangedSpy.count(), 0);
    QCOMPARE(currentResultLinkChangedSpy.count(), 1);

    QVERIFY(pdfView->setProperty("searchString", u"resolution"_s));
    QTRY_COMPARE(searchModel->count(), 10); // occurrences of "resolution" in this file
    QCOMPARE(searchModel->property("currentResult").toInt(), 0);
    QCOMPARE(currentResultChangedSpy.count(), 0);
    QCOMPARE(currentResultLinkChangedSpy.count(), 2);

    int pageWas = searchModel->currentPage();
    QVERIFY(QMetaObject::invokeMethod(pdfView, methodName.constData()));
    int resultIndex = searchModel->property("currentResult").toInt();
    QCOMPARE(resultIndex, expectedFirstStringResultIndex);
    QCOMPARE(currentResultChangedSpy.count(), 1);
    QCOMPARE(currentResultLinkChangedSpy.count(), searchModel->currentPage() == pageWas ? 3 : 4);
    QTRY_COMPARE(pdfView->property("currentPage").toInt(), searchModel->resultAtIndex(resultIndex).page());
    QCOMPARE(pdfView->property("currentPage").toInt(), expectedFirstStringResultPage);
    currentResultLinkChangedSpy.clear();

    // inputting a different search string resets currentResult
    pageWas = searchModel->currentPage();
    QVERIFY(pdfView->setProperty("searchString", u"page"_s));
    QCOMPARE(searchModel->property("currentResult").toInt(), 0);
    QCOMPARE(currentResultChangedSpy.count(), 2);
    QCOMPARE(currentResultLinkChangedSpy.count(), searchModel->currentPage() == pageWas ? 1 : 2);
    QCOMPARE(pdfView->property("currentPage").toInt(), 0);
    QTRY_COMPARE(searchModel->count(), 107); // occurrences of "page" in this file
    currentResultLinkChangedSpy.clear();

    pageWas = searchModel->currentPage();
    QVERIFY(QMetaObject::invokeMethod(pdfView, methodName.constData()));
    resultIndex = searchModel->property("currentResult").toInt();
    QCOMPARE(currentResultChangedSpy.count(), 3);
    QCOMPARE(currentResultLinkChangedSpy.count(), searchModel->currentPage() == pageWas ? 1 : 2);
    QCOMPARE(resultIndex, expectedSecondStringResultIndex);
    QTRY_COMPARE(pdfView->property("currentPage").toInt(), searchModel->resultAtIndex(resultIndex).page());
    QCOMPARE(pdfView->property("currentPage").toInt(), expectedSecondStringResultPage);
}

QTEST_MAIN(tst_ScrollablePageView)

#include "tst_scrollablepageview.moc"
