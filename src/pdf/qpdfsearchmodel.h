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
    QPdfSearchModel() : QPdfSearchModel(nullptr) {}
    explicit QPdfSearchModel(QObject *parent);
    ~QPdfSearchModel() override;

    QList<QPdfSearchResult> resultsOnPage(int page) const;
    QPdfSearchResult resultAtIndex(int index) const;

    QPdfDocument *document() const;
    QString searchString() const;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

public Q_SLOTS:
    void setSearchString(const QString &searchString);
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
