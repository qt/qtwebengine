/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickwebenginescriptcollection_p.h"
#include "qwebenginescriptcollection.h"
#include <QtWebEngineCore/private/qwebenginescriptcollection_p.h>
#include <QtQml/qqmlinfo.h>
#include <QtQml/private/qqmlengine_p.h>
#include <QtQml/private/qv4scopedvalue_p.h>
#include <QtQml/private/qv4arrayobject_p.h>

QWebEngineScript parseScript(const QJSValue &value, bool *ok)
{
    QWebEngineScript s;
    if (ok)
        *ok = false;

    if (value.isObject()) {

        if (value.hasProperty(QStringLiteral("name")))
            s.setName(value.property(QStringLiteral("name")).toString());

        if (value.hasProperty(QStringLiteral("sourceUrl")))
            s.setSourceUrl(value.property(QStringLiteral("sourceUrl")).toString());

        if (value.hasProperty(QStringLiteral("injectionPoint")))
            s.setInjectionPoint(QWebEngineScript::InjectionPoint(
                    value.property(QStringLiteral("injectionPoint")).toUInt()));

        if (value.hasProperty(QStringLiteral("sourceCode")))
            s.setSourceCode(value.property(QStringLiteral("sourceCode")).toString());

        if (value.hasProperty(QStringLiteral("worldId")))
            s.setWorldId(QWebEngineScript::ScriptWorldId(
                    value.property(QStringLiteral("worldId")).toUInt()));

        if (value.hasProperty(QStringLiteral("runOnSubframes")))
            s.setRunsOnSubFrames(value.property(QStringLiteral("runOnSubframes")).toBool());

        if (ok)
            *ok = true;
    }
    return s;
}

QQuickWebEngineScriptCollection::QQuickWebEngineScriptCollection(
        QWebEngineScriptCollection *collection)
    : d(collection)
{
}

QQuickWebEngineScriptCollection::~QQuickWebEngineScriptCollection() { }

bool QQuickWebEngineScriptCollection::contains(const QWebEngineScript &value) const
{
    return d->contains(value);
}

QList<QWebEngineScript> QQuickWebEngineScriptCollection::find(const QString &name) const
{
    return d->find(name);
}

void QQuickWebEngineScriptCollection::insert(const QWebEngineScript &s)
{
    d->insert(s);
}

void QQuickWebEngineScriptCollection::insert(const QList<QWebEngineScript> &list)
{
    d->insert(list);
}

bool QQuickWebEngineScriptCollection::remove(const QWebEngineScript &script)
{
    return d->remove(script);
}

void QQuickWebEngineScriptCollection::clear()
{
    d->clear();
}

QJSValue QQuickWebEngineScriptCollection::collection() const
{
    const QList<QWebEngineScript> &list = d->toList();
    QQmlContext *context = QQmlEngine::contextForObject(this);
    QQmlEngine *engine = context->engine();
    QV4::ExecutionEngine *v4 = QQmlEnginePrivate::getV4Engine(engine);
    QV4::Scope scope(v4);
    QV4::Scoped<QV4::ArrayObject> scriptArray(scope, v4->newArrayObject(list.length()));
    int i = 0;
    for (const auto &val : list) {
        QV4::ScopedValue sv(scope, v4->fromVariant(QVariant::fromValue(val)));
        scriptArray->put(i++, sv);
    }
    return QJSValuePrivate::fromReturnedValue(scriptArray.asReturnedValue());
}

void QQuickWebEngineScriptCollection::setCollection(const QJSValue &scripts)
{
    if (!scripts.isArray())
        return;

    QList<QWebEngineScript> scriptList;
    quint32 length = scripts.property(QStringLiteral("length")).toUInt();
    for (quint32 i = 0; i < length; ++i) {
        bool ok;
        QWebEngineScript s = parseScript(scripts.property(i), &ok);
        if (!ok) {
            qmlWarning(this) << "Unsupported script type";
            return;
        }
        scriptList.append(s);
    }
    if (scriptList != d->toList()) {
        clear();
        insert(scriptList);
        Q_EMIT collectionChanged();
    }
}
