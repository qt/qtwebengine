/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
        _Count
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
    QHash<int, QByteArray> m_roleNames;
    std::unique_ptr<QPdfBookmarkModelPrivate> d;

    Q_PRIVATE_SLOT(d, void _q_documentStatusChanged())

    friend struct QPdfBookmarkModelPrivate;
};

QT_END_NAMESPACE

#endif
