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

#ifndef QPDFSEARCHMODEL_H
#define QPDFSEARCHMODEL_H

#include <QtPdf/qtpdfglobal.h>

#include <QtCore/qabstractitemmodel.h>
#include <QtPdf/qpdfdocument.h>
#include <QtPdf/qpdfsearchresult.h>

QT_BEGIN_NAMESPACE

class QPdfSearchModelPrivate;

class Q_PDF_EXPORT QPdfSearchModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QPdfDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)

public:
    enum class Role : int {
        Page = Qt::UserRole,
        IndexOnPage,
        Location,
        ContextBefore,
        ContextAfter,
        _Count
    };
    Q_ENUM(Role)
    explicit QPdfSearchModel(QObject *parent = nullptr);
    ~QPdfSearchModel();

    QVector<QPdfSearchResult> resultsOnPage(int page) const;
    QPdfSearchResult resultAtIndex(int index) const;

    QPdfDocument *document() const;
    QString searchString() const;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

public Q_SLOTS:
    void setSearchString(QString searchString);
    void setDocument(QPdfDocument *document);

Q_SIGNALS:
    void documentChanged();
    void searchStringChanged();

protected:
    void updatePage(int page);
    void timerEvent(QTimerEvent *event) override;

private:
    QHash<int, QByteArray> m_roleNames;
    Q_DECLARE_PRIVATE(QPdfSearchModel)
};

QT_END_NAMESPACE

#endif // QPDFSEARCHMODEL_H
