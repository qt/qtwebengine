/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
    void testListStructure();
    void testPageNumberRole();
};

void tst_QPdfBookmarkModel::emptyModel()
{
    QPdfBookmarkModel model;

    QVERIFY(!model.document());
    QCOMPARE(model.structureMode(), QPdfBookmarkModel::TreeMode);
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
    QCOMPARE(model.structureMode(), QPdfBookmarkModel::TreeMode);
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

    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks.pdf")), QPdfDocument::NoError);

    QCOMPARE(modelAboutToBeResetSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.rowCount(), 3);
}

void tst_QPdfBookmarkModel::setLoadedDocument()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks.pdf")), QPdfDocument::NoError);

    QPdfBookmarkModel model;

    QSignalSpy modelAboutToBeResetSpy(&model, SIGNAL(modelAboutToBeReset()));
    QSignalSpy modelResetSpy(&model, SIGNAL(modelReset()));

    model.setDocument(&document);

    QCOMPARE(modelAboutToBeResetSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.rowCount(), 3);
}

void tst_QPdfBookmarkModel::unloadDocument()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks.pdf")), QPdfDocument::NoError);

    QPdfBookmarkModel model;
    model.setDocument(&document);

    QCOMPARE(model.rowCount(), 3);

    QSignalSpy modelAboutToBeResetSpy(&model, SIGNAL(modelAboutToBeReset()));
    QSignalSpy modelResetSpy(&model, SIGNAL(modelReset()));

    document.close();

    QCOMPARE(modelAboutToBeResetSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.rowCount(), 0);
}

void tst_QPdfBookmarkModel::testTreeStructure()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks.pdf")), QPdfDocument::NoError);

    QPdfBookmarkModel model;
    model.setDocument(&document);

    QCOMPARE(model.rowCount(), 3);

    const QModelIndex index1 = model.index(0, 0);
    QCOMPARE(index1.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 1"));
    QCOMPARE(index1.data(QPdfBookmarkModel::LevelRole).toInt(), 0);
    QCOMPARE(model.rowCount(index1), 2);

    const QModelIndex index1_1 = model.index(0, 0, index1);
    QCOMPARE(index1_1.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 1.1"));
    QCOMPARE(index1_1.data(QPdfBookmarkModel::LevelRole).toInt(), 1);
    QCOMPARE(model.rowCount(index1_1), 0);

    const QModelIndex index1_2 = model.index(1, 0, index1);
    QCOMPARE(index1_2.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 1.2"));
    QCOMPARE(index1_2.data(QPdfBookmarkModel::LevelRole).toInt(), 1);
    QCOMPARE(model.rowCount(index1_2), 0);

    const QModelIndex index2 = model.index(1, 0);
    QCOMPARE(index2.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 2"));
    QCOMPARE(index2.data(QPdfBookmarkModel::LevelRole).toInt(), 0);
    QCOMPARE(model.rowCount(index2), 2);

    const QModelIndex index2_1 = model.index(0, 0, index2);
    QCOMPARE(index2_1.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 2.1"));
    QCOMPARE(index2_1.data(QPdfBookmarkModel::LevelRole).toInt(), 1);
    QCOMPARE(model.rowCount(index2_1), 1);

    const QModelIndex index2_1_1 = model.index(0, 0, index2_1);
    QCOMPARE(index2_1_1.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 2.1.1"));
    QCOMPARE(index2_1_1.data(QPdfBookmarkModel::LevelRole).toInt(), 2);
    QCOMPARE(model.rowCount(index2_1_1), 0);

    const QModelIndex index2_2 = model.index(1, 0, index2);
    QCOMPARE(index2_2.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 2.2"));
    QCOMPARE(index2_2.data(QPdfBookmarkModel::LevelRole).toInt(), 1);
    QCOMPARE(model.rowCount(index2_2), 0);

    const QModelIndex index3 = model.index(2, 0);
    QCOMPARE(index3.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 3"));
    QCOMPARE(index3.data(QPdfBookmarkModel::LevelRole).toInt(), 0);
    QCOMPARE(model.rowCount(index3), 0);

    const QModelIndex index4 = model.index(3, 0);
    QCOMPARE(index4, QModelIndex());
}

void tst_QPdfBookmarkModel::testListStructure()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks.pdf")), QPdfDocument::NoError);

    QPdfBookmarkModel model;
    model.setDocument(&document);

    QSignalSpy modelAboutToBeResetSpy(&model, SIGNAL(modelAboutToBeReset()));
    QSignalSpy modelResetSpy(&model, SIGNAL(modelReset()));

    model.setStructureMode(QPdfBookmarkModel::ListMode);

    QCOMPARE(modelAboutToBeResetSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.rowCount(), 8);

    const QModelIndex index1 = model.index(0, 0);
    QCOMPARE(index1.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 1"));
    QCOMPARE(index1.data(QPdfBookmarkModel::LevelRole).toInt(), 0);
    QCOMPARE(model.rowCount(index1), 0);

    const QModelIndex index1_1 = model.index(1, 0);
    QCOMPARE(index1_1.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 1.1"));
    QCOMPARE(index1_1.data(QPdfBookmarkModel::LevelRole).toInt(), 1);
    QCOMPARE(model.rowCount(index1_1), 0);

    const QModelIndex index1_2 = model.index(2, 0);
    QCOMPARE(index1_2.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 1.2"));
    QCOMPARE(index1_2.data(QPdfBookmarkModel::LevelRole).toInt(), 1);
    QCOMPARE(model.rowCount(index1_2), 0);

    const QModelIndex index2 = model.index(3, 0);
    QCOMPARE(index2.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 2"));
    QCOMPARE(index2.data(QPdfBookmarkModel::LevelRole).toInt(), 0);
    QCOMPARE(model.rowCount(index2), 0);

    const QModelIndex index2_1 = model.index(4, 0);
    QCOMPARE(index2_1.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 2.1"));
    QCOMPARE(index2_1.data(QPdfBookmarkModel::LevelRole).toInt(), 1);
    QCOMPARE(model.rowCount(index2_1), 0);

    const QModelIndex index2_1_1 = model.index(5, 0);
    QCOMPARE(index2_1_1.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 2.1.1"));
    QCOMPARE(index2_1_1.data(QPdfBookmarkModel::LevelRole).toInt(), 2);
    QCOMPARE(model.rowCount(index2_1_1), 0);

    const QModelIndex index2_2 = model.index(6, 0);
    QCOMPARE(index2_2.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 2.2"));
    QCOMPARE(index2_2.data(QPdfBookmarkModel::LevelRole).toInt(), 1);
    QCOMPARE(model.rowCount(index2_2), 0);

    const QModelIndex index3 = model.index(7, 0);
    QCOMPARE(index3.data(QPdfBookmarkModel::TitleRole).toString(), QLatin1String("Section 3"));
    QCOMPARE(index3.data(QPdfBookmarkModel::LevelRole).toInt(), 0);
    QCOMPARE(model.rowCount(index3), 0);

    const QModelIndex index4 = model.index(8, 0);
    QCOMPARE(index4, QModelIndex());
}

void tst_QPdfBookmarkModel::testPageNumberRole()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("pdf-sample.bookmarks_pages.pdf")), QPdfDocument::NoError);

    QPdfBookmarkModel model;
    model.setDocument(&document);

    QCOMPARE(model.rowCount(), 3);

    const QModelIndex index1 = model.index(0, 0);
    QCOMPARE(index1.data(QPdfBookmarkModel::PageNumberRole).toInt(), 0);

    const QModelIndex index2 = model.index(1, 0);
    QCOMPARE(index2.data(QPdfBookmarkModel::PageNumberRole).toInt(), 1);

    const QModelIndex index2_1 = model.index(0, 0, index2);
    QCOMPARE(index2_1.data(QPdfBookmarkModel::PageNumberRole).toInt(), 1);

    const QModelIndex index3 = model.index(2, 0);
    QCOMPARE(index3.data(QPdfBookmarkModel::PageNumberRole).toInt(), 2);
}

QTEST_MAIN(tst_QPdfBookmarkModel)

#include "tst_qpdfbookmarkmodel.moc"
