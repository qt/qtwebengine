// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginescriptcollection.h"
#include "qwebenginescriptcollection_p.h"

#include "renderer_host/user_resource_controller_host.h"

/*!
    \class QWebEngineScriptCollection
    \inmodule QtWebEngineCore
    \since 5.5
    \brief The QWebEngineScriptCollection class represents a collection of user scripts.

    QWebEngineScriptCollection manages a set of user scripts.

    Use QWebEnginePage::scripts() and QWebEngineProfile::scripts() to access
    the collection of scripts associated with a single page or a
    number of pages sharing the same profile.

    \sa {Script Injection}
*/

/*!
    \fn QWebEngineScriptCollection::isEmpty() const

    Returns \c true if the collection is empty; otherwise returns \c false.
*/

QWebEngineScriptCollection::QWebEngineScriptCollection(QWebEngineScriptCollectionPrivate *collectionPrivate)
    :d(collectionPrivate)
{
}

/*!
    Destroys the collection.
*/
QWebEngineScriptCollection::~QWebEngineScriptCollection()
{
}

/*!
    Returns the number of elements in the collection.
 */

int QWebEngineScriptCollection::count() const
{
    return d->count();
}

/*!
    Returns \c true if the collection contains an occurrence of \a value; otherwise returns
    \c false.
 */

bool QWebEngineScriptCollection::contains(const QWebEngineScript &value) const
{
    return d->contains(value);
}

/*!
    Returns the list of scripts in the collection with the name \a name, or an empty list if none
    was found.
 */

QList<QWebEngineScript> QWebEngineScriptCollection::find(const QString &name) const
{
    return d->toList(name);
}
/*!
    Inserts the script \a s into the collection.
 */
void QWebEngineScriptCollection::insert(const QWebEngineScript &s)
{
    d->insert(s);
}
/*!
    Inserts scripts from the list \a list into the collection.
 */
void QWebEngineScriptCollection::insert(const QList<QWebEngineScript> &list)
{
    d->reserve(list.size());
    for (const QWebEngineScript &s : list)
        d->insert(s);
}

/*!
    Removes \a script from the collection.

    Returns \c true if the script was found and successfully removed from the collection; otherwise
    returns \c false.
 */
bool QWebEngineScriptCollection::remove(const QWebEngineScript &script)
{
    return d->remove(script);
}

/*!
 * Removes all scripts from this collection.
 */
void QWebEngineScriptCollection::clear()
{
    d->clear();
}

/*!
    Returns a list with the values of the scripts used in this collection.
 */
QList<QWebEngineScript> QWebEngineScriptCollection::toList() const
{
    return d->toList();
}


QWebEngineScriptCollectionPrivate::QWebEngineScriptCollectionPrivate(QtWebEngineCore::UserResourceControllerHost *controller, QSharedPointer<QtWebEngineCore::WebContentsAdapter> webContents)
    : m_scriptController(controller)
    , m_contents(webContents)
{
}

int QWebEngineScriptCollectionPrivate::count() const
{
    return m_scripts.size();
}

bool QWebEngineScriptCollectionPrivate::contains(const QWebEngineScript &s) const
{
    return m_scripts.contains(s);
}

void QWebEngineScriptCollectionPrivate::insert(const QWebEngineScript &script)
{
    m_scripts.append(script);
    if (!m_contents || m_contents->isInitialized())
        m_scriptController->addUserScript(*script.d, m_contents.data());
}

bool QWebEngineScriptCollectionPrivate::remove(const QWebEngineScript &script)
{
    if (!m_contents || m_contents->isInitialized())
        m_scriptController->removeUserScript(*script.d, m_contents.data());
    return m_scripts.removeAll(script);
}

QList<QWebEngineScript> QWebEngineScriptCollectionPrivate::toList(const QString &scriptName) const
{
    if (scriptName.isNull())
        return m_scripts;

    QList<QWebEngineScript> ret;
    for (const QWebEngineScript &script : std::as_const(m_scripts))
        if (scriptName == script.name())
            ret.append(script);
    return ret;
}

void QWebEngineScriptCollectionPrivate::clear()
{
    m_scripts.clear();
    if (!m_contents || m_contents->isInitialized())
        m_scriptController->clearAllScripts(m_contents.data());
}

void QWebEngineScriptCollectionPrivate::reserve(int capacity)
{
    m_scripts.reserve(capacity);
    if (!m_contents || m_contents->isInitialized())
        m_scriptController->reserve(m_contents.data(), capacity);
}

void QWebEngineScriptCollectionPrivate::initializationFinished(QSharedPointer<QtWebEngineCore::WebContentsAdapter> contents)
{
    Q_ASSERT(m_contents);
    Q_ASSERT(contents);

    for (const QWebEngineScript &script : std::as_const(m_scripts))
        m_scriptController->addUserScript(*script.d, contents.data());
    m_contents = contents;
}
