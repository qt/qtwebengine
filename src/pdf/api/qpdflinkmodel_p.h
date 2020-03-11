/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
        Rect = Qt::UserRole,
        Url,
        Page,
        Location,
        Zoom,
        _Count
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
