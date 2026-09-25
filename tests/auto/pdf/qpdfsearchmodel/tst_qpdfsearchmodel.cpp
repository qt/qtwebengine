// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QPdfDocument>
#include <QPdfSearchModel>
#include <QPdfSelection>
#include <QtPdf/private/qtpdfglobal_p.h>

Q_PDF_LOGGING_CATEGORY(lcTests, "qt.pdf.tests")

class tst_QPdfSearchModel: public QObject
{
    Q_OBJECT

public:
    tst_QPdfSearchModel() {}

private slots:
    void findText_data();
    void findText();
    void searchStringContext_data();
    void searchStringContext();
    void contextComesFromMatchLocation_data();
    void contextComesFromMatchLocation();
    void displayRoleShowsWhatWasFound_data();
    void displayRoleShowsWhatWasFound();
};

void tst_QPdfSearchModel::findText_data()
{
    QTest::addColumn<QString>("pdfPath");
    QTest::addColumn<QString>("searchString");
    QTest::addColumn<int>("expectedMatchCount");
    QTest::addColumn<int>("matchIndexToCheck");
    QTest::addColumn<int>("expectedRectangleCount");
    QTest::addColumn<int>("rectIndexToCheck");
    QTest::addColumn<QRect>("expectedMatchBounds");

    QTest::newRow("the search for ai") << QFINDTESTDATA("test.pdf")
            << "ai" << 3 << 0 << 1 << 0 << QRect(321, 202, 9, 11);
    QTest::newRow("rotated text") << QFINDTESTDATA("rotated_text.pdf")
            << "world!" << 2 << 0 << 1 << 0 << QRect(76, 102, 26, 28);
    QTest::newRow("displaced text") << QFINDTESTDATA("tagged_mcr_multipage.pdf")
            << "1" << 1 << 0 << 1 << 0 << QRect(34, 22, 3, 8);
}

void tst_QPdfSearchModel::findText()
{
    QFETCH(QString, pdfPath);
    QFETCH(QString, searchString);
    QFETCH(int, expectedMatchCount);
    QFETCH(int, matchIndexToCheck);
    QFETCH(int, expectedRectangleCount);
    QFETCH(int, rectIndexToCheck);
    QFETCH(QRect, expectedMatchBounds);

    QPdfDocument document;
    QPdfSearchModel model;
    QSignalSpy statusChangedSpy(&model, &QPdfSearchModel::statusChanged);
    model.setDocument(&document);
    QCOMPARE(statusChangedSpy.count(), 0);

    QCOMPARE(document.load(pdfPath), QPdfDocument::Error::None);
    QCOMPARE(model.status(), QPdfSearchModel::Status::Null);

    model.setSearchString(searchString);
    QTRY_COMPARE(model.status(), QPdfSearchModel::Status::Searching); // wait for the timer to start
    QCOMPARE(statusChangedSpy.count(), 1);

    QTRY_COMPARE(model.status(), QPdfSearchModel::Status::Finished); // wait for the timer to stop
    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(model.count(), expectedMatchCount);
    QPdfLink match = model.resultAtIndex(matchIndexToCheck);
    qCDebug(lcTests) << match;
    QList<QRectF> rects = match.rectangles();
    QCOMPARE(rects.size(), expectedRectangleCount);
    QCOMPARE(rects.at(rectIndexToCheck).toRect(), expectedMatchBounds);
}

void tst_QPdfSearchModel::searchStringContext_data()
{
    QTest::addColumn<QString>("searchString");
    QTest::addColumn<int>("resultIndex");
    QTest::addColumn<QString>("endOfContextBefore");
    QTest::addColumn<QString>("startOfContextAfter");

    QTest::addRow("normal") << "rst" << 0 << "opq" << "uvw";
    QTest::addRow("empty before") << "abc" << 0 << "" << "def";
    QTest::addRow("empty after") << "XYZ" << 1 << "UVW" << "";
    QTest::addRow("search string in ctx after") << "aa" << 0 << "XYZ⏎" << "bb⏎ccxdd⏎aa";
    QTest::addRow("search string in ctx before") << "aa" << 1 << "aabb⏎ccxdd⏎" << "bb⏎abc";
    // pdfium matches a space in the search string against the line break
    QTest::addRow("across a line break") << "bb cc" << 0 << "XYZ⏎aa" << "xdd⏎aabb";
}

void tst_QPdfSearchModel::searchStringContext()
{
    QFETCH(QString, searchString);
    QFETCH(int, resultIndex);
    QFETCH(QString, endOfContextBefore);
    QFETCH(QString, startOfContextAfter);

    const QString pdfPath = QFINDTESTDATA("search_string_context.pdf");
    QPdfDocument document;
    QPdfSearchModel model;
    model.setDocument(&document);
    document.load(pdfPath);
    QCOMPARE(document.load(pdfPath), QPdfDocument::Error::None);
    QCOMPARE(model.status(), QPdfSearchModel::Status::Null);

    model.setSearchString(searchString);
    QTRY_COMPARE(model.status(), QPdfSearchModel::Status::Finished);
    QCOMPARE_GE(model.count(), resultIndex);

    const auto res = model.resultAtIndex(resultIndex);
    QVERIFY(res.contextBefore().endsWith(endOfContextBefore));
    QVERIFY(res.contextAfter().startsWith(startOfContextAfter));
}

void tst_QPdfSearchModel::contextComesFromMatchLocation_data()
{
    QTest::addColumn<QString>("pdfPath");
    QTest::addColumn<QString>("searchString");

    QTest::newRow("the") << QFINDTESTDATA("test.pdf") << "the";
    QTest::newRow("single letter") << QFINDTESTDATA("test.pdf") << "e";
    QTest::newRow("rotated text") << QFINDTESTDATA("rotated_text.pdf") << "e";
    QTest::newRow("displaced text") << QFINDTESTDATA("tagged_mcr_multipage.pdf") << "1";
}

/*
    A result is displayed as contextBefore + searchString + contextAfter, so that
    sequence has to occur in the text of the page on which the result was found.
    It did not always: the context used to be read from wherever a hit test on the
    corner of the result's bounding box happened to land, which could be anywhere
    on the page, or nowhere at all.
*/
void tst_QPdfSearchModel::contextComesFromMatchLocation() // QTBUG-150872
{
    QFETCH(QString, pdfPath);
    QFETCH(QString, searchString);

    QPdfDocument document;
    QPdfSearchModel model;
    model.setDocument(&document);
    QCOMPARE(document.load(pdfPath), QPdfDocument::Error::None);

    model.setSearchString(searchString);

    // the context renders newlines the way doSearch() does
    const auto asContext = [](QString text) {
        text.replace(QLatin1Char('\n'), QStringLiteral("\u23CE"));
        text.remove(QLatin1Char('\r'));
        return text;
    };

    int resultCount = 0;
    for (int page = 0; page < document.pageCount(); ++page) {
        const QString pageText = asContext(document.getAllText(page).text());
        // resultsOnPage() searches the page if that has not been done yet, so
        // there is no need to wait for the background search to get there
        const auto results = model.resultsOnPage(page);
        resultCount += results.size();
        for (int i = 0; i < results.size(); ++i) {
            const QPdfLink &result = results.at(i);
            const QString before = result.contextBefore();
            const QString after = result.contextAfter();
            bool found = false;
            for (int pos = pageText.indexOf(before); pos >= 0 && !found;
                 pos = pageText.indexOf(before, pos + 1)) {
                const int matchStart = pos + before.size();
                found = pageText.mid(matchStart, searchString.size())
                                .compare(searchString, Qt::CaseInsensitive) == 0
                        && QStringView(pageText).mid(matchStart + searchString.size())
                                .startsWith(after);
            }
            QVERIFY2(found, qPrintable(QString::fromLatin1(
                    "result %1 on page %2 shows \"%3[%4]%5\", which is not on that page")
                    .arg(i).arg(page).arg(before.right(40), searchString, after.left(40))));
        }
    }
    QCOMPARE_GT(resultCount, 0);
}

void tst_QPdfSearchModel::displayRoleShowsWhatWasFound_data()
{
    QTest::addColumn<QString>("pdfPath");
    QTest::addColumn<QString>("searchString");
    QTest::addColumn<QString>("expectedMarkup");

    // the document says "Trolls", whatever case it was searched for in
    QTest::newRow("other case") << QFINDTESTDATA("test.pdf") << "TROLLS" << "<b>Trolls</b>";
    QTest::newRow("same case") << QFINDTESTDATA("test.pdf") << "Trolls" << "<b>Trolls</b>";
    // a space in the search string matches the line break in the document
    QTest::newRow("across a line break") << QFINDTESTDATA("search_string_context.pdf")
                                         << "bb cc" << "<b>bb⏎cc</b>";
}

/*
    A row shows the text that was found, not the text that was searched for:
    those differ in case, and in the characters pdfium skips over while matching.
*/
void tst_QPdfSearchModel::displayRoleShowsWhatWasFound()
{
    QFETCH(QString, pdfPath);
    QFETCH(QString, searchString);
    QFETCH(QString, expectedMarkup);

    QPdfDocument document;
    QCOMPARE(document.load(pdfPath), QPdfDocument::Error::None);
    QPdfSearchModel model;
    model.setDocument(&document);
    model.setSearchString(searchString);
    for (int page = 0; page < document.pageCount(); ++page)
        model.resultsOnPage(page); // searches the page if that has not happened yet
    QCOMPARE_GT(model.rowCount(QModelIndex()), 0);

    const QString display = model.data(model.index(0), Qt::DisplayRole).toString();
    qCDebug(lcTests) << display;
    QVERIFY2(display.contains(expectedMarkup),
             qPrintable(QString::fromLatin1("\"%1\" does not contain %2")
                                .arg(display, expectedMarkup)));
}

QTEST_MAIN(tst_QPdfSearchModel)

#include "tst_qpdfsearchmodel.moc"
