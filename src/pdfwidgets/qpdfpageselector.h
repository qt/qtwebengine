// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFPAGESELECTOR_H
#define QPDFPAGESELECTOR_H

#include <QtPdfWidgets/qtpdfwidgetsglobal.h>

#include <QtWidgets/qwidget.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QPdfDocument;
class QPdfPageSelectorPrivate;

class Q_PDF_WIDGETS_EXPORT QPdfPageSelector : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QPdfDocument* document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged USER true)
    Q_PROPERTY(QString currentPageLabel READ currentPageLabel NOTIFY currentPageLabelChanged)
public:
    QPdfPageSelector() : QPdfPageSelector(nullptr) {}
    explicit QPdfPageSelector(QWidget *parent);
    ~QPdfPageSelector() override;

    void setDocument(QPdfDocument *document);
    QPdfDocument *document() const;

    int currentPage() const;
    QString currentPageLabel() const;

public Q_SLOTS:
    void setCurrentPage(int index);

Q_SIGNALS:
    void documentChanged(QPdfDocument *document);
    void currentPageChanged(int index);
    void currentPageLabelChanged(const QString &label);

private:
    Q_DECLARE_PRIVATE(QPdfPageSelector)
    const std::unique_ptr<QPdfPageSelectorPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QPDFPAGESELECTOR_H
