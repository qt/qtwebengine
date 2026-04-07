// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QPdfDocument>
#include <QPdfSearchModel>
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

QTEST_MAIN(tst_QPdfSearchModel)

#include "tst_qpdfsearchmodel.moc"
