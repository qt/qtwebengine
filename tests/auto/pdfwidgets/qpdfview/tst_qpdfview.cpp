// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QtCore/QLoggingCategory>
#include <QtGui/QImage>
#include <QtWidgets/QScrollBar>
#include <QtPdf/QPdfDocument>
#include <QtPdf/QPdfLink>
#include <QtPdf/QPdfPageNavigator>
#include <QtPdf/QPdfSearchModel>
#include <QtPdf/private/qtpdfglobal_p.h>
#include <QtPdfWidgets/QPdfView>

Q_PDF_LOGGING_CATEGORY(lcTests, "qt.pdf.tests")

class tst_QPdfView : public QObject
{
    Q_OBJECT

private slots:
    void jumpingToASearchResultShowsIt_data();
    void jumpingToASearchResultShowsIt();
};

// QPdfView frames the current search result in cyan, so the extent of that
// color in the viewport is the part of the frame that the user can see.
static QRect currentResultOnScreen(const QImage &shot)
{
    QRect box;
    for (int y = 0; y < shot.height(); ++y) {
        for (int x = 0; x < shot.width(); ++x) {
            const QRgb pixel = shot.pixel(x, y);
            if (qBlue(pixel) > 200 && qGreen(pixel) > 200 && qRed(pixel) < 100)
                box = box.isNull() ? QRect(x, y, 1, 1) : box.united(QRect(x, y, 1, 1));
        }
    }
    return box;
}

void tst_QPdfView::jumpingToASearchResultShowsIt_data()
{
    QTest::addColumn<QPdfView::PageMode>("pageMode");
    QTest::addColumn<QPdfView::ZoomMode>("zoomMode");
    QTest::addColumn<qreal>("zoomFactor");
    QTest::addColumn<bool>("jumpWithLocationOnly");

    QTest::newRow("multi page") << QPdfView::PageMode::MultiPage
                                << QPdfView::ZoomMode::FitToWidth << 1.0 << false;
    QTest::newRow("single page") << QPdfView::PageMode::SinglePage
                                 << QPdfView::ZoomMode::FitToWidth << 1.0 << false;
    // Zoomed in, the page is wider than the viewport as well as taller, so
    // getting to a result needs horizontal scrolling too.
    QTest::newRow("zoomed in") << QPdfView::PageMode::MultiPage
                               << QPdfView::ZoomMode::Custom << 3.0 << false;
    // A jump can be given a location rather than a whole result. The view then
    // knows where the result begins but not how far it reaches, so it can only
    // bring that point into view; the result still has to end up on screen.
    QTest::newRow("location only") << QPdfView::PageMode::MultiPage
                                   << QPdfView::ZoomMode::Custom << 3.0 << true;
}

/*!
    Going to a search result has to bring the result itself into view, not only
    the page that it is on.
*/
void tst_QPdfView::jumpingToASearchResultShowsIt()
{
    QFETCH(QPdfView::PageMode, pageMode);
    QFETCH(QPdfView::ZoomMode, zoomMode);
    QFETCH(qreal, zoomFactor);
    QFETCH(bool, jumpWithLocationOnly);

    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("qpdfwriter.pdf")), QPdfDocument::Error::None);
    QPdfSearchModel search;
    search.setDocument(&document);

    QWidget window;
    QPdfView view(&window);
    view.setDocument(&document);
    view.setPageMode(pageMode);
    view.setZoomMode(zoomMode);
    if (zoomMode == QPdfView::ZoomMode::Custom)
        view.setZoomFactor(zoomFactor);
    view.setSearchModel(&search);
    view.resize(400, 300); // small, so that most results are out of sight
    window.resize(400, 320);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    search.setSearchString(QStringLiteral("resolution"));
    QTRY_COMPARE(search.count(), 10); // occurrences of "resolution" in this file

    // either the whole result, or only where it begins
    const auto goTo = [&](const QPdfLink &result) {
        if (jumpWithLocationOnly)
            view.pageNavigator()->jump(result.page(), result.location());
        else
            view.pageNavigator()->jump(result);
    };

    int scrolledTo = 0;
    for (int i = 0; i < search.count(); ++i) {
        const QPdfLink result = search.resultAtIndex(i);

        // as if the user had gone to the page and been reading its left side
        view.pageNavigator()->jump(result.page(), {}, 0);
        view.horizontalScrollBar()->setValue(view.horizontalScrollBar()->minimum());
        const QPoint before(view.horizontalScrollBar()->value(),
                            view.verticalScrollBar()->value());

        view.setCurrentSearchResultIndex(i);
        goTo(result);
        const QPoint after(view.horizontalScrollBar()->value(),
                           view.verticalScrollBar()->value());
        qCDebug(lcTests) << "result" << i << "on page" << result.page() << "at"
                         << result.location() << ": scrolled" << before << "->" << after;
        if (after != before)
            ++scrolledTo;

        // The frame that QPdfView draws around the current result has to be
        // visible now, and the whole of it: this is what says that the view
        // really went to the result, rather than only that it scrolled
        // somewhere. A frame that runs off an edge of the viewport is cut off
        // in the grab, and its box ends flush against that edge.
        QImage shot = view.viewport()->grab().toImage();
        QTRY_VERIFY2([&]{ shot = view.viewport()->grab().toImage();
                          return !currentResultOnScreen(shot).isNull(); }(),
                     qPrintable(QString::fromLatin1("result %1 on page %2 is not on screen")
                                        .arg(i).arg(result.page())));
        const QRect box = currentResultOnScreen(shot);
        const QString where = QString::fromLatin1("result %1 on page %2: frame at %3 of %4x%5")
                                      .arg(i).arg(result.page()).arg(QDebug::toString(box))
                                      .arg(shot.width()).arg(shot.height());
        // Only a jump that was given the whole result can be expected to show
        // all of it: from a location alone, the view cannot know how far the
        // result reaches, and a long one can still go past the edge.
        if (!jumpWithLocationOnly) {
            const auto *hbar = view.horizontalScrollBar();
            const auto *vbar = view.verticalScrollBar();
            if (hbar->value() > hbar->minimum())
                QVERIFY2(box.left() > 0, qPrintable(where));
            if (hbar->value() < hbar->maximum())
                QVERIFY2(box.right() < shot.width() - 1, qPrintable(where));
            if (vbar->value() > vbar->minimum())
                QVERIFY2(box.top() > 0, qPrintable(where));
            if (vbar->value() < vbar->maximum())
                QVERIFY2(box.bottom() < shot.height() - 1, qPrintable(where));
        }

        // and it is completely visible, so going there again has nothing to do
        goTo(result);
        QCOMPARE(view.horizontalScrollBar()->value(), after.x());
        QCOMPARE(view.verticalScrollBar()->value(), after.y());
    }
    // the viewport is only 300 pixels high, so results had to be scrolled to
    QCOMPARE_GT(scrolledTo, 0);
}

QTEST_MAIN(tst_QPdfView)

#include "tst_qpdfview.moc"
