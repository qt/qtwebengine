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

int WebEngineHistoryItem::index() const
{
    return m_index;
}

WebContentsAdapter *WebEngineHistoryItem::adapter() const
{
    return m_adapter;
}

bool WebEngineHistoryItem::isValid() const
{
    if (!m_adapter)
        return false;

    return m_index >= 0 && m_index < m_adapter->navigationEntryCount();
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

QList<WebEngineHistoryItem> WebEngineHistory::backItems() const
{
    updateItems();
    return m_items.mid(0, currentItemIndex());
}

QList<WebEngineHistoryItem> WebEngineHistory::forwardItems() const
{
    updateItems();
    return m_items.mid(currentItemIndex() + 1, count());
}

WebEngineHistoryItem *WebEngineHistory::itemAt(int i) const
{
    if (i >= 0 && i < count()) {
        updateItems();
        return &m_items[i];
    }

    return 0;
}

int WebEngineHistory::currentItemIndex() const
{
    return m_adapter->currentNavigationEntryIndex();
}

int WebEngineHistory::count() const
{
    return m_adapter->navigationEntryCount();
}

WebContentsAdapter *WebEngineHistory::adapter() const
{
    return m_adapter;
}

void WebEngineHistory::setAdapter(WebContentsAdapter *newWebContents)
{
    m_adapter = newWebContents;
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
