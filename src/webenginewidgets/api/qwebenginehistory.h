/*
    Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

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

class QWEBENGINEWIDGETS_EXPORT QWebEngineHistoryItem {
public:
    QWebEngineHistoryItem(const QWebEngineHistoryItem &other);
    QWebEngineHistoryItem &operator=(const QWebEngineHistoryItem &other);
    ~QWebEngineHistoryItem();

    QUrl originalUrl() const;
    QUrl url() const;

    QString title() const;
    QDateTime lastVisited() const;

    bool isValid() const;
private:
    QWebEngineHistoryItem(QWebEngineHistoryItemPrivate *priv);
    Q_DECLARE_PRIVATE_D(d.data(), QWebEngineHistoryItem);
    QExplicitlySharedDataPointer<QWebEngineHistoryItemPrivate> d;
    friend class QWebEngineHistory;
    friend class QWebEngineHistoryPrivate;
};


class QWebEngineHistoryPrivate;
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
    void goToItem(const QWebEngineHistoryItem &item);

    QWebEngineHistoryItem backItem() const;
    QWebEngineHistoryItem currentItem() const;
    QWebEngineHistoryItem forwardItem() const;
    QWebEngineHistoryItem itemAt(int i) const;

    int currentItemIndex() const;

    int count() const;

private:
    QWebEngineHistory(QWebEngineHistoryPrivate *d);
    ~QWebEngineHistory();

    Q_DISABLE_COPY(QWebEngineHistory)
    Q_DECLARE_PRIVATE(QWebEngineHistory);
    QScopedPointer<QWebEngineHistoryPrivate> d_ptr;

    friend QWEBENGINEWIDGETS_EXPORT QDataStream& operator>>(QDataStream&, QWebEngineHistory&);
    friend QWEBENGINEWIDGETS_EXPORT QDataStream& operator<<(QDataStream&, const QWebEngineHistory&);
    friend class QWebEnginePage;
    friend class QWebEnginePagePrivate;
};

QWEBENGINEWIDGETS_EXPORT QDataStream& operator<<(QDataStream& stream, const QWebEngineHistory& history);
QWEBENGINEWIDGETS_EXPORT QDataStream& operator>>(QDataStream& stream, QWebEngineHistory& history);

QT_END_NAMESPACE

#endif // QWEBENGINEHISTORY_H
