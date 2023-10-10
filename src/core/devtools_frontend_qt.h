// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DEVTOOLS_FRONTEND_QT_H
#define DEVTOOLS_FRONTEND_QT_H

#include <memory>
#include <set>

#include "web_contents_delegate_qt.h"

#include "base/containers/unique_ptr_adapters.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/web_contents_observer.h"

namespace base {
class Value;
}

namespace content {
class DevToolsFrontendHost;
class NavigationHandle;
class WebContents;
}  // namespace content

class DevToolsEyeDropper;
class PersistentPrefStore;

namespace QtWebEngineCore {

class DevToolsFrontendQt : public content::WebContentsObserver
                         , public content::DevToolsAgentHostClient {
public:
    static DevToolsFrontendQt *Show(QSharedPointer<WebContentsAdapter> frontendAdapter, content::WebContents *inspectedContents);

    void Activate();
    void Focus();
    void InspectElementAt(int x, int y);
    void Close();

    void DisconnectFromTarget();

    void CallClientFunction(const std::string &object_name,
                            const std::string &method_name,
                            base::Value arg1 = {},
                            base::Value arg2 = {},
                            base::Value arg3 = {},
                            base::OnceCallback<void(base::Value)> cb = base::NullCallback());

    WebContentsDelegateQt *frontendDelegate() const
    {
        return m_frontendDelegate;
    }

protected:
    DevToolsFrontendQt(QSharedPointer<WebContentsAdapter> webContentsAdapter, content::WebContents *inspectedContents);
    ~DevToolsFrontendQt() override;

    // content::DevToolsAgentHostClient implementation.
    void AgentHostClosed(content::DevToolsAgentHost* agent_host) override;
    void DispatchProtocolMessage(content::DevToolsAgentHost* agent_host, base::span<const uint8_t> message) override;

    void SetPreferences(const std::string& json);
    void HandleMessageFromDevToolsFrontend(base::Value message);

private:
    // WebContentsObserver overrides
    void ReadyToCommitNavigation(content::NavigationHandle* navigation_handle) override;
    void DocumentOnLoadCompletedInPrimaryMainFrame() override;
    void WebContentsDestroyed() override;

    void SendMessageAck(int request_id, base::Value arg1);
    void SetPreference(const std::string &name, const std::string &value);
    void RemovePreference(const std::string &name);
    void ClearPreferences();
    void CreateJsonPreferences(bool clear);
    void SetEyeDropperActive(bool active);
    void ColorPickedInEyeDropper(int r, int g, int b, int a);

    // We shouldn't be keeping it alive
    QWeakPointer<WebContentsAdapter> m_frontendAdapter;
    WebContentsAdapter *m_inspectedAdapter;
    WebContentsDelegateQt *m_frontendDelegate;
    content::WebContents *m_inspectedContents;
    scoped_refptr<content::DevToolsAgentHost> m_agentHost;
    int m_inspect_element_at_x;
    int m_inspect_element_at_y;
    std::unique_ptr<content::DevToolsFrontendHost> m_frontendHost;
    std::unique_ptr<DevToolsEyeDropper> m_eyeDropper;

    class NetworkResourceLoader;
    std::set<std::unique_ptr<NetworkResourceLoader>, base::UniquePtrComparator> m_loaders;

    base::DictionaryValue m_preferences;
    scoped_refptr<PersistentPrefStore> m_prefStore;
    base::WeakPtrFactory<DevToolsFrontendQt> m_weakFactory;
};

} // namespace QtWebEngineCore

#endif  // DEVTOOLS_FRONTEND_QT_H
