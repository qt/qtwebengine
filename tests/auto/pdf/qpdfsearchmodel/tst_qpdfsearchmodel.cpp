// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#include <QtTest/QtTest>

#include <QPdfDocument>
#include <QPdfSearchModel>

class tst_QPdfSearchModel: public QObject
{
    Q_OBJECT

public:
    tst_QPdfSearchModel() {}

private slots:
    void findText();
};

void tst_QPdfSearchModel::findText()
{
    QPdfDocument document;
    QCOMPARE(document.load(QFINDTESTDATA("test.pdf")), QPdfDocument::NoError);

    QPdfSearchModel model;
    model.setDocument(&document);
    QList<QRectF> matches = model.matches(1, "ai");

    qDebug() << matches;
    QCOMPARE(matches.count(), 3);
}

QTEST_MAIN(tst_QPdfSearchModel)

#include "tst_qpdfsearchmodel.moc"
