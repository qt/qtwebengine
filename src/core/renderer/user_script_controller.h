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

#ifndef USER_SCRIPT_CONTROLLER_H
#define USER_SCRIPT_CONTROLLER_H

#include "content/public/renderer/render_process_observer.h"

#include "common/user_script_data.h"

#include <QtCore/qcompilerdetection.h>
#include <QtCore/QHash>
#include <QtCore/QSet>

namespace content {
class RenderView;
}


class UserScriptController : public content::RenderProcessObserver {

public:
    static UserScriptController *instance();
    UserScriptController();
    void renderViewCreated(content::RenderView *);
    void renderViewDestroyed(content::RenderView *);
    void addScriptForView(const UserScriptData &, content::RenderView *);
    void removeScriptForView(const UserScriptData &, content::RenderView *);
    void clearScriptsForView(content::RenderView *);

private:
    Q_DISABLE_COPY(UserScriptController)

    class RenderViewObserverHelper;

    // RenderProcessObserver implementation.
    virtual bool OnControlMessageReceived(const IPC::Message &message) Q_DECL_OVERRIDE;

    void onAddScript(const UserScriptData &);
    void onRemoveScript(const UserScriptData &);
    void onClearScripts();

    typedef QSet<uint64> UserScriptSet;
    typedef QHash<const content::RenderView *, UserScriptSet> ViewUserScriptMap;
    ViewUserScriptMap m_viewUserScriptMap;
    QHash<uint64, UserScriptData> m_scripts;
};

#endif // USER_SCRIPT_CONTROLLER_H
