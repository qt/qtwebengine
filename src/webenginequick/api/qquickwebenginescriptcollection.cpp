// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebenginescriptcollection_p.h"
#include <QtWebEngineCore/qwebenginescriptcollection.h>
#include <QtWebEngineCore/private/qwebenginescriptcollection_p.h>

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
     \sa webEngineScript WebEngineScriptCollection

*/

QQuickWebEngineScriptCollection::QQuickWebEngineScriptCollection(QWebEngineScriptCollection *collection)
    : d(collection)
{
}

QQuickWebEngineScriptCollection::~QQuickWebEngineScriptCollection() { }

/*!
    \qmlmethod bool WebEngineScriptCollection::contains(webEngineScript script)
    \since QtWebEngine 6.2
    Returns \c true if the specified \a script is in the collection, \c false
    otherwise.
    \sa find()
*/

bool QQuickWebEngineScriptCollection::contains(const QWebEngineScript &value) const
{
    return d->contains(value);
}

/*!
    \qmlmethod list<webEngineScript> WebEngineScriptCollection::find(string name)
    \since QtWebEngine 6.2
    Returns a list of all user script objects with the given \a name.
    \sa contains()
*/
QList<QWebEngineScript> QQuickWebEngineScriptCollection::find(const QString &name) const
{
    return d->find(name);
}

/*!
    \qmlmethod void WebEngineScriptCollection::insert(webEngineScript script)
    \since QtWebEngine 6.2
    Inserts a single \a script into the collection.
    \sa remove()
*/
void QQuickWebEngineScriptCollection::insert(const QWebEngineScript &s)
{
    d->insert(s);
}

/*!
    \qmlmethod void WebEngineScriptCollection::insert(list<webEngineScript> list)
    \since QtWebEngine 6.2
    Inserts a \a list of webEngineScript values into the user script collection.
    \sa remove()
*/
void QQuickWebEngineScriptCollection::insert(const QList<QWebEngineScript> &list)
{
    d->insert(list);
}

/*!
    \qmlmethod bool WebEngineScriptCollection::remove(webEngineScript script)
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
    \qmlproperty list<webEngineScript> WebEngineScriptCollection::collection
    \since QtWebEngine 6.2

    This property holds a QML list of user webEngineScript values. The list can
    take webEngineScript value type or a JavaScript dictionary as values.
*/
QList<QWebEngineScript> QQuickWebEngineScriptCollection::collection() const
{
    return d->toList();
}

void QQuickWebEngineScriptCollection::setCollection(const QList<QWebEngineScript> &scriptList)
{
    if (scriptList != d->toList()) {
        clear();
        insert(scriptList);
        Q_EMIT collectionChanged();
    }
}

#include "moc_qquickwebenginescriptcollection_p.cpp"
