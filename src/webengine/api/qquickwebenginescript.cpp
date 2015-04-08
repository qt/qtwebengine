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

#include "qquickwebenginescript_p.h"
#include "qquickwebenginescript_p_p.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QStringBuilder>
#include <QtCore/QTimerEvent>
#include "user_script_controller_host.h"

using QtWebEngineCore::UserScript;

QQuickWebEngineScript::QQuickWebEngineScript()
    : d_ptr(new QQuickWebEngineScriptPrivate)
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineScript::~QQuickWebEngineScript()
{
}

QString QQuickWebEngineScript::toString() const
{
    Q_D(const QQuickWebEngineScript);
    if (d->coreScript.isNull())
        return QStringLiteral("QWebEngineScript()");
    QString ret = QStringLiteral("QWebEngineScript(") % d->coreScript.name() % QStringLiteral(", ");
    switch (d->coreScript.injectionPoint()) {
    case UserScript::DocumentElementCreation:
        ret.append(QStringLiteral("WebEngineScript::DocumentCreation, "));
        break;
    case UserScript::DocumentLoadFinished:
        ret.append(QStringLiteral("WebEngineScript::DocumentReady, "));
        break;
    case UserScript::AfterLoad:
        ret.append(QStringLiteral("WebEngineScript::Deferred, "));
        break;
    }
    ret.append(QString::number(d->coreScript.worldId()) % QStringLiteral(", ")
               % (d->coreScript.runsOnSubFrames() ? QStringLiteral("true") : QStringLiteral("false"))
               % QStringLiteral(", ") % d->coreScript.sourceCode() % QLatin1Char(')'));
    return ret;
}

QString QQuickWebEngineScript::name() const
{
    Q_D(const QQuickWebEngineScript);
    return d->coreScript.name();
}

/*!
    \qmlproperty url WebEngineScript::sourceUrl

    This property holds the remote source location of the user script (if any).

    Unlike \l sourceCode, this property allows referring to user scripts that
    are not already loaded in memory, for instance,  when stored on disk.

    Setting this property will change the \l sourceCode of the script.

    \note At present, only file-based sources are supported.

    \sa sourceCode
*/
QUrl QQuickWebEngineScript::sourceUrl() const
{
    Q_D(const QQuickWebEngineScript);
    return d->m_sourceUrl;
}

/*!
    \qmlproperty string WebEngineScript::sourceCode

    This property holds the JavaScript source code of the user script.

    \sa sourceUrl
*/
QString QQuickWebEngineScript::sourceCode() const
{
    Q_D(const QQuickWebEngineScript);
    return d->coreScript.sourceCode();
}

ASSERT_ENUMS_MATCH(QQuickWebEngineScript::Deferred, UserScript::AfterLoad)
ASSERT_ENUMS_MATCH(QQuickWebEngineScript::DocumentReady, UserScript::DocumentLoadFinished)
ASSERT_ENUMS_MATCH(QQuickWebEngineScript::DocumentCreation, UserScript::DocumentElementCreation)

QQuickWebEngineScript::InjectionPoint QQuickWebEngineScript::injectionPoint() const
{
    Q_D(const QQuickWebEngineScript);
    return static_cast<QQuickWebEngineScript::InjectionPoint>(d->coreScript.injectionPoint());
}


QQuickWebEngineScript::ScriptWorldId QQuickWebEngineScript::worldId() const
{
    Q_D(const QQuickWebEngineScript);
    return static_cast<QQuickWebEngineScript::ScriptWorldId>(d->coreScript.worldId());
}


bool QQuickWebEngineScript::runOnSubframes() const
{
    Q_D(const QQuickWebEngineScript);
    return d->coreScript.runsOnSubFrames();
}


void QQuickWebEngineScript::setName(QString arg)
{
    Q_D(QQuickWebEngineScript);
    if (arg == name())
        return;
    d->aboutToUpdateUnderlyingScript();
    d->coreScript.setName(arg);
    Q_EMIT nameChanged(arg);
}

void QQuickWebEngineScript::setSourceCode(QString arg)
{
    Q_D(QQuickWebEngineScript);
    if (arg == sourceCode())
        return;

    // setting the source directly resets the sourceUrl
    if (d->m_sourceUrl != QUrl()) {
        d->m_sourceUrl = QUrl();
        Q_EMIT sourceUrlChanged(d->m_sourceUrl);
    }

    d->aboutToUpdateUnderlyingScript();
    d->coreScript.setSourceCode(arg);
    Q_EMIT sourceCodeChanged(arg);
}

void QQuickWebEngineScript::setSourceUrl(QUrl arg)
{
    Q_D(QQuickWebEngineScript);
    if (arg == sourceUrl())
        return;

    d->m_sourceUrl = arg;
    Q_EMIT sourceUrlChanged(d->m_sourceUrl);

    QFile f(arg.toLocalFile());
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Can't open user script " << arg;
        return;
    }

    d->aboutToUpdateUnderlyingScript();
    QString source = QString::fromUtf8(f.readAll());
    d->coreScript.setSourceCode(source);
    Q_EMIT sourceCodeChanged(source);
}

void QQuickWebEngineScript::setInjectionPoint(QQuickWebEngineScript::InjectionPoint arg)
{
    Q_D(QQuickWebEngineScript);
    if (arg == injectionPoint())
        return;
    d->aboutToUpdateUnderlyingScript();
    d->coreScript.setInjectionPoint(static_cast<UserScript::InjectionPoint>(arg));
    Q_EMIT injectionPointChanged(arg);
}


void QQuickWebEngineScript::setWorldId(QQuickWebEngineScript::ScriptWorldId arg)
{
    Q_D(QQuickWebEngineScript);
    if (arg == worldId())
        return;
    d->aboutToUpdateUnderlyingScript();
    d->coreScript.setWorldId(arg);
    Q_EMIT worldIdChanged(arg);
}


void QQuickWebEngineScript::setRunOnSubframes(bool arg)
{
    Q_D(QQuickWebEngineScript);
    if (arg == runOnSubframes())
        return;
    d->aboutToUpdateUnderlyingScript();
    d->coreScript.setRunsOnSubFrames(arg);
    Q_EMIT runOnSubframesChanged(arg);
}

void QQuickWebEngineScript::timerEvent(QTimerEvent *e)
{
    Q_D(QQuickWebEngineScript);
    if (e->timerId() != d->m_basicTimer.timerId()) {
        QObject::timerEvent(e);
        return;
    }
    if (!d->m_controllerHost)
        return;
    d->m_basicTimer.stop();
    d->m_controllerHost->addUserScript(d->coreScript, d->m_adapter);
}

void QQuickWebEngineScriptPrivate::bind(QtWebEngineCore::UserScriptControllerHost *scriptController, QtWebEngineCore::WebContentsAdapter *adapter)
{
    aboutToUpdateUnderlyingScript();
    m_adapter = adapter;
    m_controllerHost = scriptController;
}

QQuickWebEngineScriptPrivate::QQuickWebEngineScriptPrivate()
    :m_controllerHost(0)
    , m_adapter(0)

{
}

void QQuickWebEngineScriptPrivate::aboutToUpdateUnderlyingScript()
{
    Q_Q(QQuickWebEngineScript);
    if (m_controllerHost)
        m_controllerHost->removeUserScript(coreScript, m_adapter);
   // Defer updates to the next event loop
   m_basicTimer.start(0, q);
}
