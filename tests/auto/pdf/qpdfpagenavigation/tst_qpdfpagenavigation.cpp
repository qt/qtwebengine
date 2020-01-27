/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtTest/QtTest>

#include <QPdfDocument>
#include <QPdfPageNavigation>

class tst_QPdfPageNavigation: public QObject
{
    Q_OBJECT

private slots:
    void defaultValues();
    void setEmptyDocument();
    void setEmptyDocumentAndLoad();
    void setLoadedDocument();
    void unloadDocument();
    void navigate();
};

void tst_QPdfPageNavigation::defaultValues()
{
    QPdfPageNavigation pageNavigation;

    QCOMPARE(pageNavigation.document(), nullptr);
    QCOMPARE(pageNavigation.currentPage(), 0);
    QCOMPARE(pageNavigation.pageCount(), 0);
    QCOMPARE(pageNavigation.canGoToPreviousPage(), false);
    QCOMPARE(pageNavigation.canGoToNextPage(), false);
}

void tst_QPdfPageNavigation::setEmptyDocument()
{
    QPdfDocument document;
    QPdfPageNavigation pageNavigation;

    pageNavigation.setDocument(&document);

    QCOMPARE(pageNavigation.document(), &document);
    QCOMPARE(pageNavigation.currentPage(), 0);
    QCOMPARE(pageNavigation.pageCount(), 0);
    QCOMPARE(pageNavigation.canGoToPreviousPage(), false);
    QCOMPARE(pageNavigation.canGoToNextPage(), false);
}

void tst_QPdfPageNavigation::setEmptyDocumentAndLoad()
{
    QPdfDocument document;
    QPdfPageNavigation pageNavigation;

    pageNavigation.setDocument(&document);

    QSignalSpy currentPageChangedSpy(&pageNavigation, &QPdfPageNavigation::currentPageChanged);
    QSignalSpy pageCountChangedSpy(&pageNavigation, &QPdfPageNavigation::pageCountChanged);
    QSignalSpy canGoToPreviousPageChangedSpy(&pageNavigation, &QPdfPageNavigation::canGoToPreviousPageChanged);
    QSignalSpy canGoToNextPageChangedSpy(&pageNavigation, &QPdfPageNavigation::canGoToNextPageChanged);

    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.pagenavigation.pdf")), QPdfDocument::NoError);

    QCOMPARE(currentPageChangedSpy.count(), 0); // current page stays '0'
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), 3);
    QCOMPARE(canGoToPreviousPageChangedSpy.count(), 0); // still no previous page available
    QCOMPARE(canGoToNextPageChangedSpy.count(), 1);
    QCOMPARE(canGoToNextPageChangedSpy[0][0].toBool(), true);
}

void tst_QPdfPageNavigation::setLoadedDocument()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.pagenavigation.pdf")), QPdfDocument::NoError);

    QPdfPageNavigation pageNavigation;

    QSignalSpy currentPageChangedSpy(&pageNavigation, &QPdfPageNavigation::currentPageChanged);
    QSignalSpy pageCountChangedSpy(&pageNavigation, &QPdfPageNavigation::pageCountChanged);
    QSignalSpy canGoToPreviousPageChangedSpy(&pageNavigation, &QPdfPageNavigation::canGoToPreviousPageChanged);
    QSignalSpy canGoToNextPageChangedSpy(&pageNavigation, &QPdfPageNavigation::canGoToNextPageChanged);

    pageNavigation.setDocument(&document);

    QCOMPARE(currentPageChangedSpy.count(), 0); // current page stays '0'
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), 3);
    QCOMPARE(canGoToPreviousPageChangedSpy.count(), 0); // still no previous page available
    QCOMPARE(canGoToNextPageChangedSpy.count(), 1);
    QCOMPARE(canGoToNextPageChangedSpy[0][0].toBool(), true);
}

void tst_QPdfPageNavigation::unloadDocument()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.pagenavigation.pdf")), QPdfDocument::NoError);

    QPdfPageNavigation pageNavigation;
    pageNavigation.setDocument(&document);

    QSignalSpy currentPageChangedSpy(&pageNavigation, &QPdfPageNavigation::currentPageChanged);
    QSignalSpy pageCountChangedSpy(&pageNavigation, &QPdfPageNavigation::pageCountChanged);
    QSignalSpy canGoToPreviousPageChangedSpy(&pageNavigation, &QPdfPageNavigation::canGoToPreviousPageChanged);
    QSignalSpy canGoToNextPageChangedSpy(&pageNavigation, &QPdfPageNavigation::canGoToNextPageChanged);

    document.close();

    QCOMPARE(currentPageChangedSpy.count(), 0); // current page stays '0'
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), 0);
    QCOMPARE(canGoToPreviousPageChangedSpy.count(), 0); // still no previous page available
    QCOMPARE(canGoToNextPageChangedSpy.count(), 1);
    QCOMPARE(canGoToNextPageChangedSpy[0][0].toBool(), false);
}

void tst_QPdfPageNavigation::navigate()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.pagenavigation.pdf")), QPdfDocument::NoError);

    QPdfPageNavigation pageNavigation;
    pageNavigation.setDocument(&document);

    QSignalSpy currentPageChangedSpy(&pageNavigation, &QPdfPageNavigation::currentPageChanged);
    QSignalSpy canGoToPreviousPageChangedSpy(&pageNavigation, &QPdfPageNavigation::canGoToPreviousPageChanged);
    QSignalSpy canGoToNextPageChangedSpy(&pageNavigation, &QPdfPageNavigation::canGoToNextPageChanged);

    QCOMPARE(pageNavigation.currentPage(), 0);

    // try to go to previous page while there is none
    QCOMPARE(pageNavigation.canGoToPreviousPage(), false);
    pageNavigation.goToPreviousPage();
    QCOMPARE(canGoToPreviousPageChangedSpy.count(), 0);
    QCOMPARE(pageNavigation.currentPage(), 0);
    QCOMPARE(pageNavigation.canGoToPreviousPage(), false);

    // try to go to next page
    QCOMPARE(pageNavigation.canGoToNextPage(), true);
    pageNavigation.goToNextPage();
    QCOMPARE(canGoToPreviousPageChangedSpy.count(), 1);
    QCOMPARE(canGoToNextPageChangedSpy.count(), 0);
    QCOMPARE(currentPageChangedSpy.count(), 1);
    QCOMPARE(pageNavigation.currentPage(), 1);
    QCOMPARE(pageNavigation.canGoToPreviousPage(), true);

    currentPageChangedSpy.clear();
    canGoToPreviousPageChangedSpy.clear();
    canGoToNextPageChangedSpy.clear();

    // try to go to last page
    pageNavigation.setCurrentPage(2);
    QCOMPARE(canGoToPreviousPageChangedSpy.count(), 0);
    QCOMPARE(canGoToNextPageChangedSpy.count(), 1);
    QCOMPARE(currentPageChangedSpy.count(), 1);
    QCOMPARE(pageNavigation.currentPage(), 2);
    QCOMPARE(pageNavigation.canGoToNextPage(), false);

    // check that invalid requests are ignored
    pageNavigation.setCurrentPage(-1);
    QCOMPARE(pageNavigation.currentPage(), 2);

    pageNavigation.setCurrentPage(3);
    QCOMPARE(pageNavigation.currentPage(), 2);
}

QTEST_MAIN(tst_QPdfPageNavigation)

#include "tst_qpdfpagenavigation.moc"
