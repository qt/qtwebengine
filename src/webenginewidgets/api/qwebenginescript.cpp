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

#include "qwebenginescript.h"

#include "user_script.h"
#include <QtCore/QDebug>

QWebEngineScript::QWebEngineScript()
    : d(new UserScript)
{
}

QWebEngineScript::QWebEngineScript(const QWebEngineScript &other)
    : d(other.d)
{
}

QWebEngineScript::~QWebEngineScript()
{
}

QWebEngineScript &QWebEngineScript::operator=(const QWebEngineScript &other)
{
    d = other.d;
    return *this;
}

bool QWebEngineScript::isNull() const
{
    return d->isNull();
}

QString QWebEngineScript::name() const
{
    return d->name();
}

void QWebEngineScript::setName(const QString &scriptName)
{
    if (scriptName == name())
        return;
    d->setName(scriptName);
}

QString QWebEngineScript::source() const
{
    return d->source();
}

void QWebEngineScript::setSource(const QString &scriptSource)
{
    if (scriptSource == source())
        return;
    d->setSource(scriptSource);
}

ASSERT_ENUMS_MATCH(QWebEngineScript::Deferred, UserScript::AfterLoad)
ASSERT_ENUMS_MATCH(QWebEngineScript::DocumentReady, UserScript::DocumentLoadFinished)
ASSERT_ENUMS_MATCH(QWebEngineScript::DocumentCreation, UserScript::DocumentElementCreation)

QWebEngineScript::InjectionPoint QWebEngineScript::injectionPoint() const
{
    return static_cast<QWebEngineScript::InjectionPoint>(d->injectionPoint());
}

void QWebEngineScript::setInjectionPoint(QWebEngineScript::InjectionPoint p)
{
    if (p == injectionPoint())
        return;
    d->setInjectionPoint(static_cast<UserScript::InjectionPoint>(p));
}

quint32 QWebEngineScript::worldId() const
{
    return d->worldId();
}

void QWebEngineScript::setWorldId(quint32 id)
{
    if (id == d->worldId())
        return;
    d->setWorldId(id);
}

bool QWebEngineScript::runsOnSubFrames() const
{
    return d->runsOnSubFrames();
}

void QWebEngineScript::setRunsOnSubFrames(bool on)
{
    if (runsOnSubFrames() == on)
        return;
    d->setRunsOnSubFrames(on);
}

bool QWebEngineScript::operator==(const QWebEngineScript &other) const
{
    return d == other.d || *d == *(other.d);
}

QWebEngineScript::QWebEngineScript(const UserScript &coreScript)
    : d(new UserScript(coreScript))
{
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QWebEngineScript &script)
{
    if (script.isNull())
        return d.maybeSpace() << "QWebEngineScript()";

    d.nospace() << "QWebEngineScript(" << script.name() << ", ";
    switch (script.injectionPoint()) {
    case QWebEngineScript::DocumentCreation:
        d << "QWebEngineScript::DocumentCreation" << ", ";
        break;
    case QWebEngineScript::DocumentReady:
        d << "QWebEngineScript::DocumentReady" << ", ";
        break;
    case QWebEngineScript::Deferred:
        d << "QWebEngineScript::Deferred" << ", ";
        break;
    }
    d << script.worldId() << ", "
      << script.runsOnSubFrames() << ", " << script.source() << ")";
    return d.space();
}
#endif
