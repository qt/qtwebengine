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

#include "user_script_controller_host.h"

#include "common/qt_messages.h"
#include "type_conversion.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_p.h"

#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_process_host_observer.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"

namespace QtWebEngineCore {

class UserScriptControllerHost::WebContentsObserverHelper : public content::WebContentsObserver {
public:
    WebContentsObserverHelper(UserScriptControllerHost *, content::WebContents *);
    virtual void AboutToNavigateRenderView(content::RenderViewHost* renderViewHost) Q_DECL_OVERRIDE;

    virtual void WebContentsDestroyed() Q_DECL_OVERRIDE;
private:
    UserScriptControllerHost *m_controllerHost;
};

UserScriptControllerHost::WebContentsObserverHelper::WebContentsObserverHelper(UserScriptControllerHost *controller, content::WebContents *contents)
    : content::WebContentsObserver(contents)
    , m_controllerHost(controller)
{
}

void UserScriptControllerHost::WebContentsObserverHelper::AboutToNavigateRenderView(content::RenderViewHost *renderViewHost)
{
    content::WebContents *contents = web_contents();
    Q_FOREACH (const UserScript &script, m_controllerHost->m_perContentsScripts.value(contents))
        renderViewHost->Send(new RenderViewObserverHelper_AddScript(renderViewHost->GetRoutingID(), script.data()));
}

void UserScriptControllerHost::WebContentsObserverHelper::WebContentsDestroyed()
{
    m_controllerHost->webContentsDestroyed(web_contents());
    delete this;
}

class UserScriptControllerHost::RenderProcessObserverHelper : public content::RenderProcessHostObserver {
public:
    RenderProcessObserverHelper(UserScriptControllerHost *);
    virtual void RenderProcessHostDestroyed(content::RenderProcessHost *) Q_DECL_OVERRIDE;
private:
    UserScriptControllerHost *m_controllerHost;
};

UserScriptControllerHost::RenderProcessObserverHelper::RenderProcessObserverHelper(UserScriptControllerHost *controller)
    : m_controllerHost(controller)
{
}

void UserScriptControllerHost::RenderProcessObserverHelper::RenderProcessHostDestroyed(content::RenderProcessHost *renderer)
{
    Q_ASSERT(m_controllerHost);
    m_controllerHost->m_observedProcesses.remove(renderer);
}

void UserScriptControllerHost::addUserScript(const UserScript &script, WebContentsAdapter *adapter)
{
    if (script.isNull())
        return;
    // Global scripts should be dispatched to all our render processes.
    if (!adapter) {
        m_profileWideScripts.insert(script);
        Q_FOREACH (content::RenderProcessHost *renderer, m_observedProcesses)
            renderer->Send(new UserScriptController_AddScript(script.data()));
    } else {
        content::WebContents *contents = adapter->webContents();
        ContentsScriptsMap::iterator it = m_perContentsScripts.find(contents);
        if (it == m_perContentsScripts.end()) {
            // We need to keep track of RenderView/RenderViewHost changes for a given contents
            // in order to make sure the scripts stay in sync
            new WebContentsObserverHelper(this, contents);
            it = m_perContentsScripts.insert(contents, (QSet<UserScript>() << script));
        } else {
            QSet<UserScript> currentScripts = it.value();
            currentScripts.insert(script);
            m_perContentsScripts.insert(contents, currentScripts);
        }
        contents->Send(new RenderViewObserverHelper_AddScript(contents->GetRoutingID(), script.data()));
    }
}

bool UserScriptControllerHost::containsUserScript(const UserScript &script, WebContentsAdapter *adapter)
{
    if (script.isNull())
        return false;
    // Global scripts should be dispatched to all our render processes.
    if (!adapter)
        return m_profileWideScripts.contains(script);
    return m_perContentsScripts.value(adapter->webContents()).contains(script);
}

bool UserScriptControllerHost::removeUserScript(const UserScript &script, WebContentsAdapter *adapter)
{
    if (script.isNull())
        return false;
    if (!adapter) {
        QSet<UserScript>::iterator it = m_profileWideScripts.find(script);
        if (it == m_profileWideScripts.end())
            return false;
        Q_FOREACH (content::RenderProcessHost *renderer, m_observedProcesses)
            renderer->Send(new UserScriptController_RemoveScript((*it).data()));
        m_profileWideScripts.erase(it);
    } else {
        content::WebContents *contents = adapter->webContents();
        if (!m_perContentsScripts.contains(contents))
            return false;
        QSet<UserScript> &set(m_perContentsScripts[contents]);
        QSet<UserScript>::iterator it = set.find(script);
        if (it == set.end())
            return false;
        contents->Send(new RenderViewObserverHelper_RemoveScript(contents->GetRoutingID(), (*it).data()));
        set.erase(it);
    }
    return true;
}

void UserScriptControllerHost::clearAllScripts(WebContentsAdapter *adapter)
{
    if (!adapter) {
        m_profileWideScripts.clear();
        Q_FOREACH (content::RenderProcessHost *renderer, m_observedProcesses)
            renderer->Send(new UserScriptController_ClearScripts);
    } else {
        content::WebContents *contents = adapter->webContents();
        m_perContentsScripts.remove(contents);
        contents->Send(new RenderViewObserverHelper_ClearScripts(contents->GetRoutingID()));
    }
}

const QSet<UserScript> UserScriptControllerHost::registeredScripts(WebContentsAdapter *adapter) const
{
    if (!adapter)
        return m_profileWideScripts;
    return m_perContentsScripts.value(adapter->webContents());
}

void UserScriptControllerHost::reserve(WebContentsAdapter *adapter, int count)
{
    if (!adapter)
        m_profileWideScripts.reserve(count);
    else
        m_perContentsScripts[adapter->webContents()].reserve(count);
}

void UserScriptControllerHost::renderProcessStartedWithHost(content::RenderProcessHost *renderer)
{
    if (m_observedProcesses.contains(renderer))
        return;

    if (m_renderProcessObserver.isNull())
        m_renderProcessObserver.reset(new RenderProcessObserverHelper(this));
    renderer->AddObserver(m_renderProcessObserver.data());
    m_observedProcesses.insert(renderer);
    Q_FOREACH (const UserScript &script, m_profileWideScripts)
        renderer->Send(new UserScriptController_AddScript(script.data()));
}

void UserScriptControllerHost::webContentsDestroyed(content::WebContents *contents)
{
    m_perContentsScripts.remove(contents);
}

UserScriptControllerHost::UserScriptControllerHost()
{
}

UserScriptControllerHost::~UserScriptControllerHost()
{
    Q_FOREACH (content::RenderProcessHost *renderer, m_observedProcesses)
        renderer->RemoveObserver(m_renderProcessObserver.data());
}

} // namespace
