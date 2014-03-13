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
#include "qwebenginehistoryitem_p.h"
#include "qwebenginepage_p.h"

#include "web_contents_adapter.h"
#include "web_engine_history.h"

#if defined(Q_OS_WIN)
#define __func__ __FUNCTION__
#endif

QT_BEGIN_NAMESPACE

QWebEngineHistoryItemPrivate::QWebEngineHistoryItemPrivate(const WebEngineHistoryItem *item, const QExplicitlySharedDataPointer<WebEngineHistory> &history)
    : item(item)
    , history(history)
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

bool QWebEngineHistoryItem::isValid() const
{
    Q_D(const QWebEngineHistoryItem);
    if (!d->item || !d->history)
        return false;

    return d->item->isValid();
}

QUrl QWebEngineHistoryItem::url() const
{
    Q_D(const QWebEngineHistoryItem);
    if (!d->history || !d->item->adapter())
        return QUrl();

    return d->item->adapter()->getNavigationEntryUrl(d->item->index());
}


QUrl QWebEngineHistoryItem::originalUrl() const
{
    Q_D(const QWebEngineHistoryItem);
    if (!d->history || !d->item->adapter())
        return QUrl();

    return d->item->adapter()->getNavigationEntryOriginalUrl(d->item->index());
}

QString QWebEngineHistoryItem::title() const
{
    Q_D(const QWebEngineHistoryItem);
    if (!d->history || !d->item->adapter())
        return QString();

    return d->item->adapter()->getNavigationEntryTitle(d->item->index());
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



QWebEngineHistory::QWebEngineHistory(WebEngineHistory *history)
    : m_history(history)
{
}

QWebEngineHistory::~QWebEngineHistory()
{
    m_history->invalidateItems();
}

void QWebEngineHistory::clear()
{
    m_history->clear();
}

QList<QWebEngineHistoryItem> QWebEngineHistory::items() const
{
    QList<QWebEngineHistoryItem> items;

    int count = m_history->items().count();
    Q_ASSERT(m_history->count() == count);
    for (int i = 0; i < count; i++) {
        items.append(QWebEngineHistoryItem(new QWebEngineHistoryItemPrivate(&m_history->items().at(i), m_history)));
    }

    return items;
}

QList<QWebEngineHistoryItem> QWebEngineHistory::backItems(int maxItems) const
{
    QList<QWebEngineHistoryItem> items;

    int count = m_history->backItems().count();
    int start = count - maxItems;
    for (int i = (start > 0 ? start : 0) ; i < count; i++) {
        items.append(QWebEngineHistoryItem(new QWebEngineHistoryItemPrivate(&m_history->items().at(i), m_history)));
    }

    return items;
}

QList<QWebEngineHistoryItem> QWebEngineHistory::forwardItems(int maxItems) const
{
    QList<QWebEngineHistoryItem> items;

    int count = std::min(m_history->forwardItems().count(), maxItems);
    for (int i = 0; i < count; i++) {
        items.append(QWebEngineHistoryItem(new QWebEngineHistoryItemPrivate(&m_history->items().at(i), m_history)));
    }

    return items;
}

bool QWebEngineHistory::canGoBack() const
{
    return m_history->adapter()->canGoBack();
}

bool QWebEngineHistory::canGoForward() const
{
    return m_history->adapter()->canGoForward();
}

void QWebEngineHistory::back()
{
    m_history->adapter()->navigateToOffset(-1);
}

void QWebEngineHistory::forward()
{
    m_history->adapter()->navigateToOffset(1);
}

void QWebEngineHistory::goToItem(const QWebEngineHistoryItem item)
{
    Q_ASSERT(m_history->adapter() == item.d->item->adapter());
    m_history->adapter()->navigateToIndex(item.d->item->index());
}

QWebEngineHistoryItem QWebEngineHistory::backItem() const
{
    WebEngineHistoryItem *item = m_history->itemAt(m_history->currentItemIndex() - 1);
    return QWebEngineHistoryItem(new QWebEngineHistoryItemPrivate(item, m_history));
}

QWebEngineHistoryItem QWebEngineHistory::currentItem() const
{
    WebEngineHistoryItem *item = m_history->itemAt(m_history->currentItemIndex());
    return QWebEngineHistoryItem(new QWebEngineHistoryItemPrivate(item, m_history));
}

QWebEngineHistoryItem QWebEngineHistory::forwardItem() const
{
    WebEngineHistoryItem *item = m_history->itemAt(m_history->currentItemIndex() + 1);
    return QWebEngineHistoryItem(new QWebEngineHistoryItemPrivate(item, m_history));
}

QWebEngineHistoryItem QWebEngineHistory::itemAt(int i) const
{
    WebEngineHistoryItem *item = m_history->itemAt(i);

    // Return an invalid item right away.
    if (!item)
        return QWebEngineHistoryItem(new QWebEngineHistoryItemPrivate(0, m_history));

    return QWebEngineHistoryItem(new QWebEngineHistoryItemPrivate(item, m_history));
}

int QWebEngineHistory::currentItemIndex() const
{
    return m_history->currentItemIndex();
}

int QWebEngineHistory::count() const
{
    return m_history->count();
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
    Q_UNUSED(history);
    qWarning("Not implemented: %s", __func__);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, QWebEngineHistory& history)
{
    Q_UNUSED(history);
    qWarning("Not implemented: %s", __func__);
    return stream;
}

QT_END_NAMESPACE
