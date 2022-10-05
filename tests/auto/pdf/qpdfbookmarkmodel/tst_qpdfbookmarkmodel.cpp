// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#include <QtTest/QtTest>

#include <QPdfDocument>
#include <QPdfBookmarkModel>

class tst_QPdfBookmarkModel: public QObject
{
    Q_OBJECT

public:
    tst_QPdfBookmarkModel()
    {
        qRegisterMetaType<QPdfDocument::Status>();
    }

private slots:
    void emptyModel();
    void setEmptyDocument();
    void setEmptyDocumentAndLoad();
    void setLoadedDocument();
    void unloadDocument();
    void testTreeStructure();
    void testPageNumberRole();
    void testLocationAndZoomRoles();
};

void tst_QPdfBookmarkModel::emptyModel()
{
    QPdfBookmarkModel model;

    QVERIFY(!model.document());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 1);
    QCOMPARE(model.index(0, 0).isValid(), false);
}

void tst_QPdfBookmarkModel::setEmptyDocument()
{
    QPdfDocument document;
    QPdfBookmarkModel model;

    model.setDocument(&document);

    QCOMPARE(model.document(), &document);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 1);
    QCOMPARE(model.index(0, 0).isValid(), false);
}

void tst_QPdfBookmarkModel::setEmptyDocumentAndLoad()
{
    QPdfDocument document;
    QPdfBookmarkModel model;

    model.setDocument(&document);

    QSignalSpy modelAboutToBeResetSpy(&model, SIGNAL(modelAboutToBeReset()));
    QSignalSpy modelResetSpy(&model, SIGNAL(modelReset()));

    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks.pdf")), QPdfDocument::Error::None);

    QCOMPARE(modelAboutToBeResetSpy.size(), 1);
    QCOMPARE(modelResetSpy.size(), 1);

    QCOMPARE(model.rowCount(), 3);
}

void tst_QPdfBookmarkModel::setLoadedDocument()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks.pdf")), QPdfDocument::Error::None);

    QPdfBookmarkModel model;

    QSignalSpy modelAboutToBeResetSpy(&model, SIGNAL(modelAboutToBeReset()));
    QSignalSpy modelResetSpy(&model, SIGNAL(modelReset()));

    model.setDocument(&document);

    QCOMPARE(modelAboutToBeResetSpy.size(), 1);
    QCOMPARE(modelResetSpy.size(), 1);

    QCOMPARE(model.rowCount(), 3);
}

void tst_QPdfBookmarkModel::unloadDocument()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks.pdf")), QPdfDocument::Error::None);

    QPdfBookmarkModel model;
    model.setDocument(&document);

    QCOMPARE(model.rowCount(), 3);

    QSignalSpy modelAboutToBeResetSpy(&model, SIGNAL(modelAboutToBeReset()));
    QSignalSpy modelResetSpy(&model, SIGNAL(modelReset()));

    document.close();

    QCOMPARE(modelAboutToBeResetSpy.size(), 1);
    QCOMPARE(modelResetSpy.size(), 1);

    QCOMPARE(model.rowCount(), 0);
}

void tst_QPdfBookmarkModel::testTreeStructure()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks.pdf")), QPdfDocument::Error::None);

    QPdfBookmarkModel model;
    model.setDocument(&document);

    QCOMPARE(model.rowCount(), 3);

    const QModelIndex index1 = model.index(0, 0);
    QCOMPARE(index1.data(int(QPdfBookmarkModel::Role::Title)).toString(), QLatin1String("Section 1"));
    QCOMPARE(index1.data(int(QPdfBookmarkModel::Role::Level)).toInt(), 0);
    QCOMPARE(model.rowCount(index1), 2);

    const QModelIndex index1_1 = model.index(0, 0, index1);
    QCOMPARE(index1_1.data(int(QPdfBookmarkModel::Role::Title)).toString(), QLatin1String("Section 1.1"));
    QCOMPARE(index1_1.data(int(QPdfBookmarkModel::Role::Level)).toInt(), 1);
    QCOMPARE(model.rowCount(index1_1), 0);

    const QModelIndex index1_2 = model.index(1, 0, index1);
    QCOMPARE(index1_2.data(int(QPdfBookmarkModel::Role::Title)).toString(), QLatin1String("Section 1.2"));
    QCOMPARE(index1_2.data(int(QPdfBookmarkModel::Role::Level)).toInt(), 1);
    QCOMPARE(model.rowCount(index1_2), 0);

    const QModelIndex index2 = model.index(1, 0);
    QCOMPARE(index2.data(int(QPdfBookmarkModel::Role::Title)).toString(), QLatin1String("Section 2"));
    QCOMPARE(index2.data(int(QPdfBookmarkModel::Role::Level)).toInt(), 0);
    QCOMPARE(model.rowCount(index2), 2);

    const QModelIndex index2_1 = model.index(0, 0, index2);
    QCOMPARE(index2_1.data(int(QPdfBookmarkModel::Role::Title)).toString(), QLatin1String("Section 2.1"));
    QCOMPARE(index2_1.data(int(QPdfBookmarkModel::Role::Level)).toInt(), 1);
    QCOMPARE(model.rowCount(index2_1), 1);

    const QModelIndex index2_1_1 = model.index(0, 0, index2_1);
    QCOMPARE(index2_1_1.data(int(QPdfBookmarkModel::Role::Title)).toString(), QLatin1String("Section 2.1.1"));
    QCOMPARE(index2_1_1.data(int(QPdfBookmarkModel::Role::Level)).toInt(), 2);
    QCOMPARE(model.rowCount(index2_1_1), 0);

    const QModelIndex index2_2 = model.index(1, 0, index2);
    QCOMPARE(index2_2.data(int(QPdfBookmarkModel::Role::Title)).toString(), QLatin1String("Section 2.2"));
    QCOMPARE(index2_2.data(int(QPdfBookmarkModel::Role::Level)).toInt(), 1);
    QCOMPARE(model.rowCount(index2_2), 0);

    const QModelIndex index3 = model.index(2, 0);
    QCOMPARE(index3.data(int(QPdfBookmarkModel::Role::Title)).toString(), QLatin1String("Section 3"));
    QCOMPARE(index3.data(int(QPdfBookmarkModel::Role::Level)).toInt(), 0);
    QCOMPARE(model.rowCount(index3), 0);

    const QModelIndex index4 = model.index(3, 0);
    QCOMPARE(index4, QModelIndex());
}

void tst_QPdfBookmarkModel::testPageNumberRole()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks_pages.pdf")), QPdfDocument::Error::None);

    QPdfBookmarkModel model;
    model.setDocument(&document);

    QCOMPARE(model.rowCount(), 3);

    const QModelIndex index1 = model.index(0, 0);
    QCOMPARE(index1.data(int(QPdfBookmarkModel::Role::Page)).toInt(), 0);

    const QModelIndex index2 = model.index(1, 0);
    QCOMPARE(index2.data(int(QPdfBookmarkModel::Role::Page)).toInt(), 1);

    const QModelIndex index2_1 = model.index(0, 0, index2);
    QCOMPARE(index2_1.data(int(QPdfBookmarkModel::Role::Page)).toInt(), 1);

    const QModelIndex index3 = model.index(2, 0);
    QCOMPARE(index3.data(int(QPdfBookmarkModel::Role::Page)).toInt(), 2);
}

void tst_QPdfBookmarkModel::testLocationAndZoomRoles()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks_pages.pdf")), QPdfDocument::Error::None);

    QPdfBookmarkModel model;
    model.setDocument(&document);

    QCOMPARE(model.rowCount(), 3);

    const QModelIndex index1 = model.index(0, 0);
    QCOMPARE(index1.data(int(QPdfBookmarkModel::Role::Location)).toPoint(), QPoint(57, 69));
    QCOMPARE(index1.data(int(QPdfBookmarkModel::Role::Zoom)).toInt(), 0);

    const QModelIndex index2 = model.index(1, 0);
    QCOMPARE(index2.data(int(QPdfBookmarkModel::Role::Location)).toPoint(), QPoint(57, 57));
    QCOMPARE(index2.data(int(QPdfBookmarkModel::Role::Zoom)).toInt(), 0);

    const QModelIndex index2_1 = model.index(0, 0, index2);
    QCOMPARE(index2_1.data(int(QPdfBookmarkModel::Role::Location)).toPoint(), QPoint(57, 526));
    QCOMPARE(index2_1.data(int(QPdfBookmarkModel::Role::Zoom)).toInt(), 0);

    const QModelIndex index3 = model.index(2, 0);
    QCOMPARE(index3.data(int(QPdfBookmarkModel::Role::Location)).toPoint(), QPoint(57, 402));
    QCOMPARE(index3.data(int(QPdfBookmarkModel::Role::Zoom)).toInt(), 0);
}

QTEST_MAIN(tst_QPdfBookmarkModel)

#include "tst_qpdfbookmarkmodel.moc"
