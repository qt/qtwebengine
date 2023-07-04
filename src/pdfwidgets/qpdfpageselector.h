// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFPAGESELECTOR_H
#define QPDFPAGESELECTOR_H

#include <QtPdfWidgets/qtpdfwidgetsglobal.h>
#include <QtWidgets/qspinbox.h>

QT_BEGIN_NAMESPACE

class QPdfDocument;
class QPdfPageNavigator;
class QPdfPageSelectorPrivate;

class Q_PDF_WIDGETS_EXPORT QPdfPageSelector : public QSpinBox
{
    Q_OBJECT

    Q_PROPERTY(QPdfDocument* document READ document WRITE setDocument NOTIFY documentChanged)

public:
    QPdfPageSelector() : QPdfPageSelector(nullptr) {}
    explicit QPdfPageSelector(QWidget *parent);
    ~QPdfPageSelector();

    void setDocument(QPdfDocument *document);
    QPdfDocument *document() const;

Q_SIGNALS:
    void documentChanged(QPdfDocument *document);

protected:
    int valueFromText(const QString &text) const override;
    QString textFromValue(int value) const override;
    QValidator::State validate(QString &text, int &pos) const override;

private:
    Q_DECLARE_PRIVATE(QPdfPageSelector)
    QScopedPointer<QPdfPageSelectorPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QPDFPAGESELECTOR_H
