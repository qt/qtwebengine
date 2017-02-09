/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "common/user_script_data.h"
#include "user_script.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

ASSERT_ENUMS_MATCH(UserScript::AfterLoad, UserScriptData::AfterLoad)
ASSERT_ENUMS_MATCH(UserScript::DocumentLoadFinished, UserScriptData::DocumentLoadFinished)
ASSERT_ENUMS_MATCH(UserScript::DocumentElementCreation, UserScriptData::DocumentElementCreation)

UserScript::UserScript()
    : QSharedData()
{
}

UserScript::UserScript(const UserScript &other)
    : QSharedData(other)
{
    if (other.isNull())
        return;
    scriptData.reset(new UserScriptData(*other.scriptData));
    m_name = other.m_name;
}

UserScript::~UserScript()
{
}

UserScript &UserScript::operator=(const UserScript &other)
{
    if (other.isNull()) {
        scriptData.reset();
        m_name = QString();
        return *this;
    }
    scriptData.reset(new UserScriptData(*other.scriptData));
    m_name = other.m_name;
    return *this;
}

QString UserScript::name() const
{
    return m_name;
}

void UserScript::setName(const QString &name)
{
    m_name = name;
    initData();
    scriptData->url = GURL(QStringLiteral("userScript:%1").arg(name).toStdString());
}

QString UserScript::sourceCode() const
{
    if (isNull())
        return QString();
    return toQt(scriptData->source);
}

void UserScript::setSourceCode(const QString &source)
{
    initData();
    scriptData->source = source.toStdString();
}

UserScript::InjectionPoint UserScript::injectionPoint() const
{
    if (isNull())
        return UserScript::AfterLoad;
    return static_cast<UserScript::InjectionPoint>(scriptData->injectionPoint);
}

void UserScript::setInjectionPoint(UserScript::InjectionPoint p)
{
    initData();
    scriptData->injectionPoint = p;
}

uint UserScript::worldId() const
{
    if (isNull())
        return 1;
    return scriptData->worldId;
}

void UserScript::setWorldId(uint id)
{
    initData();
    scriptData->worldId = id;
}

bool UserScript::runsOnSubFrames() const
{
    if (isNull())
        return false;
    return scriptData->injectForSubframes;
}

void UserScript::setRunsOnSubFrames(bool on)
{
    initData();
    scriptData->injectForSubframes = on;
}

bool UserScript::operator==(const UserScript &other) const
{
    if (isNull() != other.isNull())
        return false;
    if (isNull()) // neither is valid
        return true;
    return worldId() == other.worldId()
            && runsOnSubFrames() == other.runsOnSubFrames()
            && injectionPoint() == other.injectionPoint()
            && name() == other.name() && sourceCode() == other.sourceCode();
}

void UserScript::initData()
{
    if (scriptData.isNull())
        scriptData.reset(new UserScriptData);
}

bool UserScript::isNull() const
{
    return scriptData.isNull();
}

UserScriptData &UserScript::data() const
{
    return *(scriptData.data());
}

} // namespace QtWebEngineCore
