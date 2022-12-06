// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DEVTOOLS_FRONTEND_QT_H
#define DEVTOOLS_FRONTEND_QT_H

#include <memory>
#include <set>

#include "web_contents_delegate_qt.h"

#include "base/containers/unique_ptr_adapters.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/devtools/devtools_file_helper.h"
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

class DevToolsFrontendQt : public content::WebContentsObserver,
                           public content::DevToolsAgentHostClient,
                           public DevToolsFileHelper::Delegate
{
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

    static bool IsValidFrontendURL(const GURL &url);

protected:
    DevToolsFrontendQt(QSharedPointer<WebContentsAdapter> webContentsAdapter, content::WebContents *inspectedContents);
    ~DevToolsFrontendQt() override;

    // content::DevToolsAgentHostClient implementation.
    void AgentHostClosed(content::DevToolsAgentHost* agent_host) override;
    void DispatchProtocolMessage(content::DevToolsAgentHost* agent_host, base::span<const uint8_t> message) override;

    void SetPreferences(const std::string& json);
    void HandleMessageFromDevToolsFrontend(base::Value::Dict message);

private:
    // WebContentsObserver overrides
    void ReadyToCommitNavigation(content::NavigationHandle* navigation_handle) override;
    void DocumentOnLoadCompletedInPrimaryMainFrame() override;
    void WebContentsDestroyed() override;

    // DevToolsFileHelper::Delegate overrides.
    void FileSystemAdded(const std::string &error,
                         const DevToolsFileHelper::FileSystem *file_system) override;
    void FileSystemRemoved(const std::string &file_system_path) override;
    void FilePathsChanged(const std::vector<std::string> &changed_paths,
                          const std::vector<std::string> &added_paths,
                          const std::vector<std::string> &removed_paths) override;

    void SendMessageAck(int request_id, base::Value arg1);
    void SetPreference(const std::string &name, const std::string &value);
    void RemovePreference(const std::string &name);
    void ClearPreferences();
    void CreateJsonPreferences(bool clear);
    void SetEyeDropperActive(bool active);
    void ColorPickedInEyeDropper(int r, int g, int b, int a);

    void SaveToFile(const std::string &url, const std::string &content, bool saveAs);
    void FileSavedAs(const std::string &url, const std::string &fileSystemPath);
    void CanceledFileSaveAs(const std::string &url);
    void AppendToFile(const std::string &url, const std::string &content);
    void AppendedTo(const std::string &url);
    void AddFileSystem(const std::string &type);
    void UpgradeDraggedFileSystemPermissions(const std::string &fileSystem);
    void RemoveFileSystem(const std::string &fileSystemPath);

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
    std::unique_ptr<DevToolsFileHelper> m_fileHelper;

    class NetworkResourceLoader;
    std::set<std::unique_ptr<NetworkResourceLoader>, base::UniquePtrComparator> m_loaders;

    base::Value::Dict m_preferences;
    scoped_refptr<PersistentPrefStore> m_prefStore;
    base::WeakPtrFactory<DevToolsFrontendQt> m_weakFactory;
};

} // namespace QtWebEngineCore

#endif  // DEVTOOLS_FRONTEND_QT_H
