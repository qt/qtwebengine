// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINESCRIPTCOLLECTION_H
#define QQUICKWEBENGINESCRIPTCOLLECTION_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtQml/qjsvalue.h>
#include <QtWebEngineCore/qwebenginescript.h>
#include <QtWebEngineQuick/private/qtwebenginequickglobal_p.h>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QQuickWebEngineScriptCollectionPrivate;

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineScriptCollection : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QJSValue collection READ collection WRITE setCollection NOTIFY collectionChanged)
    ~QQuickWebEngineScriptCollection();

    Q_INVOKABLE bool contains(const QWebEngineScript &value) const;
    Q_INVOKABLE QList<QWebEngineScript> find(const QString &name) const;
    Q_INVOKABLE void insert(const QWebEngineScript &);
    Q_INVOKABLE void insert(const QList<QWebEngineScript> &list);
    Q_INVOKABLE bool remove(const QWebEngineScript &);
    Q_INVOKABLE void clear();

    QJSValue collection() const;
    void setCollection(const QJSValue &scripts);

Q_SIGNALS:
    void collectionChanged();

private:
    Q_DISABLE_COPY(QQuickWebEngineScriptCollection)
    QQuickWebEngineScriptCollection(QQuickWebEngineScriptCollectionPrivate *d);
    QScopedPointer<QQuickWebEngineScriptCollectionPrivate> d;
    friend class QQuickWebEngineProfilePrivate;
    friend class QQuickWebEngineViewPrivate;
    QQmlEngine* qmlEngine();
    void setQmlEngine(QQmlEngine *engine);
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QQuickWebEngineScriptCollection *)

#endif // QWEBENGINESCRIPTCOLLECTION_H
