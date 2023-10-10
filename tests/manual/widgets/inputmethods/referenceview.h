// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef REFERENCEVIEW_H
#define REFERENCEVIEW_H

#include <QLineEdit>
#include <QWidget>

class ReferenceView : public QWidget
{
    Q_OBJECT
public:
    explicit ReferenceView(QWidget *parent = 0);
    QLineEdit *referenceInput();

private:
    QLineEdit *m_referenceInput;
};

#endif // REFERENCEVIEW_H
