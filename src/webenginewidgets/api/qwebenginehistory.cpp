/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebenginehistory.h"
#include "qwebenginehistory_p.h"

#include "qwebenginepage_p.h"
#include "web_contents_adapter.h"

#if defined(Q_OS_WIN)
#define __func__ __FUNCTION__
#endif

QT_BEGIN_NAMESPACE

QWebEngineHistoryItemPrivate::QWebEngineHistoryItemPrivate(QWebEnginePagePrivate *page, int index)
    : page(page)
    , index(index)
{
}

QWebEngineHistoryItem::QWebEngineHistoryItem(QWebEngineHistoryItemPrivate *d)
    : d(d)
{
}

QWebEngineHistoryItem::QWebEngineHistoryItem(const QWebEngineHistoryItem &other)
    : d(other.d)
{
}

QWebEngineHistoryItem &QWebEngineHistoryItem::operator=(const QWebEngineHistoryItem &other)
{
    d = other.d;
    return *this;
}

QWebEngineHistoryItem::~QWebEngineHistoryItem()
{
}

QUrl QWebEngineHistoryItem::originalUrl() const
{
    Q_D(const QWebEngineHistoryItem);
    return d->page ? d->page->webContents()->getNavigationEntryOriginalUrl(d->index) : QUrl();
}

QUrl QWebEngineHistoryItem::url() const
{
    Q_D(const QWebEngineHistoryItem);
    return d->page ? d->page->webContents()->getNavigationEntryUrl(d->index) : QUrl();
}

QString QWebEngineHistoryItem::title() const
{
    Q_D(const QWebEngineHistoryItem);
    return d->page ? d->page->webContents()->getNavigationEntryTitle(d->index) : QString();
}

QDateTime QWebEngineHistoryItem::lastVisited() const
{
    qWarning("Not implemented: %s", __func__);
    return QDateTime();
}

QIcon QWebEngineHistoryItem::icon() const
{
    qWarning("Not implemented: %s", __func__);
    return QIcon();
}

QVariant QWebEngineHistoryItem::userData() const
{
    return QVariant();
}

void QWebEngineHistoryItem::setUserData(const QVariant& userData)
{
    Q_UNUSED(userData);
    qWarning("Not implemented: %s", __func__);
}

bool QWebEngineHistoryItem::isValid() const
{
    Q_D(const QWebEngineHistoryItem);
    if (!d->page)
        return false;
    return d->index >= 0 && d->index < d->page->webContents()->navigationEntryCount();
}

QWebEngineHistoryPrivate::QWebEngineHistoryPrivate(QWebEnginePagePrivate *page)
    : page(page)
{
}

QWebEngineHistoryPrivate::~QWebEngineHistoryPrivate()
{
    // Invalidate shared item references possibly still out there.
    QList<QWebEngineHistoryItem>::iterator it, end;
    for (it = items.begin(), end = items.end(); it != end; ++it)
        it->d->page = 0;
}

void QWebEngineHistoryPrivate::updateItems() const
{
    // Keep track of items we return to be able to invalidate them
    // and avoid dangling references to our page.
    int entryCount = page->webContents()->navigationEntryCount();
    while (items.size() > entryCount) {
        items.last().d->page = 0;
        items.removeLast();
    }
    while (items.size() < entryCount) {
        int nextIndex = items.size();
        items.append(QWebEngineHistoryItem(new QWebEngineHistoryItemPrivate(page, nextIndex)));
    }
}

QWebEngineHistory::QWebEngineHistory(QWebEngineHistoryPrivate *d)
    : d_ptr(d)
{
}

QWebEngineHistory::~QWebEngineHistory()
{
}

void QWebEngineHistory::clear()
{
    Q_D(const QWebEngineHistory);
    d->page->webContents()->clearNavigationHistory();
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
    return d->page->webContents()->canGoBack();
}

bool QWebEngineHistory::canGoForward() const
{
    Q_D(const QWebEngineHistory);
    return d->page->webContents()->canGoForward();
}

void QWebEngineHistory::back()
{
    Q_D(const QWebEngineHistory);
    d->page->webContents()->navigateToOffset(-1);
}

void QWebEngineHistory::forward()
{
    Q_D(const QWebEngineHistory);
    d->page->webContents()->navigateToOffset(1);
}

void QWebEngineHistory::goToItem(const QWebEngineHistoryItem &item)
{
    Q_D(const QWebEngineHistory);
    Q_ASSERT(item.d->page == d->page);
    d->page->webContents()->navigateToIndex(item.d->index);
}

QWebEngineHistoryItem QWebEngineHistory::backItem() const
{
    return itemAt(currentItemIndex() - 1);
}

QWebEngineHistoryItem QWebEngineHistory::currentItem() const
{
    Q_D(const QWebEngineHistory);
    d->updateItems();
    return d->items[currentItemIndex()];
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
    return d->page->webContents()->currentNavigationEntryIndex();
}

int QWebEngineHistory::count() const
{
    Q_D(const QWebEngineHistory);
    return d->page->webContents()->navigationEntryCount();
}

int QWebEngineHistory::maximumItemCount() const
{
    return 100;
}

void QWebEngineHistory::setMaximumItemCount(int count)
{
    Q_UNUSED(count);
    qWarning("Not implemented: %s", __func__);
}

QDataStream& operator<<(QDataStream& stream, const QWebEngineHistory& history)
{
    history.d_func()->page->webContents()->serializeNavigationHistory(stream);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, QWebEngineHistory& history)
{
    history.d_func()->page->recreateFromSerializedHistory(stream);
    return stream;
}

QT_END_NAMESPACE
