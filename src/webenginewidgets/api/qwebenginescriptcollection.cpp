/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebenginescriptcollection.h"
#include "qwebenginescriptcollection_p.h"

#include "user_script_controller_host.h"

using QtWebEngineCore::UserScript;

/*!
    \class QWebEngineScriptCollection
    \inmodule QtWebEngineWidgets
    \since 5.5
    \brief The QWebEngineScriptCollection class represents a collection of user scripts.

*/

QWebEngineScriptCollection::QWebEngineScriptCollection(QWebEngineScriptCollectionPrivate *collectionPrivate)
    :d(collectionPrivate)
{
}

QWebEngineScriptCollection::~QWebEngineScriptCollection()
{
}

/*!
 * \brief QWebEngineScriptCollection::count
 * \return the number of elements in the collection.
 */

int QWebEngineScriptCollection::count() const
{
    return d->count();
}

/*!
 * \brief QWebEngineScriptCollection::contains
 * \param value
 * \return \c true if the collection contains an occurrence of \a value; otherwise returns false.
 */

bool QWebEngineScriptCollection::contains(const QWebEngineScript &value) const
{
    return d->contains(value);
}

/*!
 * \brief QWebEngineScriptCollection::findScript
 * \param name
 * \return the first script found in collection the name property of which is \a name, or a null QWebEngineScript if none was found.
 * \note the order in which the script collection is traversed is undefined, which means this should be used when the unicity is
 * guaranteed at the application level.
 * \sa findScripts()
 */

QWebEngineScript QWebEngineScriptCollection::findScript(const QString &name) const
{
    return d->find(name);
}

/*!
 * \brief QWebEngineScriptCollection::findScripts
 * \param name
 * \return the list of scripts in the collection the name property of which is \a name, or an empty list if none was found.
 */

QList<QWebEngineScript> QWebEngineScriptCollection::findScripts(const QString &name) const
{
    return d->toList(name);
}
/*!
 * \brief QWebEngineScriptCollection::insert
 * \param s
 *
 * Inserts script \c s into the collection.
 */
void QWebEngineScriptCollection::insert(const QWebEngineScript &s)
{
    d->insert(s);
}
/*!
 * \brief QWebEngineScriptCollection::insert
 * \param list
 *
 * Inserts scripts \c list into the collection.
 */
void QWebEngineScriptCollection::insert(const QList<QWebEngineScript> &list)
{
    d->reserve(list.size());
    Q_FOREACH (const QWebEngineScript &s, list)
        d->insert(s);
}

/*!
 * \brief QWebEngineScriptCollection::remove
 * \param script
 * Removes \a script from the collection, if it is present.
 * \return \c true if the script was found and successfully removed from the collection, \c false otherwise.
 */
bool QWebEngineScriptCollection::remove(const QWebEngineScript &script)
{
    return d->remove(script);
}

/*!
 * \brief QWebEngineScriptCollection::clear
 * Removes all scripts from this collection.
 */
void QWebEngineScriptCollection::clear()
{
    d->clear();
}

/*!
 * \brief QWebEngineScriptCollection::toList
 * \return a QList with the values of the scripts used in this collection.
 */
QList<QWebEngineScript> QWebEngineScriptCollection::toList() const
{
    return d->toList();
}


QWebEngineScriptCollectionPrivate::QWebEngineScriptCollectionPrivate(QtWebEngineCore::UserScriptControllerHost *controller, QtWebEngineCore::WebContentsAdapter *webContents)
    : m_scriptController(controller)
    , m_contents(webContents)
{
}

int QWebEngineScriptCollectionPrivate::count() const
{
    return m_scriptController->registeredScripts(m_contents).count();
}

bool QWebEngineScriptCollectionPrivate::contains(const QWebEngineScript &s) const
{
    return m_scriptController->containsUserScript(*s.d, m_contents);
}

void QWebEngineScriptCollectionPrivate::insert(const QWebEngineScript &script)
{
    if (!script.d)
        return;
    m_scriptController->addUserScript(*script.d, m_contents);
}

bool QWebEngineScriptCollectionPrivate::remove(const QWebEngineScript &script)
{
    if (!script.d)
        return false;
    return m_scriptController->removeUserScript(*script.d, m_contents);
}

QList<QWebEngineScript> QWebEngineScriptCollectionPrivate::toList(const QString &scriptName) const
{
    QList<QWebEngineScript> ret;
    Q_FOREACH (const UserScript &script, m_scriptController->registeredScripts(m_contents))
        if (scriptName.isNull() || scriptName == script.name())
            ret.append(QWebEngineScript(script));
    return ret;
}

QWebEngineScript QWebEngineScriptCollectionPrivate::find(const QString &name) const
{
    Q_FOREACH (const UserScript &script, m_scriptController->registeredScripts(m_contents))
        if (name == script.name())
            return QWebEngineScript(script);
    return QWebEngineScript();
}

void QWebEngineScriptCollectionPrivate::clear()
{
    m_scriptController->clearAllScripts(m_contents);
}

void QWebEngineScriptCollectionPrivate::reserve(int capacity)
{
    m_scriptController->reserve(m_contents, capacity);
}
