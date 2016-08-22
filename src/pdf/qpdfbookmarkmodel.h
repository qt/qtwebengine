/******************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt PDF Module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#ifndef QPDFBOOKMARKMODEL_H
#define QPDFBOOKMARKMODEL_H

#include "qtpdfglobal.h"

#include <QAbstractItemModel>

QT_BEGIN_NAMESPACE

class QPdfDocument;
class QPdfBookmarkModelPrivate;

class Q_PDF_EXPORT QPdfBookmarkModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QPdfDocument* document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(StructureMode structureMode READ structureMode WRITE setStructureMode NOTIFY structureModeChanged)

public:
    enum StructureMode
    {
        TreeMode,
        ListMode
    };
    Q_ENUM(StructureMode)

    enum Role
    {
        TitleRole = Qt::DisplayRole,
        LevelRole = Qt::UserRole
    };
    Q_ENUM(Role)

    explicit QPdfBookmarkModel(QObject *parent = Q_NULLPTR);

    QPdfDocument* document() const;
    void setDocument(QPdfDocument *document);

    StructureMode structureMode() const;
    void setStructureMode(StructureMode mode);

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void documentChanged(QPdfDocument *document);
    void structureModeChanged(QPdfBookmarkModel::StructureMode structureMode);

private:
    Q_DECLARE_PRIVATE(QPdfBookmarkModel)

    Q_PRIVATE_SLOT(d_func(), void _q_documentStatusChanged())
};

QT_END_NAMESPACE

#endif
