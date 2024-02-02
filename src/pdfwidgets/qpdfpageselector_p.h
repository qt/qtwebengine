// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFPAGESELECTOR_P_H
#define QPDFPAGESELECTOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qpdfpageselector.h"

#include <QtWidgets/qspinbox.h>

#include <QPointer>

QT_BEGIN_NAMESPACE

class QPdfPageSelectorSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    QPdfPageSelectorSpinBox() : QPdfPageSelectorSpinBox(nullptr) {}
    explicit QPdfPageSelectorSpinBox(QWidget *parent);
    ~QPdfPageSelectorSpinBox();

    void setDocument(QPdfDocument *document);
    QPdfDocument *document() const { return m_document.get(); }

Q_SIGNALS:
    void _q_documentChanged(QPdfDocument *document);

protected:
    int valueFromText(const QString &text) const override;
    QString textFromValue(int value) const override;
    QValidator::State validate(QString &text, int &pos) const override;

private:
    void documentStatusChanged();
private:
    QPointer<QPdfDocument> m_document;
    QMetaObject::Connection m_documentStatusChangedConnection;
};

class QPdfPageSelectorPrivate
{
public:
    QPdfPageSelectorSpinBox *spinBox;
};

QT_END_NAMESPACE

#endif // QPDFPAGESELECTOR_P_H
