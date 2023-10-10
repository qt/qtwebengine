// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFBOOKMARKMODEL_H
#define QPDFBOOKMARKMODEL_H

#include <QtPdf/qtpdfglobal.h>
#include <QtCore/qabstractitemmodel.h>

QT_BEGIN_NAMESPACE

class QPdfDocument;
struct QPdfBookmarkModelPrivate;

class Q_PDF_EXPORT QPdfBookmarkModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QPdfDocument* document READ document WRITE setDocument NOTIFY documentChanged)

public:
    enum class Role : int
    {
        Title = Qt::UserRole,
        Level,
        Page,
        Location,
        Zoom,
        NRoles
    };
    Q_ENUM(Role)

    QPdfBookmarkModel() : QPdfBookmarkModel(nullptr) {}
    explicit QPdfBookmarkModel(QObject *parent);
    ~QPdfBookmarkModel() override;

    QPdfDocument* document() const;
    void setDocument(QPdfDocument *document);

    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void documentChanged(QPdfDocument *document);

private:
    std::unique_ptr<QPdfBookmarkModelPrivate> d;

    Q_PRIVATE_SLOT(d, void _q_documentStatusChanged())

    friend struct QPdfBookmarkModelPrivate;
};

QT_END_NAMESPACE

#endif
