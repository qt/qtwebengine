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

#ifndef QWEBENGINEHISTORY_H
#define QWEBENGINEHISTORY_H

#include <QtCore/qurl.h>
#include <QtCore/qstring.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qshareddata.h>
#include <QtGui/qicon.h>
#include <QtWebEngineWidgets/qtwebenginewidgetsglobal.h>

QT_BEGIN_NAMESPACE

class QWebEngineHistory;
class QWebEngineHistoryItemPrivate;
class QWebEnginePage;
class QWebEnginePagePrivate;
class WebEngineHistory;
class WebEngineHistoryItem;

class QWEBENGINEWIDGETS_EXPORT QWebEngineHistoryItem {
public:
    QWebEngineHistoryItem(const QWebEngineHistoryItem &other);
    QWebEngineHistoryItem &operator=(const QWebEngineHistoryItem &other);
    ~QWebEngineHistoryItem();

    bool isValid() const;

    QUrl originalUrl() const;
    QUrl url() const;

    QString title() const;
    QDateTime lastVisited() const;

    QIcon icon() const;

    QVariant userData() const;
    void setUserData(const QVariant& userData);

private:
    QWebEngineHistoryItem();
    QWebEngineHistoryItem(QWebEngineHistoryItemPrivate*);

    Q_DECLARE_PRIVATE_D(d.data(), QWebEngineHistoryItem);
    QExplicitlySharedDataPointer<QWebEngineHistoryItemPrivate> d;

    friend class QWebEngineHistory;
};


class QWEBENGINEWIDGETS_EXPORT QWebEngineHistory {
public:
    void clear();

    QList<QWebEngineHistoryItem> items() const;
    QList<QWebEngineHistoryItem> backItems(int maxItems) const;
    QList<QWebEngineHistoryItem> forwardItems(int maxItems) const;

    bool canGoBack() const;
    bool canGoForward() const;

    void back();
    void forward();
    void goToItem(const QWebEngineHistoryItem item);

    QWebEngineHistoryItem backItem() const;
    QWebEngineHistoryItem currentItem() const;
    QWebEngineHistoryItem forwardItem() const;
    QWebEngineHistoryItem itemAt(int i) const;

    int currentItemIndex() const;

    int count() const;

    int maximumItemCount() const;
    void setMaximumItemCount(int count);

private:
    QWebEngineHistory(WebEngineHistory*);
    ~QWebEngineHistory();

    Q_DISABLE_COPY(QWebEngineHistory)
    QExplicitlySharedDataPointer<WebEngineHistory> m_history;

    friend QWEBENGINEWIDGETS_EXPORT QDataStream& operator>>(QDataStream&, QWebEngineHistory&);
    friend QWEBENGINEWIDGETS_EXPORT QDataStream& operator<<(QDataStream&, const QWebEngineHistory&);
    friend class QWebEnginePage;
    friend class QWebEnginePagePrivate;
};

QWEBENGINEWIDGETS_EXPORT QDataStream& operator<<(QDataStream& stream, const QWebEngineHistory& history);
QWEBENGINEWIDGETS_EXPORT QDataStream& operator>>(QDataStream& stream, QWebEngineHistory& history);

QT_END_NAMESPACE

#endif // QWEBENGINEHISTORY_H
