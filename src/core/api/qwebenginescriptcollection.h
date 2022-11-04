// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINESCRIPTCOLLECTION_H
#define QWEBENGINESCRIPTCOLLECTION_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtWebEngineCore/qwebenginescript.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE
class QWebEngineScriptCollectionPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineScriptCollection
{
public:
    ~QWebEngineScriptCollection();

    bool isEmpty() const { return !count(); }
    int count() const;
    bool contains(const QWebEngineScript &value) const;
    QList<QWebEngineScript> find(const QString &name) const;
    void insert(const QWebEngineScript &);
    void insert(const QList<QWebEngineScript> &list);
    bool remove(const QWebEngineScript &);
    void clear();

    QList<QWebEngineScript> toList() const;

private:
    Q_DISABLE_COPY(QWebEngineScriptCollection)
    friend class QWebEnginePagePrivate;
    friend class QWebEngineProfilePrivate;
    friend class QQuickWebEngineProfilePrivate;
    friend class QQuickWebEngineViewPrivate;
    friend class QQuickWebEngineScriptCollectionPrivate;
    QWebEngineScriptCollection(QWebEngineScriptCollectionPrivate *);

    QScopedPointer<QWebEngineScriptCollectionPrivate> d;
};

QT_END_NAMESPACE
#endif // QWEBENGINESCRIPTCOLLECTION_H
