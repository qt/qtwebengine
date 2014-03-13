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
#ifndef QQUICKWEBENGINEHISOTRY_P_H
#define QQUICKWEBENGINEHISTORY_P_H

#include <qtwebengineglobal_p.h>
#include <QAbstractListModel>
#include <QQuickItem>
#include <QUrl>
#include <QVariant>

#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class WebEngineHistory;
class WebEngineHistoryItem;
class QQuickWebEngineHistory;
class QQuickWebEngineHistoryListModelPrivate;
class QQuickWebEngineLoadRequest;

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineHistoryListModel : public QAbstractListModel {
    Q_OBJECT

public:
    QQuickWebEngineHistoryListModel(QQuickWebEngineHistoryListModelPrivate*);
    virtual ~QQuickWebEngineHistoryListModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;
    QHash<int, QByteArray> roleNames() const;
    void reset();

private:
    QQuickWebEngineHistoryListModel();
    WebEngineHistoryItem itemAt(int) const;

    Q_DECLARE_PRIVATE(QQuickWebEngineHistoryListModel);
    QScopedPointer<QQuickWebEngineHistoryListModelPrivate> d_ptr;

    friend class QQuickWebEngineHistory;
};

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineHistory : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QQuickWebEngineHistoryListModel *backItems READ backItems CONSTANT FINAL)
    Q_PROPERTY(QQuickWebEngineHistoryListModel *forwardItems READ forwardItems CONSTANT FINAL)
    Q_PROPERTY(QQuickWebEngineHistoryListModel *items READ backForwardItems CONSTANT FINAL)
    Q_PROPERTY(int currentItemIndex READ currentItemIndex CONSTANT FINAL)

public:
    QQuickWebEngineHistory(WebEngineHistory*);
    virtual ~QQuickWebEngineHistory();

    enum NavigationHistoryRoles {
        UrlRole = Qt::UserRole + 1,
        TitleRole = Qt::UserRole + 2
    };

    QQuickWebEngineHistoryListModel *backItems() const;
    QQuickWebEngineHistoryListModel *forwardItems() const;
    QQuickWebEngineHistoryListModel *backForwardItems() const;

    bool canGoBack() const;
    bool canGoForward() const;

    void goBackTo(int);
    void goForwardTo(int);

    void goBack();
    void goForward();

    int currentItemIndex();

public Q_SLOTS:
    void onLoadingChanged(QQuickWebEngineLoadRequest *loadRequest);

private:
    QQuickWebEngineHistory();

    QExplicitlySharedDataPointer<WebEngineHistory> m_history;

    QScopedPointer<QQuickWebEngineHistoryListModel> m_backItems;
    QScopedPointer<QQuickWebEngineHistoryListModel> m_forwardItems;
    QScopedPointer<QQuickWebEngineHistoryListModel> m_backForwardItems;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickWebEngineHistory)

#endif // QQUICKWEBENGINEHISTORY_P_H
