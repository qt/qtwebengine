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

#ifndef USER_SCRIPT_H
#define USER_SCRIPT_H

#include "qtwebenginecoreglobal.h"

#include <QtCore/QAtomicInt>
#include <QtCore/QScopedPointer>
#include <QtCore/QSharedData>
#include <QtCore/QString>

struct UserScriptData;

namespace QtWebEngineCore {

class UserScriptControllerHost;

class QWEBENGINE_EXPORT UserScript : public QSharedData {
public:
    enum InjectionPoint {
        AfterLoad,
        DocumentLoadFinished,
        DocumentElementCreation
    };

    UserScript();
    UserScript(const UserScript &other);
    ~UserScript();
    UserScript &operator=(const UserScript &other);

    bool isNull() const;

    QString name() const;
    void setName(const QString &);

    QString sourceCode() const;
    void setSourceCode(const QString &);

    InjectionPoint injectionPoint() const;
    void setInjectionPoint(InjectionPoint);

    uint worldId() const;
    void setWorldId(uint id);

    bool runsOnSubFrames() const;
    void setRunsOnSubFrames(bool on);

    bool operator==(const UserScript &) const;

private:
    void initData();
    UserScriptData &data() const;
    friend class UserScriptControllerHost;

    QScopedPointer<UserScriptData> scriptData;
    QString m_name;
};

} // namespace QtWebEngineCore

QT_BEGIN_NAMESPACE
uint qHash(const QtWebEngineCore::UserScript &, uint seed = 0);
QT_END_NAMESPACE

#endif // USER_SCRIPT_H
