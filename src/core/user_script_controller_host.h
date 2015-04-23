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

#ifndef USER_SCRIPT_CONTROLLER_HOST_H
#define USER_SCRIPT_CONTROLLER_HOST_H

#include "qtwebenginecoreglobal.h"

#include <QtCore/QSet>
#include <QtCore/QScopedPointer>
#include "user_script.h"

namespace content {
class RenderProcessHost;
class WebContents;
}

namespace QtWebEngineCore {

class WebContentsAdapter;
class WebContentsAdapterPrivate;

class QWEBENGINE_EXPORT UserScriptControllerHost {

public:
    UserScriptControllerHost();
    ~UserScriptControllerHost();

    void addUserScript(const UserScript &script, WebContentsAdapter *adapter);
    bool containsUserScript(const UserScript &script, WebContentsAdapter *adapter);
    bool removeUserScript(const UserScript &script, WebContentsAdapter *adapter);
    void clearAllScripts(WebContentsAdapter *adapter);
    void reserve(WebContentsAdapter *adapter, int count);
    const QSet<UserScript> registeredScripts(WebContentsAdapter *adapter) const;

    void renderProcessStartedWithHost(content::RenderProcessHost *renderer);

private:
    Q_DISABLE_COPY(UserScriptControllerHost)
    class WebContentsObserverHelper;
    class RenderProcessObserverHelper;

    void webContentsDestroyed(content::WebContents *);

    QSet<UserScript> m_profileWideScripts;
    typedef QHash<content::WebContents *, QSet<UserScript>> ContentsScriptsMap;
    ContentsScriptsMap m_perContentsScripts;
    QSet<content::RenderProcessHost *> m_observedProcesses;
    QScopedPointer<RenderProcessObserverHelper> m_renderProcessObserver;
};

} // namespace

#endif // USER_SCRIPT_CONTROLLER_HOST_H
