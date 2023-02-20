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

using namespace Qt::StringLiterals;

Q_LOGGING_CATEGORY(lcTests, "qt.pdf.tests")

class tst_MultiPageView : public QQuickDataTest
{
    Q_OBJECT

private Q_SLOTS:
    void internalLink_data();
    void internalLink();
    void navigation_data();
    void navigation();
    void password();
    void selectionAndClipboard();
    void search();

public:
    enum NavigationAction {
        Back,
        Forward,
        GotoPage,
        GotoLocation,
        ClickLink
    };
    Q_ENUM(NavigationAction)

    struct NavigationCommand {
        NavigationAction action;
        int index;
        QPointF location;
        qreal zoom;
        QPointF expectedContentPos;
        int expectedCurrentPage;
    };

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
    pdfView->setProperty("source", testFileUrl("bookmarksAndLinks.pdf"));
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

void tst_MultiPageView::navigation_data()
{
    QTest::addColumn<QList<NavigationCommand>>("actions");
    const int totalPageSpacing = 832; // 826 points + 6 px (rowSpacing)

    QList<NavigationCommand> actions;
    actions << NavigationCommand {NavigationAction::GotoPage, 2, {}, 0, {0, 1664}, 2}
            << NavigationCommand {NavigationAction::GotoPage, 3, {}, 0, {0, 2496}, 3}
            << NavigationCommand {NavigationAction::Back, 0, {}, 0, {0, 1664}, 2}
            << NavigationCommand {NavigationAction::Back, 0, {}, 0, {0, 0}, 0};
    QTest::newRow("goto and back") << actions;

    actions.clear();
    actions // first link is "More..." going to page 0, location 8, 740
            << NavigationCommand {NavigationAction::ClickLink, 0, {465, 65}, 0, {0, 740}, 0}
            << NavigationCommand {NavigationAction::Back, 0, {}, 0, {0, 0}, 0}
            // link "setPdfVersion()" going to page 3, location 8, 295
            << NavigationCommand {NavigationAction::ClickLink, 0, {255, 455}, 0, {0, totalPageSpacing * 3 + 295}, 3}
            << NavigationCommand {NavigationAction::Back, 0, {}, 0, {0, 0}, 0};
    QTest::newRow("click links and go back, twice") << actions;

    actions.clear();
    actions // first link is "More..." going to page 0, location 8, 740
            << NavigationCommand {NavigationAction::ClickLink, 0, {465, 65}, 0, {0, 740}, 0}
               // link "newPage()" going to page 1, location 8, 290
            << NavigationCommand {NavigationAction::ClickLink, 0, {480, 40}, 0, {0, totalPageSpacing + 290}, 1} // fails, goes back to page 0
            << NavigationCommand {NavigationAction::Back, 0, {}, 0, {8, 740}, 0}
            << NavigationCommand {NavigationAction::Back, 0, {}, 0, {0, 0}, 0};
    QTest::newRow("click two links in series and then go back") << actions;
}

void tst_MultiPageView::navigation()
{
    QFETCH(QList<NavigationCommand>, actions);

    QQuickView window;
    window.setColor(Qt::gray);
    window.setSource(testFileUrl("multiPageViewWithFeedback.qml"));
    QTRY_COMPARE(window.status(), QQuickView::Ready);
    QQuickItem *pdfView = window.rootObject();
    QVERIFY(pdfView);
    QObject *doc = pdfView->property("document").value<QObject *>();
    QVERIFY(doc);
    doc->setProperty("source", testFileUrl("qpdfwriter.pdf"));
    QQuickItem *table = static_cast<QQuickItem *>(findFirstChild(pdfView, "QQuickTableView"));
    QVERIFY(table);
    // Expect that contentY == destination y after a jump, for ease of comparison.
    // 0.01 is close enough to 0 that we can compare int positions accurately,
    // but nonzero so that QRectF::isValid() is true in tableView.positionViewAtCell()
    table->setProperty("jumpLocationMargin", QPointF(0.01, 0.01));

    window.show();
    window.requestActivate();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QTRY_COMPARE(table->property("contentHeight").toInt(), 3322);
    QCOMPARE(table->property("contentY").toInt(), 0);

    for (const NavigationCommand &nav : actions) {
        switch (nav.action) {
        case NavigationAction::Back:
            QVERIFY(QMetaObject::invokeMethod(pdfView, "back"));
            QCOMPARE(pdfView->property("forwardEnabled").toBool(), true);
            break;
        case NavigationAction::Forward:
            QVERIFY(QMetaObject::invokeMethod(pdfView, "forward"));
            QCOMPARE(pdfView->property("backEnabled").toBool(), true);
            break;
        case NavigationAction::GotoPage:
            QVERIFY(QMetaObject::invokeMethod(pdfView, "goToPage",
                                              Q_ARG(QVariant, QVariant(nav.index))));
            QCOMPARE(pdfView->property("backEnabled").toBool(), true);
            break;
        case NavigationAction::GotoLocation:
            QVERIFY(QMetaObject::invokeMethod(pdfView, "goToLocation",
                                              Q_ARG(QVariant, QVariant(nav.index)),
                                              Q_ARG(QVariant, QVariant(nav.location)),
                                              Q_ARG(QVariant, QVariant(nav.zoom)) ));
            break;
        case NavigationAction::ClickLink:
            // Link delegates don't exist until page rendering is done
            QTRY_VERIFY(pdfView->property("currentPageRenderingStatus").toInt() == 1); // QQuickImage::Status::Ready
            QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, nav.location.toPoint());
            // Wait for the destination page to be rendered
            QTRY_VERIFY(pdfView->property("currentPageRenderingStatus").toInt() == 1); // QQuickImage::Status::Ready
            break;
        }
        qCDebug(lcTests) << "action" << nav.action << "index" << nav.index
                         << "contentX,Y" << table->property("contentX").toInt() << table->property("contentY").toInt()
                         << "expected" << nav.expectedContentPos;
        QTRY_COMPARE(table->property("contentY").toInt(), nav.expectedContentPos.y());
        // some minor side-to-side scrolling happens, in practice
        QVERIFY(qAbs(table->property("contentX").toInt() - nav.expectedContentPos.x()) < 10);
        QCOMPARE(pdfView->property("currentPage").toInt(), nav.expectedCurrentPage);
    }

    QCOMPARE(pdfView->property("backEnabled").toBool(), false);
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

    QVERIFY(pdfView->setProperty("source", testFileUrl(u"pdf-sample.protected.pdf"_s)));

    QTRY_COMPARE(passwordRequiredSpy.size(), 1);
    qCDebug(lcTests) << "error while awaiting password" << doc->error()
                     << "passwordRequired count" << passwordRequiredSpy.size()
                     << "statusChanged count" << statusChangedSpy.size();
    QCOMPARE(doc->property("status").toInt(), int(QPdfDocument::Status::Error));
    QCOMPARE(pageCountChangedSpy.size(), 0);
    QCOMPARE(extPageCountChangedSpy.size(), 0);
    QCOMPARE(statusChangedSpy.size(), 2); // Loading and then Error
    statusChangedSpy.clear();
    QVERIFY(doc->setProperty("password", u"Qt"_s));
    QCOMPARE(passwordChangedSpy.size(), 1);
    QTRY_COMPARE(doc->property("status").toInt(), int(QPdfDocument::Status::Ready));
    qCDebug(lcTests) << "after setPassword" << doc->error()
                     << "passwordChanged count" << passwordChangedSpy.size()
                     << "statusChanged count" << statusChangedSpy.size()
                     << "pageCountChanged count" << pageCountChangedSpy.size();
    QCOMPARE(statusChangedSpy.size(), 2); // Loading and then Ready
    QCOMPARE(pageCountChangedSpy.size(), 1);
    QCOMPARE(extPageCountChangedSpy.size(), pageCountChangedSpy.size());
}

void tst_MultiPageView::selectionAndClipboard()
{
    QQuickView window;
    QVERIFY(showView(window, testFileUrl("multiPageView.qml")));
    QQuickItem *pdfView = window.rootObject();
    QVERIFY(pdfView);
    QQuickPdfDocument *doc = pdfView->property("document").value<QQuickPdfDocument*>();
    QVERIFY(doc);
    QVERIFY(doc->setProperty("password", u"Qt"_s));
    QVERIFY(pdfView->setProperty("source", testFileUrl((u"pdf-sample.protected.pdf"_s))));
    QTRY_COMPARE(pdfView->property("currentPageRenderingStatus").toInt(), QQuickPdfPageImage::Ready);

    QVERIFY(QMetaObject::invokeMethod(pdfView, "selectAll"));
    QString sel = pdfView->property("selectedText").toString();
    QCOMPARE(sel.size(), 1073);

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
    QVERIFY(doc->setProperty("password", u"Qt"_s));
    QVERIFY(pdfView->setProperty("source", testFileUrl(u"pdf-sample.protected.pdf"_s)));
    QTRY_COMPARE(pdfView->property("currentPageRenderingStatus").toInt(), QQuickPdfPageImage::Ready);
    QPdfSearchModel *searchModel = pdfView->property("searchModel").value<QPdfSearchModel*>();
    QVERIFY(searchModel);
    QQuickItem *table = static_cast<QQuickItem *>(findFirstChild(pdfView, "QQuickTableView"));
    QVERIFY(table);
    QQuickItem *firstPage = tableViewItemAtCell(table, 0, 0);
    QVERIFY(firstPage);
    QObject *multiline = findFirstChild(firstPage, "QQuickPathMultiline");
    QVERIFY(multiline);

    pdfView->setProperty("searchString", u"PDF"_s);
    QTRY_COMPARE(searchModel->rowCount(QModelIndex()), 7); // occurrences of the word "PDF" in this file
    const int count = searchModel->rowCount(QModelIndex());
    QList<QList<QPointF>> resultOutlines = multiline->property("paths").value<QList<QList<QPointF>>>();
    QCOMPARE(resultOutlines.size(), 7);
    QPoint contentPos = tableViewContentPos(table);
    int movements = 0;
    for (int i = 0; i < count; ++i) {
        // only one page, so IndexOnPage data is the same as overall index
        QCOMPARE(i, searchModel->data(searchModel->index(i), int(QPdfSearchModel::Role::IndexOnPage)).toInt());
        QCOMPARE(resultOutlines.at(i).size(), 5); // 5-point polygon is a rectangle (including drawing back to the start, to close it)
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
