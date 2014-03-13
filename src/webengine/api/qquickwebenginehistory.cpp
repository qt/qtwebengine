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
#include "qquickwebenginehistory_p.h"
#include "qquickwebenginehistory_p_p.h"
#include "qquickwebengineloadrequest_p.h"
#include "qquickwebengineview_p_p.h"
#include "web_contents_adapter.h"
#include "web_engine_history.h"

QT_BEGIN_NAMESPACE

QQuickWebEngineHistoryListModelPrivate::QQuickWebEngineHistoryListModelPrivate(const QExplicitlySharedDataPointer<WebEngineHistory> &history,
                                                                               itemsFunctionPointer backForwardItems)
    : m_history(history)
    , backForwardItems(backForwardItems)
{
}

QList<WebEngineHistoryItem> QQuickWebEngineHistoryListModelPrivate::items() const
{
    return (m_history.data()->*backForwardItems)();
}


QQuickWebEngineHistoryListModel::QQuickWebEngineHistoryListModel()
    : QAbstractListModel()
{
}

QQuickWebEngineHistoryListModel::QQuickWebEngineHistoryListModel(QQuickWebEngineHistoryListModelPrivate *d)
    : QAbstractListModel()
    , d_ptr(d)
{
}

QQuickWebEngineHistoryListModel::~QQuickWebEngineHistoryListModel()
{
}

QHash<int, QByteArray> QQuickWebEngineHistoryListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[QQuickWebEngineHistory::UrlRole] = "url";
    roles[QQuickWebEngineHistory::TitleRole] = "title";
    return roles;
}

int QQuickWebEngineHistoryListModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    Q_D(const QQuickWebEngineHistoryListModel);
    return d->items().count();
}

QVariant QQuickWebEngineHistoryListModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QQuickWebEngineHistoryListModel);

    if (!index.isValid())
        return QVariant();

    if (role < QQuickWebEngineHistory::UrlRole || role > QQuickWebEngineHistory::TitleRole)
        return QVariant();

    if (role == QQuickWebEngineHistory::UrlRole) {
        const WebEngineHistoryItem item = d->items().at(index.row());
        Q_ASSERT(item.adapter());

        return QUrl(item.adapter()->getNavigationEntryUrl(item.index()));
    }

    if (role == QQuickWebEngineHistory::TitleRole) {
        const WebEngineHistoryItem item = d->items().at(index.row());
        Q_ASSERT(item.adapter());

        return QString(item.adapter()->getNavigationEntryTitle(item.index()));
    }

    return QVariant();
}

void QQuickWebEngineHistoryListModel::reset()
{
    beginResetModel();
    endResetModel();
}

WebEngineHistoryItem QQuickWebEngineHistoryListModel::itemAt(int index) const
{
    Q_D(const QQuickWebEngineHistoryListModel);
    Q_ASSERT(0 <= index && index < d->items().count());

    return d->items().at(index);
}


QQuickWebEngineHistory::QQuickWebEngineHistory(WebEngineHistory *history)
    : m_history(history)
    , m_backItems(new QQuickWebEngineHistoryListModel(new QQuickWebEngineHistoryListModelPrivate(m_history, &WebEngineHistory::backItems)))
    , m_forwardItems(new QQuickWebEngineHistoryListModel(new QQuickWebEngineHistoryListModelPrivate(m_history, &WebEngineHistory::forwardItems)))
    , m_backForwardItems(new QQuickWebEngineHistoryListModel(new QQuickWebEngineHistoryListModelPrivate(m_history, &WebEngineHistory::items)))
{
}

QQuickWebEngineHistory::~QQuickWebEngineHistory()
{
}

QQuickWebEngineHistoryListModel *QQuickWebEngineHistory::backItems() const
{
    return m_backItems.data();
}

QQuickWebEngineHistoryListModel *QQuickWebEngineHistory::forwardItems() const
{
    return m_forwardItems.data();
}

QQuickWebEngineHistoryListModel *QQuickWebEngineHistory::backForwardItems() const
{
    return m_backForwardItems.data();
}

bool QQuickWebEngineHistory::canGoBack() const
{
    return m_history->adapter()->canGoBack();
}

bool QQuickWebEngineHistory::canGoForward() const
{
    return m_history->adapter()->canGoForward();
}

void QQuickWebEngineHistory::goBackTo(int index)
{
    if (index < 0 || index >= m_backItems->rowCount()) return;

    WebEngineHistoryItem item = m_backItems->itemAt(index);
    Q_ASSERT(m_history->adapter() == item.adapter());

    m_history->adapter()->navigateToIndex(item.index());
}

void QQuickWebEngineHistory::goForwardTo(int index)
{
    if (index < 0 || index >= m_forwardItems->rowCount()) return;

    WebEngineHistoryItem item = m_forwardItems->itemAt(index);
    Q_ASSERT(m_history->adapter() == item.adapter());

    m_history->adapter()->navigateToIndex(item.index());
}

void QQuickWebEngineHistory::goBack()
{
    m_history->adapter()->navigateToOffset(-1);
}

void QQuickWebEngineHistory::goForward()
{
    m_history->adapter()->navigateToOffset(1);
}

int QQuickWebEngineHistory::currentItemIndex()
{
    return m_history->currentItemIndex();
}

void QQuickWebEngineHistory::onLoadingChanged(QQuickWebEngineLoadRequest *loadRequest)
{
    if (loadRequest->status() != QQuickWebEngineView::LoadSucceededStatus)
        return;

    m_backItems->reset();
    m_forwardItems->reset();
    m_backForwardItems->reset();
}

QT_END_NAMESPACE
