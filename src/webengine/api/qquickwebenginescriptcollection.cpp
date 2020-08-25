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

#include "qquickwebenginescriptcollection.h"
#include "qquickwebenginescriptcollection_p.h"
#include "renderer_host/user_resource_controller_host.h"
#include <QtQml/QQmlInfo>
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
        QQuickWebEngineScriptCollectionPrivate *collectionPrivate)
    : d(collectionPrivate)
{
}

QQuickWebEngineScriptCollection::~QQuickWebEngineScriptCollection() { }

int QQuickWebEngineScriptCollection::count() const
{
    return d->count();
}

bool QQuickWebEngineScriptCollection::contains(const QWebEngineScript &value) const
{
    return d->contains(value);
}

QWebEngineScript QQuickWebEngineScriptCollection::findScript(const QString &name) const
{
    return d->find(name);
}

QList<QWebEngineScript> QQuickWebEngineScriptCollection::findScripts(const QString &name) const
{
    return d->toList(name);
}

void QQuickWebEngineScriptCollection::insert(const QWebEngineScript &s)
{
    d->insert(s);
}

void QQuickWebEngineScriptCollection::insert(const QList<QWebEngineScript> &list)
{
    d->reserve(list.size());
    for (const QWebEngineScript &s : list)
        d->insert(s);
}

bool QQuickWebEngineScriptCollection::remove(const QWebEngineScript &script)
{
    return d->remove(script);
}

void QQuickWebEngineScriptCollection::clear()
{
    d->clear();
}

QList<QWebEngineScript> QQuickWebEngineScriptCollection::toList() const
{
    return d->toList();
}

QQuickWebEngineScriptCollectionPrivate::QQuickWebEngineScriptCollectionPrivate(
        QtWebEngineCore::UserResourceControllerHost *controller,
        QSharedPointer<QtWebEngineCore::WebContentsAdapter> webContents)
    : m_scriptController(controller), m_contents(webContents)
{
}

int QQuickWebEngineScriptCollectionPrivate::count() const
{
    return m_scripts.count();
}

bool QQuickWebEngineScriptCollectionPrivate::contains(const QWebEngineScript &s) const
{
    return m_scripts.contains(s);
}

void QQuickWebEngineScriptCollectionPrivate::insert(const QWebEngineScript &script)
{
    m_scripts.append(script);
    if (!m_contents || m_contents->isInitialized())
        m_scriptController->addUserScript(*script.d, m_contents.data());
}

bool QQuickWebEngineScriptCollectionPrivate::remove(const QWebEngineScript &script)
{
    if (!m_contents || m_contents->isInitialized())
        m_scriptController->removeUserScript(*script.d, m_contents.data());
    return m_scripts.removeAll(script);
}

QList<QWebEngineScript>
QQuickWebEngineScriptCollectionPrivate::toList(const QString &scriptName) const
{
    QList<QWebEngineScript> ret;
    for (const QWebEngineScript &script : qAsConst(m_scripts))
        if (scriptName == script.name())
            ret.append(script);
    return ret;
}

QWebEngineScript QQuickWebEngineScriptCollectionPrivate::find(const QString &name) const
{
    for (const QWebEngineScript &script : qAsConst(m_scripts))
        if (name == script.name())
            return script;
    return QWebEngineScript();
}

void QQuickWebEngineScriptCollectionPrivate::clear()
{
    m_scripts.clear();
    if (!m_contents || m_contents->isInitialized())
        m_scriptController->clearAllScripts(m_contents.data());
}

void QQuickWebEngineScriptCollectionPrivate::reserve(int capacity)
{
    m_scripts.reserve(capacity);
    if (!m_contents || m_contents->isInitialized())
        m_scriptController->reserve(m_contents.data(), capacity);
}

void QQuickWebEngineScriptCollectionPrivate::initializationFinished(
        QSharedPointer<QtWebEngineCore::WebContentsAdapter> contents)
{
    Q_ASSERT(m_contents);
    Q_ASSERT(contents);

    for (const QWebEngineScript &script : qAsConst(m_scripts))
        m_scriptController->addUserScript(*script.d, contents.data());
    m_contents = contents;
}

QJSValue QQuickWebEngineScriptCollection::collection() const
{
    const QList<QWebEngineScript> &list = d->m_scripts;
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
    if (scriptList != d->m_scripts) {
        clear();
        insert(scriptList);
        Q_EMIT collectionChanged();
    }
}
