// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtTest/QtTest>

#include <QPdfDocument>
#include <QPdfSearchModel>

Q_LOGGING_CATEGORY(lcTests, "qt.pdf.tests")

class tst_QPdfSearchModel: public QObject
{
    Q_OBJECT

public:
    tst_QPdfSearchModel() {}

private slots:
    void findText_data();
    void findText();
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
    QCOMPARE(document.load(pdfPath), QPdfDocument::Error::None);

    QPdfSearchModel model;
    model.setDocument(&document);
    model.setSearchString(searchString);

    QTRY_COMPARE(model.count(), expectedMatchCount); // wait for the timer
    QPdfLink match = model.resultAtIndex(matchIndexToCheck);
    qCDebug(lcTests) << match;
    QList<QRectF> rects = match.rectangles();
    QCOMPARE(rects.size(), expectedRectangleCount);
    QCOMPARE(rects.at(rectIndexToCheck).toRect(), expectedMatchBounds);
}

QTEST_MAIN(tst_QPdfSearchModel)

#include "tst_qpdfsearchmodel.moc"
