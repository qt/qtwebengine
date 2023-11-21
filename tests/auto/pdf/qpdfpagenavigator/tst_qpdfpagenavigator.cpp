// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#include <QtTest/QtTest>

#include <QPdfDocument>
#include <QPdfView>
#include <QPdfPageNavigator>

class tst_QPdfPageNavigator: public QObject
{
    Q_OBJECT

private slots:
    void offScreenSignals();
};

void tst_QPdfPageNavigator::offScreenSignals()
{
    QPdfDocument document;
    QPdfView pdfView;
    QPdfPageNavigator *navigator = pdfView.pageNavigator();

    QSignalSpy currentPageChanged(navigator, &QPdfPageNavigator::currentPageChanged);
    QSignalSpy currentLocationChanged(navigator, &QPdfPageNavigator::currentLocationChanged);
    QSignalSpy backAvailableChanged(navigator, &QPdfPageNavigator::backAvailableChanged);
    QSignalSpy forwardAvailableChanged(navigator, &QPdfPageNavigator::forwardAvailableChanged);
    QSignalSpy jumped(navigator, &QPdfPageNavigator::jumped);

    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks_pages.pdf")), QPdfDocument::Error::None);
    QVERIFY2(document.pageCount() == 3, "Test document has changed! 3 pages expected.");
    pdfView.setDocument(&document);

    // Start with a clean history
    QCOMPARE(forwardAvailableChanged.count(), 0);
    QCOMPARE(backAvailableChanged.count(), 0);

    navigator->jump(3, QPoint());
    QCOMPARE(forwardAvailableChanged.count(), 0);
    QCOMPARE(backAvailableChanged.count(), 1);
    QCOMPARE(currentPageChanged.count(), 1);
    QCOMPARE(currentLocationChanged.count(), 0);
    QCOMPARE(jumped.count(), 1);

    navigator->jump(1, QPoint());
    QCOMPARE(forwardAvailableChanged.count(), 0);
    QCOMPARE(backAvailableChanged.count(), 1);
    QCOMPARE(currentPageChanged.count(), 2);
    QCOMPARE(currentLocationChanged.count(), 0);
    QCOMPARE(jumped.count(), 2);

    navigator->back();
    QCOMPARE(forwardAvailableChanged.count(), 1);
    QCOMPARE(backAvailableChanged.count(), 1);
    QCOMPARE(currentPageChanged.count(), 3);
    QCOMPARE(currentLocationChanged.count(), 0);
    QCOMPARE(jumped.count(), 3);

    navigator->forward();
    QCOMPARE(forwardAvailableChanged.count(), 2);
    QCOMPARE(backAvailableChanged.count(), 1);
    QCOMPARE(currentPageChanged.count(), 4);
    QCOMPARE(currentLocationChanged.count(), 0);
    QCOMPARE(jumped.count(), 4);
}

QTEST_MAIN(tst_QPdfPageNavigator)

#include "tst_qpdfpagenavigator.moc"
