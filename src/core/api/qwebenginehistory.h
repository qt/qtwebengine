// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEHISTORY_H
#define QWEBENGINEHISTORY_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qurl.h>
#include <QtCore/qstring.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QWebEngineHistory;
class QWebEngineHistoryPrivate;
class QWebEngineHistoryItemPrivate;
class QWebEngineHistoryModelPrivate;
class QWebEnginePagePrivate;
class QQuickWebEngineViewPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineHistoryItem
{
public:
    QWebEngineHistoryItem(const QWebEngineHistoryItem &other);
    QWebEngineHistoryItem(QWebEngineHistoryItem &&other);
    QWebEngineHistoryItem &operator=(const QWebEngineHistoryItem &other);
    QWebEngineHistoryItem &operator=(QWebEngineHistoryItem &&other);
    ~QWebEngineHistoryItem();

    QUrl originalUrl() const;
    QUrl url() const;

    QString title() const;
    QDateTime lastVisited() const;
    QUrl iconUrl() const;

    bool isValid() const;

    void swap(QWebEngineHistoryItem &other) noexcept { d.swap(other.d); }

private:
    QWebEngineHistoryItem(QWebEngineHistoryItemPrivate *priv);
    Q_DECLARE_PRIVATE_D(d.data(), QWebEngineHistoryItem)
    QExplicitlySharedDataPointer<QWebEngineHistoryItemPrivate> d;
    friend class QWebEngineHistory;
    friend class QWebEngineHistoryPrivate;
};

Q_DECLARE_SHARED(QWebEngineHistoryItem)

class Q_WEBENGINECORE_EXPORT QWebEngineHistoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        UrlRole = Qt::UserRole,
        TitleRole,
        OffsetRole,
        IconUrlRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void reset();

private:
    QWebEngineHistoryModel(QWebEngineHistoryModelPrivate *);
    virtual ~QWebEngineHistoryModel();

    Q_DISABLE_COPY(QWebEngineHistoryModel)
    Q_DECLARE_PRIVATE(QWebEngineHistoryModel)
    QScopedPointer<QWebEngineHistoryModelPrivate> d_ptr;

    friend class QWebEngineHistory;
    friend class QWebEngineHistoryPrivate;
    friend void QScopedPointerDeleter<QWebEngineHistoryModel>::cleanup(QWebEngineHistoryModel *) noexcept;
};

class Q_WEBENGINECORE_EXPORT QWebEngineHistory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QWebEngineHistoryModel *items READ itemsModel CONSTANT FINAL)
    Q_PROPERTY(QWebEngineHistoryModel *backItems READ backItemsModel CONSTANT FINAL)
    Q_PROPERTY(QWebEngineHistoryModel *forwardItems READ forwardItemsModel CONSTANT FINAL)

public:
    Q_REVISION(1) Q_INVOKABLE void clear();

    QList<QWebEngineHistoryItem> items() const;
    QList<QWebEngineHistoryItem> backItems(int maxItems) const;
    QList<QWebEngineHistoryItem> forwardItems(int maxItems) const;

    bool canGoBack() const;
    bool canGoForward() const;

    void back();
    void forward();
    void goToItem(const QWebEngineHistoryItem &item);

    QWebEngineHistoryItem backItem() const;
    QWebEngineHistoryItem currentItem() const;
    QWebEngineHistoryItem forwardItem() const;
    QWebEngineHistoryItem itemAt(int i) const;

    int currentItemIndex() const;

    int count() const;

    QWebEngineHistoryModel *itemsModel() const;
    QWebEngineHistoryModel *backItemsModel() const;
    QWebEngineHistoryModel *forwardItemsModel() const;

private:
    QWebEngineHistory(QWebEngineHistoryPrivate *d);
    ~QWebEngineHistory();

    Q_DISABLE_COPY(QWebEngineHistory)
    Q_DECLARE_PRIVATE(QWebEngineHistory)
    QScopedPointer<QWebEngineHistoryPrivate> d_ptr;

    void reset();

    friend Q_WEBENGINECORE_EXPORT QDataStream &operator>>(QDataStream &, QWebEngineHistory &);
    friend Q_WEBENGINECORE_EXPORT QDataStream &operator<<(QDataStream &, const QWebEngineHistory &);
    friend class QWebEnginePagePrivate;
    friend class QQuickWebEngineViewPrivate;
    friend void QScopedPointerDeleter<QWebEngineHistory>::cleanup(QWebEngineHistory *) noexcept;
};

QT_END_NAMESPACE

#endif // QWEBENGINEHISTORY_H
