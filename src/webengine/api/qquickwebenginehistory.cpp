/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickwebenginehistory_p.h"
#include "qquickwebenginehistory_p_p.h"
#include "qquickwebengineloadrequest_p.h"
#include "qquickwebengineview_p_p.h"
#include "web_contents_adapter.h"

QT_BEGIN_NAMESPACE

QQuickWebEngineHistoryListModelPrivate::QQuickWebEngineHistoryListModelPrivate(QQuickWebEngineViewPrivate *view)
    : view(view)
{
}

QQuickWebEngineHistoryListModelPrivate::~QQuickWebEngineHistoryListModelPrivate()
{
}

int QQuickWebEngineHistoryListModelPrivate::count() const
{
    if (!adapter())
        return -1;
    return adapter()->navigationEntryCount();
}

int QQuickWebEngineHistoryListModelPrivate::index(int index) const
{
    return index;
}

int QQuickWebEngineHistoryListModelPrivate::offsetForIndex(int index) const
{
    return index - adapter()->currentNavigationEntryIndex();
}

QtWebEngineCore::WebContentsAdapter *QQuickWebEngineHistoryListModelPrivate::adapter() const
{
    return view->adapter.data();
}

QQuickWebEngineBackHistoryListModelPrivate::QQuickWebEngineBackHistoryListModelPrivate(QQuickWebEngineViewPrivate *view)
    : QQuickWebEngineHistoryListModelPrivate(view)
{
}

int QQuickWebEngineBackHistoryListModelPrivate::count() const
{
    if (!adapter())
        return -1;
    return adapter()->currentNavigationEntryIndex();
}

int QQuickWebEngineBackHistoryListModelPrivate::index(int i) const
{
    Q_ASSERT(i >= 0 && i < count());
    return count() - 1 - i;
}

int QQuickWebEngineBackHistoryListModelPrivate::offsetForIndex(int index) const
{
    return - index - 1;
}

QQuickWebEngineForwardHistoryListModelPrivate::QQuickWebEngineForwardHistoryListModelPrivate(QQuickWebEngineViewPrivate *view)
    : QQuickWebEngineHistoryListModelPrivate(view)
{
}

int QQuickWebEngineForwardHistoryListModelPrivate::count() const
{
    if (!adapter())
        return -1;
    return adapter()->navigationEntryCount() - adapter()->currentNavigationEntryIndex() - 1;
}

int QQuickWebEngineForwardHistoryListModelPrivate::index(int i) const
{
    return adapter()->currentNavigationEntryIndex() + i + 1;
}

int QQuickWebEngineForwardHistoryListModelPrivate::offsetForIndex(int index) const
{
    return index + 1;
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
    roles[QQuickWebEngineHistory::OffsetRole] = "offset";
    return roles;
}

int QQuickWebEngineHistoryListModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    Q_D(const QQuickWebEngineHistoryListModel);
    return d->count();
}

QVariant QQuickWebEngineHistoryListModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QQuickWebEngineHistoryListModel);

    if (!index.isValid())
        return QVariant();

    if (role < QQuickWebEngineHistory::UrlRole || role > QQuickWebEngineHistory::OffsetRole)
        return QVariant();

    if (role == QQuickWebEngineHistory::UrlRole)
        return QUrl(d->adapter()->getNavigationEntryUrl(d->index(index.row())));

    if (role == QQuickWebEngineHistory::TitleRole)
        return QString(d->adapter()->getNavigationEntryTitle(d->index(index.row())));

    if (role == QQuickWebEngineHistory::OffsetRole)
        return d->offsetForIndex(index.row());
    return QVariant();
}

void QQuickWebEngineHistoryListModel::reset()
{
    beginResetModel();
    endResetModel();
}

QQuickWebEngineHistoryPrivate::QQuickWebEngineHistoryPrivate(QQuickWebEngineViewPrivate *view)
    : m_view(view)
{
}

QQuickWebEngineHistoryPrivate::~QQuickWebEngineHistoryPrivate()
{
}

QQuickWebEngineHistory::QQuickWebEngineHistory(QQuickWebEngineViewPrivate *view)
    : d_ptr(new QQuickWebEngineHistoryPrivate(view))
{
}

QQuickWebEngineHistory::~QQuickWebEngineHistory()
{
}

QQuickWebEngineHistoryListModel *QQuickWebEngineHistory::items() const
{
    Q_D(const QQuickWebEngineHistory);
    if (!d->m_navigationModel)
        d->m_navigationModel.reset(new QQuickWebEngineHistoryListModel(new QQuickWebEngineHistoryListModelPrivate(d->m_view)));
    return d->m_navigationModel.data();
}

QQuickWebEngineHistoryListModel *QQuickWebEngineHistory::backItems() const
{
    Q_D(const QQuickWebEngineHistory);
    if (!d->m_backNavigationModel)
        d->m_backNavigationModel.reset(new QQuickWebEngineHistoryListModel(new QQuickWebEngineBackHistoryListModelPrivate(d->m_view)));
    return d->m_backNavigationModel.data();
}

QQuickWebEngineHistoryListModel *QQuickWebEngineHistory::forwardItems() const
{
    Q_D(const QQuickWebEngineHistory);
    if (!d->m_forwardNavigationModel)
        d->m_forwardNavigationModel.reset(new QQuickWebEngineHistoryListModel(new QQuickWebEngineForwardHistoryListModelPrivate(d->m_view)));
    return d->m_forwardNavigationModel.data();
}

void QQuickWebEngineHistory::reset()
{
    Q_D(QQuickWebEngineHistory);
    if (d->m_navigationModel)
        d->m_navigationModel->reset();
    if (d->m_backNavigationModel)
        d->m_backNavigationModel->reset();
    if (d->m_forwardNavigationModel)
        d->m_forwardNavigationModel->reset();
}


QT_END_NAMESPACE
