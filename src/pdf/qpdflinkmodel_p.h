// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFLINKMODEL_P_H
#define QPDFLINKMODEL_P_H

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

#include "qtpdfglobal.h"
#include "qpdfdocument.h"

#include <QObject>
#include <QAbstractListModel>

QT_BEGIN_NAMESPACE

class QPdfLinkModelPrivate;

class Q_PDF_EXPORT QPdfLinkModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QPdfDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageChanged)

public:
    enum class Role : int {
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
    ~QPdfLinkModel();

    QPdfDocument *document() const;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    int page() const;

public Q_SLOTS:
    void setDocument(QPdfDocument *document);
    void setPage(int page);

Q_SIGNALS:
    void documentChanged();
    void pageChanged(int page);

private Q_SLOTS:
    void onStatusChanged(QPdfDocument::Status status);

private:
    QHash<int, QByteArray> m_roleNames;
    Q_DECLARE_PRIVATE(QPdfLinkModel)
};

QT_END_NAMESPACE

#endif // QPDFLINKMODEL_P_H
