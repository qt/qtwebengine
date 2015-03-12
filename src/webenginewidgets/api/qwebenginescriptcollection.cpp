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

QWebEngineScriptCollection::QWebEngineScriptCollection(QWebEngineScriptCollectionPrivate *collectionPrivate)
    :d(collectionPrivate)
{
}

QWebEngineScriptCollection::~QWebEngineScriptCollection()
{
}

int QWebEngineScriptCollection::count() const
{
    return d->count();
}

bool QWebEngineScriptCollection::contains(const QWebEngineScript &value) const
{
    return d->contains(value);
}

QWebEngineScript QWebEngineScriptCollection::findScript(const QString &name) const
{
    return d->find(name);
}

QList<QWebEngineScript> QWebEngineScriptCollection::findScripts(const QString &name) const
{
    return d->toList(name);
}

void QWebEngineScriptCollection::insert(const QWebEngineScript &s)
{
    d->insert(s);
}

void QWebEngineScriptCollection::insert(const QList<QWebEngineScript> &list)
{
    d->reserve(list.size());
    Q_FOREACH (const QWebEngineScript &s, list)
        d->insert(s);
}

bool QWebEngineScriptCollection::remove(const QWebEngineScript &script)
{
    return d->remove(script);
}

void QWebEngineScriptCollection::clear()
{
    d->clear();
}

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
