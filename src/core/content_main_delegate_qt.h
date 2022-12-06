// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CONTENT_MAIN_DELEGATE_QT_H
#define CONTENT_MAIN_DELEGATE_QT_H

#include "content/public/app/content_main_delegate.h"

#include "compositor/content_gpu_client_qt.h"
#include "content_browser_client_qt.h"
#include "content_client_qt.h"
#include "content_utility_client_qt.h"

namespace QtWebEngineCore {

class ContentMainDelegateQt : public content::ContentMainDelegate
{
public:

    // This is where the embedder puts all of its startup code that needs to run
    // before the sandbox is engaged.
    void PreSandboxStartup() override;

    content::ContentClient *CreateContentClient() override;
    content::ContentBrowserClient* CreateContentBrowserClient() override;
    content::ContentGpuClient* CreateContentGpuClient() override;
    content::ContentRendererClient* CreateContentRendererClient() override;
    content::ContentUtilityClient* CreateContentUtilityClient() override;
    absl::optional<int> BasicStartupComplete() override;

private:
    ContentClientQt m_contentClient;
    std::unique_ptr<ContentBrowserClientQt> m_browserClient;
    std::unique_ptr<ContentGpuClientQt> m_gpuClient;
    std::unique_ptr<ContentUtilityClientQt> m_utilityClient;
};

} // namespace QtWebEngineCore

#endif // CONTENT_MAIN_DELEGATE_QT_H
