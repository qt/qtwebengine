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
#include "web_engine_history.h"
#include "web_contents_adapter.h"

WebEngineHistoryItem::WebEngineHistoryItem(WebContentsAdapter *adapter, int index)
    : m_adapter(adapter)
    , m_index(index)
{
}

WebEngineHistoryItem::~WebEngineHistoryItem()
{
}

bool WebEngineHistoryItem::isValid() const
{
    if (!m_adapter)
        return false;

    return m_index >= 0 && m_index < m_adapter->navigationEntryCount();
}

QUrl WebEngineHistoryItem::originalUrl() const
{
    return m_adapter ? m_adapter->getNavigationEntryOriginalUrl(m_index) : QUrl();
}

QUrl WebEngineHistoryItem::url() const
{
    return m_adapter ? m_adapter->getNavigationEntryUrl(m_index) : QUrl();
}

QString WebEngineHistoryItem::title() const
{
    return m_adapter ? m_adapter->getNavigationEntryTitle(m_index) : QString();
}

WebEngineHistory::WebEngineHistory(WebContentsAdapter *adapter)
    : m_adapter(adapter)
{
}

WebEngineHistory::~WebEngineHistory()
{
}

void WebEngineHistory::invalidateItems()
{
    // Invalidate shared item references possibly still out there.
    QList<WebEngineHistoryItem>::iterator it, end;
    for (it = m_items.begin(), end = m_items.end(); it != end; ++it)
        (*it).m_adapter = 0;
}

void WebEngineHistory::clear()
{
    m_adapter->clearNavigationHistory();
}

QList<WebEngineHistoryItem> WebEngineHistory::items() const
{
    updateItems();

    return m_items;
}

QList<WebEngineHistoryItem> WebEngineHistory::backItems(int maxItems) const
{
    updateItems();

    const int end = currentItemIndex();
    const int start = std::max(0, end - maxItems);

    return m_items.mid(start, end - start);
}

QList<WebEngineHistoryItem> WebEngineHistory::forwardItems(int maxItems) const
{
    updateItems();

    const int start = currentItemIndex() + 1;
    const int end = std::min(count(), start + maxItems);

    return m_items.mid(start, end - start);
}

bool WebEngineHistory::canGoBack() const
{
    return m_adapter->canGoBack();
}

bool WebEngineHistory::canGoForward() const
{
    return m_adapter->canGoForward();
}

void WebEngineHistory::back()
{
    m_adapter->navigateToOffset(-1);
}

void WebEngineHistory::forward()
{
    m_adapter->navigateToOffset(1);
}

void WebEngineHistory::goToItem(const WebEngineHistoryItem *item)
{
    Q_ASSERT(item->m_adapter == m_adapter);
    m_adapter->navigateToIndex(item->m_index);
}

WebEngineHistoryItem *WebEngineHistory::backItem() const
{
    return itemAt(currentItemIndex() - 1);
}

WebEngineHistoryItem *WebEngineHistory::currentItem() const
{
    updateItems();
    return &m_items[currentItemIndex()];
}

WebEngineHistoryItem *WebEngineHistory::forwardItem() const
{
    return itemAt(currentItemIndex() + 1);
}

WebEngineHistoryItem *WebEngineHistory::itemAt(int i) const
{
    if (i >= 0 && i < count()) {
        updateItems();
        return &m_items[i];
    }

    return NULL;
}

int WebEngineHistory::currentItemIndex() const
{
    return m_adapter->currentNavigationEntryIndex();
}

int WebEngineHistory::count() const
{
    return m_adapter->navigationEntryCount();
}


void WebEngineHistory::updateItems() const
{
    // Keep track of items we return to be able to invalidate them
    // and avoid dangling references to our adapter.
    int entryCount = m_adapter->navigationEntryCount();
    while (m_items.size() > entryCount) {
        m_items.last().m_adapter = 0;
        m_items.removeLast();
    }
    while (m_items.size() < entryCount) {
        int nextIndex = m_items.size();
        m_items.append(WebEngineHistoryItem(m_adapter, nextIndex));
    }
}
