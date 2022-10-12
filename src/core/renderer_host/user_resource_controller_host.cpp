// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "user_resource_controller_host.h"

#include "type_conversion.h"
#include "web_contents_adapter.h"

#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_process_host_observer.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "ipc/ipc_channel_proxy.h"
#include "qtwebengine/userscript/userscript.mojom.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"

namespace QtWebEngineCore {

class UserResourceControllerHost::WebContentsObserverHelper : public content::WebContentsObserver
{
public:
    WebContentsObserverHelper(UserResourceControllerHost *, content::WebContents *);

    // WebContentsObserver overrides:
    void RenderFrameCreated(content::RenderFrameHost *renderFrameHost) override;
    void RenderFrameHostChanged(content::RenderFrameHost *oldHost, content::RenderFrameHost *newHost) override;
    void RenderFrameDeleted(content::RenderFrameHost *render_frame_host) override;
    void WebContentsDestroyed() override;

private:
    UserResourceControllerHost *m_controllerHost;
};

UserResourceControllerHost::WebContentsObserverHelper::WebContentsObserverHelper(UserResourceControllerHost *controller,
                                                                                 content::WebContents *contents)
        : content::WebContentsObserver(contents)
        , m_controllerHost(controller)
{
}

void UserResourceControllerHost::WebContentsObserverHelper::RenderFrameCreated(content::RenderFrameHost *renderFrameHost)
{
    content::WebContents *contents = web_contents();
    auto &remote = m_controllerHost->GetUserResourceControllerRenderFrame(renderFrameHost);
    const QList<UserScript> scripts = m_controllerHost->m_perContentsScripts.value(contents);
    for (const UserScript &script : scripts)
        remote->AddScript(script.data());
}

void UserResourceControllerHost::WebContentsObserverHelper::RenderFrameHostChanged(content::RenderFrameHost *oldHost,
                                                                                   content::RenderFrameHost *newHost)
{
    if (oldHost) {
        auto &remote = m_controllerHost->GetUserResourceControllerRenderFrame(oldHost);
        remote->ClearScripts();
    }
}

void UserResourceControllerHost::WebContentsObserverHelper::RenderFrameDeleted(
        content::RenderFrameHost *render_frame_host)
{
    m_controllerHost->m_renderFrames.erase(render_frame_host);
}

void UserResourceControllerHost::WebContentsObserverHelper::WebContentsDestroyed()
{
    m_controllerHost->webContentsDestroyed(web_contents());
    delete this;
}

class UserResourceControllerHost::RenderProcessObserverHelper : public content::RenderProcessHostObserver
{
public:
    RenderProcessObserverHelper(UserResourceControllerHost *);
    void RenderProcessHostDestroyed(content::RenderProcessHost *) override;

private:
    UserResourceControllerHost *m_controllerHost;
};

UserResourceControllerHost::RenderProcessObserverHelper::RenderProcessObserverHelper(UserResourceControllerHost *controller)
    : m_controllerHost(controller)
{
}

void UserResourceControllerHost::RenderProcessObserverHelper::RenderProcessHostDestroyed(content::RenderProcessHost *renderer)
{
    Q_ASSERT(m_controllerHost);
    delete m_controllerHost->m_observedProcesses[renderer];
    m_controllerHost->m_observedProcesses.remove(renderer);
}

void UserResourceControllerHost::addUserScript(const UserScript &script, WebContentsAdapter *adapter)
{
    // Global scripts should be dispatched to all our render processes.
    const bool isProfileWideScript = !adapter;
    if (isProfileWideScript) {
        if (!m_profileWideScripts.contains(script)) {
            m_profileWideScripts.append(script);
            for (auto controller : m_observedProcesses.values())
                (*controller)->AddScript(script.data());
        }
    } else {
        content::WebContents *contents = adapter->webContents();
        ContentsScriptsMap::iterator it = m_perContentsScripts.find(contents);
        if (it == m_perContentsScripts.end()) {
            // We need to keep track of RenderView/RenderViewHost changes for a given contents
            // in order to make sure the scripts stay in sync
            new WebContentsObserverHelper(this, contents);
            it = m_perContentsScripts.insert(contents, (QList<UserScript>() << script));
        } else {
            QList<UserScript> currentScripts = it.value();
            if (!currentScripts.contains(script)) {
                currentScripts.append(script);
                m_perContentsScripts.insert(contents, currentScripts);
            }
        }
        GetUserResourceControllerRenderFrame(contents->GetPrimaryMainFrame())
                ->AddScript(script.data());
    }
}

bool UserResourceControllerHost::removeUserScript(const UserScript &script, WebContentsAdapter *adapter)
{
    const bool isProfileWideScript = !adapter;
    if (isProfileWideScript) {
        QList<UserScript>::iterator it = std::find(m_profileWideScripts.begin(), m_profileWideScripts.end(), script);
        if (it == m_profileWideScripts.end())
            return false;
        for (auto controller : m_observedProcesses.values())
            (*controller)->RemoveScript((*it).data());
        m_profileWideScripts.erase(it);
    } else {
        content::WebContents *contents = adapter->webContents();
        if (!m_perContentsScripts.contains(contents))
            return false;
        QList<UserScript> &list(m_perContentsScripts[contents]);
        QList<UserScript>::iterator it = std::find(list.begin(), list.end(), script);
        if (it == list.end())
            return false;
        GetUserResourceControllerRenderFrame(contents->GetPrimaryMainFrame())
                ->RemoveScript((*it).data());
        list.erase(it);
    }
    return true;
}

void UserResourceControllerHost::clearAllScripts(WebContentsAdapter *adapter)
{
    const bool isProfileWideScript = !adapter;
    if (isProfileWideScript) {
        m_profileWideScripts.clear();
        for (auto controller : m_observedProcesses.values())
            (*controller)->ClearScripts();
    } else {
        content::WebContents *contents = adapter->webContents();
        m_perContentsScripts.remove(contents);
        mojo::AssociatedRemote<qtwebengine::mojom::UserResourceControllerRenderFrame>
                userResourceController;
        GetUserResourceControllerRenderFrame(contents->GetPrimaryMainFrame())
                ->ClearScripts();
    }
}

void UserResourceControllerHost::reserve(WebContentsAdapter *adapter, int count)
{
    const bool isProfileWideScript = !adapter;
    if (isProfileWideScript)
        m_profileWideScripts.reserve(count);
    else
        m_perContentsScripts[adapter->webContents()].reserve(count);
}

void UserResourceControllerHost::renderProcessStartedWithHost(content::RenderProcessHost *renderer)
{
    if (m_observedProcesses.keys().contains(renderer))
        return;

    if (m_renderProcessObserver.isNull())
        m_renderProcessObserver.reset(new RenderProcessObserverHelper(this));
    renderer->AddObserver(m_renderProcessObserver.data());
    auto userResourceController = new UserResourceControllerRemote;
    renderer->GetChannel()->GetRemoteAssociatedInterface(userResourceController);
    m_observedProcesses.insert(renderer, userResourceController);
    for (const UserScript &script : std::as_const(m_profileWideScripts)) {
        (*userResourceController)->AddScript(script.data());
    }
}

void UserResourceControllerHost::webContentsDestroyed(content::WebContents *contents)
{
    m_perContentsScripts.remove(contents);
}

UserResourceControllerHost::UserResourceControllerHost()
{
}

UserResourceControllerHost::~UserResourceControllerHost()
{
    for (content::RenderProcessHost *renderer : m_observedProcesses.keys()) {
        renderer->RemoveObserver(m_renderProcessObserver.data());
        delete m_observedProcesses[renderer];
    }
}

const UserResourceControllerRenderFrameRemote &
UserResourceControllerHost::GetUserResourceControllerRenderFrame(content::RenderFrameHost *rfh)
{
    auto it = m_renderFrames.find(rfh);
    if (it == m_renderFrames.end()) {
        UserResourceControllerRenderFrameRemote remote;
        rfh->GetRemoteAssociatedInterfaces()->GetInterface(remote.BindNewEndpointAndPassReceiver());
        it = m_renderFrames.insert(std::make_pair(rfh, std::move(remote))).first;
    } else if (it->second.is_bound() && !it->second.is_connected()) {
        it->second.reset();
        rfh->GetRemoteAssociatedInterfaces()->GetInterface(&it->second);
    }

    return it->second;
}

} // namespace
