// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebenginescriptcollection_p.h"
#include "qquickwebenginescriptcollection_p_p.h"
#include "qwebenginescriptcollection.h"
#include <QtWebEngineCore/private/qwebenginescriptcollection_p.h>
#include <QtQml/qqmlinfo.h>
#include <QtQml/private/qqmlengine_p.h>
#include <QtQml/private/qv4scopedvalue_p.h>
#include <QtQml/private/qv4arrayobject_p.h>

/*!
    \qmltype WebEngineScriptCollection
    \brief Manages a collection of user scripts.
    \since QtWebEngine 6.2

    \inqmlmodule QtWebEngine

    WebEngineScriptCollection handles a user scripts collection, which
    is injected in the JavaScript engine during the loading of web content.

    Use \l{WebEngineView::userScripts}{WebEgineView.userScripts} and
    \l{WebEngineProfile::userScripts}{WebEngineProfile.userScripts} to access
    the collection of scripts  associated with a single page or number of pages
    sharing the same profile.

    The collection of user script objects in QML can be created for a set of
    user script objects by simple assignment to
    \l{WebEngineScriptCollection::collection}{WebEngineScriptCollection.collection}
    property or by WebEngineScriptCollection methods.

    \note The new user script can be instantiated with JavaScript dictionaries when using
    \e collection property.

    See the following code snippets demonstrating the usage:

    \list
        \li \e collection property with JavaScript dictionaries
        \code
            var scriptFoo = { name: "Foo",
                            sourceUrl: Qt.resolvedUrl("foo.js"),
                            injectionPoint: WebEngineScript.DocumentReady }

            webEngineView.userScripts.collection = [ scriptFoo, scriptBar ];
        \endcode
        \li \e collection property with user script object as value type
         \code
            var script = WebEngine.script()
            script.name = "FOO"
            webEngineView.userScripts.collection = [ script ]
         \endcode
         \li user script collection \e insert method can be used only with value type
            or list of value types
         \code
            var script = WebEngine.script()
            script.name = "FOO"
            webEngineView.userScripts.insert(script)

            var list = [ script ]
            webEngineView.userScripts.insert(list)
         \endcode
     \endlist
     \sa WebEngineScript WebEngineScriptCollection

*/

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

QQuickWebEngineScriptCollectionPrivate::QQuickWebEngineScriptCollectionPrivate(QWebEngineScriptCollectionPrivate *p)
    : QWebEngineScriptCollection(p)
{

}

QQuickWebEngineScriptCollectionPrivate::~QQuickWebEngineScriptCollectionPrivate()
{
}

QQuickWebEngineScriptCollection::QQuickWebEngineScriptCollection(QQuickWebEngineScriptCollectionPrivate *p)
    : d(p)
{
}

QQuickWebEngineScriptCollection::~QQuickWebEngineScriptCollection() { }

/*!
    \qmlmethod void WebEngineScriptCollection::contains(WebEngineScript script)
    \since QtWebEngine 6.2
    Checks if the specified \a script is in the collection.
    \sa find()
*/

bool QQuickWebEngineScriptCollection::contains(const QWebEngineScript &value) const
{
    return d->contains(value);
}

/*!
    \qmlmethod list<WebEngineScript> WebEngineScriptCollection::find(string name)
    \since QtWebEngine 6.2
    Returns a list of all user script objects with the given \a name.
    \sa find()
*/
QList<QWebEngineScript> QQuickWebEngineScriptCollection::find(const QString &name) const
{
    return d->find(name);
}

/*!
    \qmlmethod void WebEngineScriptCollection::insert(WebEngineScript script)
    \since QtWebEngine 6.2
    Inserts a single \a script into the collection.
    \sa find()
*/
void QQuickWebEngineScriptCollection::insert(const QWebEngineScript &s)
{
    d->insert(s);
}

/*!
    \qmlmethod void WebEngineScriptCollection::insert(list<WebEngineScript> list)
    \since QtWebEngine 6.2
    Inserts a \a list of WebEngineScript values into the user script collection.
    \sa find()
*/
void QQuickWebEngineScriptCollection::insert(const QList<QWebEngineScript> &list)
{
    d->insert(list);
}

/*!
    \qmlmethod bool WebEngineScriptCollection::remove(WebEngineScript script)
    \since QtWebEngine 6.2
    Returns \c true if a given \a script is removed from the collection.
    \sa insert()
*/
bool QQuickWebEngineScriptCollection::remove(const QWebEngineScript &script)
{
    return d->remove(script);
}

/*!
    \qmlmethod void WebEngineScriptCollection::clear()
    \since QtWebEngine 6.2
    Removes all script objects from this collection.
*/
void QQuickWebEngineScriptCollection::clear()
{
    d->clear();
}

/*!
    \qmlproperty list<WebEngineScript> WebEngineScriptCollection::collection
    \since QtWebEngine 6.2

    This property holds a JavaScript array of user script objects. The array can
    take WebEngineScript basic type or a JavaScript dictionary as values.
*/
QJSValue QQuickWebEngineScriptCollection::collection() const
{
    if (!d->m_qmlEngine) {
        qmlWarning(this) << "Scripts collection doesn't have QML engine set! Undefined value is returned.";
        return QJSValue();
    }

    const QList<QWebEngineScript> &list = d->toList();
    QV4::ExecutionEngine *v4 = QQmlEnginePrivate::getV4Engine(d->m_qmlEngine);
    QV4::Scope scope(v4);
    QV4::Scoped<QV4::ArrayObject> scriptArray(scope, v4->newArrayObject(list.size()));
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

QQmlEngine* QQuickWebEngineScriptCollection::qmlEngine()
{
    return d->m_qmlEngine;
}

void QQuickWebEngineScriptCollection::setQmlEngine(QQmlEngine *engine)
{
    Q_ASSERT(engine);
    d->m_qmlEngine = engine;
}
#include "moc_qquickwebenginescriptcollection_p.cpp"
