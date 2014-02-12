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
#include "qwebenginepage_p.h"
#include "web_engine_history.h"

#if defined(Q_OS_WIN)
#define __func__ __FUNCTION__
#endif

QT_BEGIN_NAMESPACE

QWebEngineHistoryItem::QWebEngineHistoryItem(const WebEngineHistoryItem *item, const QExplicitlySharedDataPointer<WebEngineHistory>& history)
    : m_item(item)
    , m_history(history)
{
}

QWebEngineHistoryItem::QWebEngineHistoryItem(const QWebEngineHistoryItem &other)
    : m_item(other.m_item)
    , m_history(other.m_history)
{
}

QWebEngineHistoryItem &QWebEngineHistoryItem::operator=(const QWebEngineHistoryItem &other)
{
    m_item = other.m_item;
    m_history = other.m_history;
    return *this;
}

QWebEngineHistoryItem::~QWebEngineHistoryItem()
{
}

QUrl QWebEngineHistoryItem::originalUrl() const
{
    return m_history ? m_item->originalUrl() : QUrl();
}

QUrl QWebEngineHistoryItem::url() const
{
    return m_history ? m_item->url() : QUrl();
}

QString QWebEngineHistoryItem::title() const
{
    return m_history ? m_item->title() : QString();
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
    if (!m_item || !m_history)
        return false;

    return m_item->isValid();
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
        items.append(QWebEngineHistoryItem(&m_history->items().at(i), m_history));
    }

    return items;
}

QList<QWebEngineHistoryItem> QWebEngineHistory::backItems(int maxItems) const
{
    QList<QWebEngineHistoryItem> items;

    int count = m_history->backItems(maxItems).count();
    for (int i = 0; i < count; i++) {
        items.append(QWebEngineHistoryItem(&m_history->backItems(maxItems).at(i), m_history));
    }

    return items;
}

QList<QWebEngineHistoryItem> QWebEngineHistory::forwardItems(int maxItems) const
{
    QList<QWebEngineHistoryItem> items;

    int count = m_history->forwardItems(maxItems).count();
    for (int i = 0; i < count; i++) {
        items.append(QWebEngineHistoryItem(&m_history->forwardItems(maxItems).at(i), m_history));
    }

    return items;
}

bool QWebEngineHistory::canGoBack() const
{
    return m_history->canGoBack();
}

bool QWebEngineHistory::canGoForward() const
{
    return m_history->canGoForward();
}

void QWebEngineHistory::back()
{
    m_history->back();
}

void QWebEngineHistory::forward()
{
    m_history->forward();
}

void QWebEngineHistory::goToItem(const QWebEngineHistoryItem item)
{
    m_history->goToItem(item.m_item);
}

QWebEngineHistoryItem QWebEngineHistory::backItem() const
{
    return QWebEngineHistoryItem(m_history->backItem(), m_history);
}

QWebEngineHistoryItem QWebEngineHistory::currentItem() const
{
    return QWebEngineHistoryItem(m_history->currentItem(), m_history);
}

QWebEngineHistoryItem QWebEngineHistory::forwardItem() const
{
    return QWebEngineHistoryItem(m_history->forwardItem(), m_history);
}

QWebEngineHistoryItem QWebEngineHistory::itemAt(int i) const
{
    WebEngineHistoryItem *item = m_history->itemAt(i);

    // Return an invalid item right away.
    if (!item)
        return QWebEngineHistoryItem(NULL, m_history);

    return QWebEngineHistoryItem(item, m_history);
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
