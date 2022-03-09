// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QSignalSpy>
#include <QTest>
#include <QtCore/QLoggingCategory>
#include <QtGui/QClipboard>
#include <QtGui/QPointingDevice>
#include <QtQuick/QQuickView>
#include <QtPdfQuick/private/qquickpdflinkmodel_p.h>
#include <QtPdfQuick/private/qquickpdfsearchmodel_p.h>
#include <QtPdfQuick/private/qquickpdfpageimage_p.h>
#include "../shared/util.h"

Q_LOGGING_CATEGORY(lcTests, "qt.pdf.tests")

class tst_MultiPageView : public QQuickDataTest
{
    Q_OBJECT

private Q_SLOTS:
    void internalLink_data();
    void internalLink();
    void password();
    void selectionAndClipboard();
    void search();

private:
    QScopedPointer<QPointingDevice> touchscreen = QScopedPointer<QPointingDevice>(QTest::createTouchDevice());
};

void tst_MultiPageView::internalLink_data()
{
    QTest::addColumn<int>("linkIndex");
    QTest::addColumn<int>("expectedPage");
    QTest::addColumn<qreal>("expectedZoom");
    QTest::addColumn<QPoint>("expectedScroll");

    QTest::newRow("first link") << 0 << 1 << qreal(1) << QPoint(134, 1276);
    // TODO fails because it zooms out, and the view leaves gaps between pages currently
//    QTest::newRow("second link") << 1 << 2 << qreal(0.5) << QPoint(0, 717);
}

void tst_MultiPageView::internalLink()
{
    QFETCH(int, linkIndex);
    QFETCH(int, expectedPage);
    QFETCH(qreal, expectedZoom);
    QFETCH(QPoint, expectedScroll);

    QQuickView window;
    QVERIFY(showView(window, testFileUrl("multiPageView.qml")));
    QQuickItem *pdfView = window.rootObject();
    QVERIFY(pdfView);
    pdfView->setProperty("source", "bookmarksAndLinks.pdf");
    QTRY_COMPARE(pdfView->property("currentPageRenderingStatus").toInt(), QQuickPdfPageImage::Ready);

    QQuickItem *table = static_cast<QQuickItem *>(findFirstChild(pdfView, "QQuickTableView"));
    QVERIFY(table);
    QQuickItem *firstPage = tableViewItemAtCell(table, 0, 0);
    QVERIFY(firstPage);
    QQuickPdfLinkModel *linkModel = firstPage->findChild<QQuickPdfLinkModel*>();
    QVERIFY(linkModel);
    QQuickItem *repeater = qobject_cast<QQuickItem *>(linkModel->parent());
    QVERIFY(repeater);
    QVERIFY(repeater->property("count").toInt() > linkIndex);

    QCOMPARE(pdfView->property("backEnabled").toBool(), false);
    QCOMPARE(pdfView->property("forwardEnabled").toBool(), false);

    // get the PdfLinkDelegate instance, which has a TapHandler declared inside
    QQuickItem *linkDelegate = repeaterItemAt(repeater, linkIndex);
    QVERIFY(linkDelegate);
    const auto modelIdx = linkModel->index(linkIndex);
    const int linkPage = linkModel->data(modelIdx, int(QPdfLinkModel::Role::Page)).toInt();
    QVERIFY(linkPage >= 0);
    const QPointF linkLocation = linkModel->data(modelIdx, int(QPdfLinkModel::Role::Location)).toPointF();
    const qreal linkZoom = linkModel->data(modelIdx, int(QPdfLinkModel::Role::Zoom)).toReal();

    // click on it, and check whether it went to the right place
    const auto point = linkDelegate->position().toPoint() + QPoint(15, 15);
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point);
    QTRY_COMPARE(tableViewContentPos(table).y(), expectedScroll.y());
    const auto linkScrollPos = tableViewContentPos(table);
    qCDebug(lcTests, "clicked link @ %d, %d and expected scrolling to %d, %d; actually scrolled to %d, %d",
            point.x(), point.y(), expectedScroll.x(), expectedScroll.y(), linkScrollPos.x(), linkScrollPos.y());
    QVERIFY(qAbs(linkScrollPos.x() - expectedScroll.x()) < 15);
    QTRY_COMPARE(pdfView->property("currentPageRenderingStatus").toInt(), QQuickPdfPageImage::Ready);
    QCOMPARE(pdfView->property("currentPage").toInt(), linkPage);
    QCOMPARE(linkPage, expectedPage);
    QCOMPARE(pdfView->property("renderScale").toReal(), linkZoom);
    QCOMPARE(linkZoom, expectedZoom);
    qCDebug(lcTests, "link %d goes to page %d location {%lf,%lf} zoom %lf scroll to {%lf,%lf}",
            linkIndex, linkPage, linkLocation.x(), linkLocation.y(), linkZoom,
            table->property("contentX").toReal(), table->property("contentY").toReal());

    // check that we can go back to where we came from
    QCOMPARE(pdfView->property("backEnabled").toBool(), true);
    QCOMPARE(pdfView->property("forwardEnabled").toBool(), false);
    QVERIFY(QMetaObject::invokeMethod(pdfView, "back"));
    QTRY_COMPARE(tableViewContentPos(table), QPoint(0, 0));
    QCOMPARE(pdfView->property("currentPage").toInt(), 0);
    QCOMPARE(pdfView->property("renderScale").toReal(), qreal(1));

    // and then forward again
    QCOMPARE(pdfView->property("backEnabled").toBool(), false);
    QCOMPARE(pdfView->property("forwardEnabled").toBool(), true);
    QVERIFY(QMetaObject::invokeMethod(pdfView, "forward"));
    QTRY_COMPARE(tableViewContentPos(table), linkScrollPos);
    QCOMPARE(pdfView->property("currentPage").toInt(), linkPage);
    QCOMPARE(pdfView->property("renderScale").toReal(), linkZoom);
}

void tst_MultiPageView::password()
{
    QQuickView window;
    QVERIFY(showView(window, testFileUrl("multiPageView.qml")));
    QQuickItem *pdfView = window.rootObject();
    QVERIFY(pdfView);
    QQuickPdfDocument *doc = pdfView->property("document").value<QQuickPdfDocument*>();
    QVERIFY(doc);
    QPdfDocument *cppDoc = static_cast<QPdfDocument *>(qmlExtendedObject(doc));
    QVERIFY(cppDoc);
    QSignalSpy passwordRequiredSpy(doc, SIGNAL(passwordRequired()));
    // actually QPdfDocument::passwordRequired, but QML_EXTENDED gives us this signal virtually in QQuickPdfDocument
    QVERIFY(passwordRequiredSpy.isValid());
    QSignalSpy passwordChangedSpy(doc, SIGNAL(passwordChanged()));
    // actually QPdfDocument::passwordChanged, but QML_EXTENDED gives us this signal virtually in QQuickPdfDocument
    QVERIFY(passwordChangedSpy.isValid());
    QSignalSpy statusChangedSpy(doc, SIGNAL(statusChanged(QPdfDocument::Status)));
    // actually QPdfDocument::statusChanged, but QML_EXTENDED gives us this signal virtually in QQuickPdfDocument
    QVERIFY(statusChangedSpy.isValid());
    QSignalSpy pageCountChangedSpy(doc, SIGNAL(pageCountChanged(int)));
    // QPdfDocument::pageCountChanged(int), but QML_EXTENDED gives us this signal virtually in QQuickPdfDocument
    QVERIFY(pageCountChangedSpy.isValid());
    QSignalSpy extPageCountChangedSpy(cppDoc, &QPdfDocument::pageCountChanged);
    // actual QPdfDocument::pageCountChanged(int), for comparison with the illusory QQuickPdfDocument::pageCountChanged
    QVERIFY(extPageCountChangedSpy.isValid());

    QVERIFY(pdfView->setProperty("source", u"pdf-sample.protected.pdf"_qs));

    QTRY_COMPARE(passwordRequiredSpy.count(), 1);
    qCDebug(lcTests) << "error while awaiting password" << doc->error()
                     << "passwordRequired count" << passwordRequiredSpy.count()
                     << "statusChanged count" << statusChangedSpy.count();
    QCOMPARE(doc->property("status").toInt(), int(QPdfDocument::Status::Error));
    QCOMPARE(pageCountChangedSpy.count(), 0);
    QCOMPARE(extPageCountChangedSpy.count(), 0);
    QCOMPARE(statusChangedSpy.count(), 2); // Loading and then Error
    statusChangedSpy.clear();
    QVERIFY(doc->setProperty("password", u"Qt"_qs));
    QCOMPARE(passwordChangedSpy.count(), 1);
    QTRY_COMPARE(doc->property("status").toInt(), int(QPdfDocument::Status::Ready));
    qCDebug(lcTests) << "after setPassword" << doc->error()
                     << "passwordChanged count" << passwordChangedSpy.count()
                     << "statusChanged count" << statusChangedSpy.count()
                     << "pageCountChanged count" << pageCountChangedSpy.count();
    QCOMPARE(statusChangedSpy.count(), 2); // Loading and then Ready
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(extPageCountChangedSpy.count(), pageCountChangedSpy.count());
}

void tst_MultiPageView::selectionAndClipboard()
{
    QQuickView window;
    QVERIFY(showView(window, testFileUrl("multiPageView.qml")));
    QQuickItem *pdfView = window.rootObject();
    QVERIFY(pdfView);
    QQuickPdfDocument *doc = pdfView->property("document").value<QQuickPdfDocument*>();
    QVERIFY(doc);
    QVERIFY(doc->setProperty("password", u"Qt"_qs));
    QVERIFY(pdfView->setProperty("source", u"pdf-sample.protected.pdf"_qs));
    QTRY_COMPARE(pdfView->property("currentPageRenderingStatus").toInt(), QQuickPdfPageImage::Ready);

    QVERIFY(QMetaObject::invokeMethod(pdfView, "selectAll"));
    QString sel = pdfView->property("selectedText").toString();
    QCOMPARE(sel.length(), 1073);

#if QT_CONFIG(clipboard)
    QClipboard *clip = qApp->clipboard();
    if (clip->supportsSelection())
        QCOMPARE(clip->text(QClipboard::Selection), sel);
    QVERIFY(QMetaObject::invokeMethod(pdfView, "copySelectionToClipboard"));
    QCOMPARE(clip->text(QClipboard::Clipboard), sel);
#endif // clipboard
}

void tst_MultiPageView::search()
{
    QQuickView window;
    QVERIFY(showView(window, testFileUrl("multiPageView.qml")));
    window.setResizeMode(QQuickView::SizeRootObjectToView);
    window.resize(200, 200);
    QQuickItem *pdfView = window.rootObject();
    QVERIFY(pdfView);
    QTRY_COMPARE(pdfView->width(), 200);
    QQuickPdfDocument *doc = pdfView->property("document").value<QQuickPdfDocument*>();
    QVERIFY(doc);
    QVERIFY(doc->setProperty("password", u"Qt"_qs));
    QVERIFY(pdfView->setProperty("source", u"pdf-sample.protected.pdf"_qs));
    QTRY_COMPARE(pdfView->property("currentPageRenderingStatus").toInt(), QQuickPdfPageImage::Ready);
    QPdfSearchModel *searchModel = pdfView->property("searchModel").value<QPdfSearchModel*>();
    QVERIFY(searchModel);
    QQuickItem *table = static_cast<QQuickItem *>(findFirstChild(pdfView, "QQuickTableView"));
    QVERIFY(table);
    QQuickItem *firstPage = tableViewItemAtCell(table, 0, 0);
    QVERIFY(firstPage);
    QObject *multiline = findFirstChild(firstPage, "QQuickPathMultiline");
    QVERIFY(multiline);

    pdfView->setProperty("searchString", u"PDF"_qs);
    QTRY_COMPARE(searchModel->rowCount(QModelIndex()), 7); // occurrences of the word "PDF" in this file
    const int count = searchModel->rowCount(QModelIndex());
    QList<QList<QPointF>> resultOutlines = multiline->property("paths").value<QList<QList<QPointF>>>();
    QCOMPARE(resultOutlines.count(), 7);
    QPoint contentPos = tableViewContentPos(table);
    int movements = 0;
    for (int i = 0; i < count; ++i) {
        // only one page, so IndexOnPage data is the same as overall index
        QCOMPARE(i, searchModel->data(searchModel->index(i), int(QPdfSearchModel::Role::IndexOnPage)).toInt());
        QCOMPARE(resultOutlines.at(i).count(), 5); // 5-point polygon is a rectangle (including drawing back to the start, to close it)
        QCOMPARE(resultOutlines.at(i).first(), searchModel->data(searchModel->index(i), int(QPdfSearchModel::Role::Location)).toPointF());

        QVERIFY(QMetaObject::invokeMethod(pdfView, "searchForward"));
        QTest::qWait(500); // animation time; but it doesn't always need to move
        // TODO maybe: if movement starts, wait for it to stop somehow?
        qCDebug(lcTests) << i << resultOutlines.at(i) << "scrolled to" << tableViewContentPos(table);
        if (tableViewContentPos(table) != contentPos)
            ++movements;
        contentPos = tableViewContentPos(table);
    }
    qCDebug(lcTests) << "total movements" << movements;
    QVERIFY(movements > 4);
}

QTEST_MAIN(tst_MultiPageView)
#include "tst_multipageview.moc"
