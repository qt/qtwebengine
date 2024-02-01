// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFLINKMODEL_H
#define QPDFLINKMODEL_H

#include <QtPdf/qtpdfglobal.h>
#include <QtPdf/qpdfdocument.h>
#include <QtPdf/qpdflink.h>

#include <QtCore/QAbstractListModel>

#include <memory>

QT_BEGIN_NAMESPACE

class QPdfLinkModelPrivate;

class Q_PDF_EXPORT QPdfLinkModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QPdfDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageChanged)

public:
    enum class Role {
        Link = Qt::UserRole,
        Rectangle,
        Url,
        Page,
        Location,
        Zoom,
        NRoles
    };
    Q_ENUM(Role)
    explicit QPdfLinkModel(QObject *parent = nullptr);
    ~QPdfLinkModel() override;

    QPdfDocument *document() const;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    int page() const;

    QPdfLink linkAt(QPointF point) const;

public Q_SLOTS:
    void setDocument(QPdfDocument *document);
    void setPage(int page);

Q_SIGNALS:
    void documentChanged();
    void pageChanged(int page);

private Q_SLOTS:
    void onStatusChanged(QPdfDocument::Status status);

private:
    Q_DECLARE_PRIVATE(QPdfLinkModel)
    const std::unique_ptr<QPdfLinkModelPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QPDFLINKMODEL_H
