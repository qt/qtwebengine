// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef USER_SCRIPT_H
#define USER_SCRIPT_H

#include "qtwebenginecoreglobal_p.h"

#include "qtwebengine/userscript/user_script_data.h"

#include <QtCore/QScopedPointer>
#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtCore/QUrl>


namespace QtWebEngineCore {
class UserResourceControllerHost;

class UserScript : public QSharedData
{
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

    QString name() const;
    void setName(const QString &);

    QString sourceCode() const;
    void setSourceCode(const QString &);

    QUrl sourceUrl() const;
    void setSourceUrl(const QUrl &);

    InjectionPoint injectionPoint() const;
    void setInjectionPoint(InjectionPoint);

    quint32 worldId() const;
    void setWorldId(quint32 id);

    bool runsOnSubFrames() const;
    void setRunsOnSubFrames(bool on);

    bool operator==(const UserScript &) const;

private:
    const UserScriptData &data() const;
    void parseMetadataHeader();
    friend class UserResourceControllerHost;

    UserScriptData m_scriptData;
    QString m_name;
    QUrl m_url;
};

} // namespace QtWebEngineCore

#endif // USER_SCRIPT_H
