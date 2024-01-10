// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DEVTOOLS_FRONTEND_QT_H
#define DEVTOOLS_FRONTEND_QT_H

#include <memory>
#include <set>

#include "web_contents_delegate_qt.h"

#include "base/containers/unique_ptr_adapters.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/devtools/devtools_ui_bindings.h"
#include "content/public/browser/web_contents_observer.h"

class DevToolsEyeDropper;

namespace content {
class DevToolsAgentHost;
class WebContents;
} // namespace content

namespace QtWebEngineCore {
class WebContentsAdapter;

class DevToolsFrontendQt : public DevToolsUIBindings::Delegate, public content::WebContentsObserver
{
public:
    static DevToolsFrontendQt *Show(QSharedPointer<WebContentsAdapter> frontendAdapter,
                                    content::WebContents *inspectedContents);

    void Activate();
    void Focus();
    void InspectElementAt(int x, int y);
    void Close();

    void DisconnectFromTarget();

    WebContentsDelegateQt *frontendDelegate() const;

    static bool IsValidFrontendURL(const GURL &url);
    static std::string GetId(content::WebContents *inspectedContents);

protected:
    DevToolsFrontendQt(QSharedPointer<WebContentsAdapter> webContentsAdapter,
                       content::WebContents *inspectedContents);
    ~DevToolsFrontendQt() override;

private:
    void ColorPickedInEyeDropper(int r, int g, int b, int a);

    // content::WebContentsObserver overrides
    void WebContentsDestroyed() override;

    // DevToolsUIBindings::Delegate overrides
    void ActivateWindow() override;
    void SetEyeDropperActive(bool active) override;
    void OpenInNewTab(const std::string &url) override;
    void InspectedContentsClosing() override;
    void OnLoadCompleted() override;

    void InspectElementCompleted() override{};
    void CloseWindow() override;
    void Inspect(scoped_refptr<content::DevToolsAgentHost>) override{};
    void SetInspectedPageBounds(const gfx::Rect &) override{};
    void SetIsDocked(bool) override{};
    void SetWhitelistedShortcuts(const std::string &) override{};
    void OpenNodeFrontend() override{};
    void ReadyForTest() override{};
    void ConnectionReady() override{};
    void SetOpenNewWindowForPopups(bool) override{};
    void RenderProcessGone(bool) override{};
    void ShowCertificateViewer(const std::string &) override{};

    // We shouldn't be keeping it alive
    QWeakPointer<WebContentsAdapter> m_frontendAdapter;
    content::WebContents *m_inspectedContents;
    std::unique_ptr<DevToolsEyeDropper> m_eyeDropper;
    DevToolsUIBindings *m_bindings;
};

} // namespace QtWebEngineCore

#endif // DEVTOOLS_FRONTEND_QT_H
