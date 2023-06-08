// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginehistory.h"
#include "qwebenginehistory_p.h"

#include "web_contents_adapter.h"

QT_BEGIN_NAMESPACE

/*!
    \fn QWebEngineHistoryItem::swap(QWebEngineHistoryItem &other)
    Swaps the history item with the \a other item.
*/

QWebEngineHistoryItemPrivate::QWebEngineHistoryItemPrivate(
        QtWebEngineCore::WebContentsAdapterClient *client, int index)
    : client(client), index(index)
{
}

QtWebEngineCore::WebContentsAdapter *QWebEngineHistoryItemPrivate::adapter() const
{
    return client ? client->webContentsAdapter() : nullptr;
}

QWebEngineHistoryItem::QWebEngineHistoryItem(QWebEngineHistoryItemPrivate *d) : d(d) { }
QWebEngineHistoryItem::QWebEngineHistoryItem(const QWebEngineHistoryItem &other) = default;
QWebEngineHistoryItem::QWebEngineHistoryItem(QWebEngineHistoryItem &&other) = default;
QWebEngineHistoryItem &QWebEngineHistoryItem::operator=(const QWebEngineHistoryItem &other) = default;
QWebEngineHistoryItem &QWebEngineHistoryItem::operator=(QWebEngineHistoryItem &&other) = default;
QWebEngineHistoryItem::~QWebEngineHistoryItem() = default;

QUrl QWebEngineHistoryItem::originalUrl() const
{
    Q_D(const QWebEngineHistoryItem);
    return d->adapter() ? d->adapter()->getNavigationEntryOriginalUrl(d->index) : QUrl();
}

QUrl QWebEngineHistoryItem::url() const
{
    Q_D(const QWebEngineHistoryItem);
    return d->adapter() ? d->adapter()->getNavigationEntryUrl(d->index) : QUrl();
}

QString QWebEngineHistoryItem::title() const
{
    Q_D(const QWebEngineHistoryItem);
    return d->adapter() ? d->adapter()->getNavigationEntryTitle(d->index) : QString();
}

QDateTime QWebEngineHistoryItem::lastVisited() const
{
    Q_D(const QWebEngineHistoryItem);
    return d->adapter() ? d->adapter()->getNavigationEntryTimestamp(d->index) : QDateTime();
}

/*!
    Returns the URL of the icon associated with the history item.

    \sa url(), originalUrl(), title()
*/
QUrl QWebEngineHistoryItem::iconUrl() const
{
    Q_D(const QWebEngineHistoryItem);
    return d->adapter() ? d->adapter()->getNavigationEntryIconUrl(d->index) : QUrl();
}

bool QWebEngineHistoryItem::isValid() const
{
    Q_D(const QWebEngineHistoryItem);
    if (!d->client)
        return false;
    return d->index >= 0 && d->index < d->adapter()->navigationEntryCount();
}

QWebEngineHistoryPrivate::QWebEngineHistoryPrivate(QtWebEngineCore::WebContentsAdapterClient *client,
                                                   const ImageProviderUrl &imageProviderUrl)
    : client(client), imageProviderUrl(imageProviderUrl)
{
    Q_ASSERT(client);
}

QWebEngineHistoryPrivate::~QWebEngineHistoryPrivate()
{
    // Invalidate shared item references possibly still out there.
    QList<QWebEngineHistoryItem>::iterator it, end;
    for (it = items.begin(), end = items.end(); it != end; ++it)
        it->d->client = nullptr;
}

void QWebEngineHistoryPrivate::updateItems() const
{
    // Keep track of items we return to be able to invalidate them
    // and avoid dangling references to our client.
    int entryCount = adapter()->navigationEntryCount();
    while (items.size() > entryCount) {
        items.last().d->client = nullptr;
        items.removeLast();
    }
    while (items.size() < entryCount) {
        int nextIndex = items.size();
        items.append(QWebEngineHistoryItem(new QWebEngineHistoryItemPrivate(client, nextIndex)));
    }
}

QtWebEngineCore::WebContentsAdapter *QWebEngineHistoryPrivate::adapter() const
{
    Q_ASSERT(client->webContentsAdapter());
    return client->webContentsAdapter();
}

QWebEngineHistoryModelPrivate::QWebEngineHistoryModelPrivate(const QWebEngineHistoryPrivate *history)
    : history(history)
{
    Q_ASSERT(history);
}

QWebEngineHistoryModelPrivate::~QWebEngineHistoryModelPrivate()
{
}

QtWebEngineCore::WebContentsAdapter *QWebEngineHistoryModelPrivate::adapter() const
{
    Q_ASSERT(history->adapter());
    return history->adapter();
}

int QWebEngineHistoryModelPrivate::count() const
{
    return adapter()->navigationEntryCount();
}

int QWebEngineHistoryModelPrivate::index(int index) const
{
    return index;
}

int QWebEngineHistoryModelPrivate::offsetForIndex(int index) const
{
    return index - adapter()->currentNavigationEntryIndex();
}

int QWebEngineBackHistoryModelPrivate::count() const
{
    return adapter()->currentNavigationEntryIndex();
}

int QWebEngineBackHistoryModelPrivate::index(int i) const
{
    Q_ASSERT(i >= 0 && i < count());
    return count() - 1 - i;
}

int QWebEngineBackHistoryModelPrivate::offsetForIndex(int index) const
{
    return - index - 1;
}

int QWebEngineForwardHistoryModelPrivate::count() const
{
    if (!adapter()->isInitialized())
        return 0;
    return adapter()->navigationEntryCount() - adapter()->currentNavigationEntryIndex() - 1;
}

int QWebEngineForwardHistoryModelPrivate::index(int i) const
{
    return adapter()->currentNavigationEntryIndex() + i + 1;
}

int QWebEngineForwardHistoryModelPrivate::offsetForIndex(int index) const
{
    return index + 1;
}

QWebEngineHistoryModel::QWebEngineHistoryModel(QWebEngineHistoryModelPrivate *d)
    : d_ptr(d)
{
}

QWebEngineHistoryModel::~QWebEngineHistoryModel()
{
}

QHash<int, QByteArray> QWebEngineHistoryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::ToolTipRole] = "toolTip";
    roles[UrlRole] = "url";
    roles[TitleRole] = "title";
    roles[OffsetRole] = "offset";
    roles[IconUrlRole] = "icon";
    return roles;
}

int QWebEngineHistoryModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    Q_D(const QWebEngineHistoryModel);
    return d->count();
}

QVariant QWebEngineHistoryModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QWebEngineHistoryModel);

    if (!index.isValid())
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
        case TitleRole:
            return d->adapter()->getNavigationEntryTitle(d->index(index.row()));

        case Qt::ToolTipRole:
        case UrlRole:
            return d->adapter()->getNavigationEntryUrl(d->index(index.row()));

        case OffsetRole:
            return d->offsetForIndex(index.row());

        case IconUrlRole: {
            QUrl url = QUrl(d->adapter()->getNavigationEntryIconUrl(d->index(index.row())));
            return d->history->urlOrImageProviderUrl(url);
        }
        default:
            break;
    }

    return QVariant();
}

void QWebEngineHistoryModel::reset()
{
    beginResetModel();
    endResetModel();
}

QWebEngineHistory::QWebEngineHistory(QWebEngineHistoryPrivate *d) : d_ptr(d) { }

QWebEngineHistory::~QWebEngineHistory() { }

void QWebEngineHistory::clear()
{
    Q_D(const QWebEngineHistory);
    d->adapter()->clearNavigationHistory();
    d->client->updateNavigationActions();
    reset();
}

QList<QWebEngineHistoryItem> QWebEngineHistory::items() const
{
    Q_D(const QWebEngineHistory);
    d->updateItems();
    return d->items;
}

QList<QWebEngineHistoryItem> QWebEngineHistory::backItems(int maxItems) const
{
    Q_D(const QWebEngineHistory);
    d->updateItems();
    const int end = currentItemIndex();
    const int start = std::max(0, end - maxItems);
    return d->items.mid(start, end - start);
}

QList<QWebEngineHistoryItem> QWebEngineHistory::forwardItems(int maxItems) const
{
    Q_D(const QWebEngineHistory);
    d->updateItems();
    const int start = currentItemIndex() + 1;
    const int end = std::min(count(), start + maxItems);
    return d->items.mid(start, end - start);
}

bool QWebEngineHistory::canGoBack() const
{
    Q_D(const QWebEngineHistory);
    return d->adapter()->canGoToOffset(-1);
}

bool QWebEngineHistory::canGoForward() const
{
    Q_D(const QWebEngineHistory);
    return d->adapter()->canGoToOffset(1);
}

void QWebEngineHistory::back()
{
    Q_D(const QWebEngineHistory);
    d->adapter()->navigateToOffset(-1);
}

void QWebEngineHistory::forward()
{
    Q_D(const QWebEngineHistory);
    d->adapter()->navigateToOffset(1);
}

void QWebEngineHistory::goToItem(const QWebEngineHistoryItem &item)
{
    Q_D(const QWebEngineHistory);
    Q_ASSERT(item.d->client == d->client);
    d->adapter()->navigateToIndex(item.d->index);
}

QWebEngineHistoryItem QWebEngineHistory::backItem() const
{
    return itemAt(currentItemIndex() - 1);
}

QWebEngineHistoryItem QWebEngineHistory::currentItem() const
{
    return itemAt(currentItemIndex());
}

QWebEngineHistoryItem QWebEngineHistory::forwardItem() const
{
    return itemAt(currentItemIndex() + 1);
}

QWebEngineHistoryItem QWebEngineHistory::itemAt(int i) const
{
    Q_D(const QWebEngineHistory);
    if (i >= 0 && i < count()) {
        d->updateItems();
        return d->items[i];
    } else {
        // Return an invalid item right away.
        QWebEngineHistoryItem item(new QWebEngineHistoryItemPrivate(0, i));
        Q_ASSERT(!item.isValid());
        return item;
    }
}

int QWebEngineHistory::currentItemIndex() const
{
    Q_D(const QWebEngineHistory);
    return d->adapter()->currentNavigationEntryIndex();
}

int QWebEngineHistory::count() const
{
    Q_D(const QWebEngineHistory);
    if (!d->adapter()->isInitialized())
        return 0;
    return d->adapter()->navigationEntryCount();
}

QWebEngineHistoryModel *QWebEngineHistory::itemsModel() const
{
    Q_D(const QWebEngineHistory);
    if (!d->navigationModel)
        d->navigationModel.reset(new QWebEngineHistoryModel(new QWebEngineHistoryModelPrivate(d)));
    return d->navigationModel.data();
}

QWebEngineHistoryModel *QWebEngineHistory::backItemsModel() const
{
    Q_D(const QWebEngineHistory);
    if (!d->backNavigationModel)
        d->backNavigationModel.reset(new QWebEngineHistoryModel(new QWebEngineBackHistoryModelPrivate(d)));
    return d->backNavigationModel.data();
}

QWebEngineHistoryModel *QWebEngineHistory::forwardItemsModel() const
{
    Q_D(const QWebEngineHistory);
    if (!d->forwardNavigationModel)
        d->forwardNavigationModel.reset(new QWebEngineHistoryModel(new QWebEngineForwardHistoryModelPrivate(d)));
    return d->forwardNavigationModel.data();
}

void QWebEngineHistory::reset()
{
    Q_D(QWebEngineHistory);
    if (d->navigationModel)
        d->navigationModel->reset();
    if (d->backNavigationModel)
        d->backNavigationModel->reset();
    if (d->forwardNavigationModel)
        d->forwardNavigationModel->reset();
}

QT_END_NAMESPACE

#include "moc_qwebenginehistory.cpp"
