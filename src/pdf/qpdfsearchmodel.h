// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFSEARCHMODEL_H
#define QPDFSEARCHMODEL_H

#include <QtPdf/qtpdfglobal.h>

#include <QtCore/qabstractitemmodel.h>
#include <QtPdf/qpdfdocument.h>
#include <QtPdf/qpdflink.h>

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
        NRoles
    };
    Q_ENUM(Role)
    QPdfSearchModel() : QPdfSearchModel(nullptr) {}
    explicit QPdfSearchModel(QObject *parent);
    ~QPdfSearchModel() override;

    QList<QPdfLink> resultsOnPage(int page) const;
    QPdfLink resultAtIndex(int index) const;

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
