// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "extensions/plugin_service_filter_qt.h"

#include "content/public/browser/render_process_host.h"

#include "web_contents_delegate_qt.h"
#include "web_engine_settings.h"

using namespace QtWebEngineCore;

namespace extensions {

// static
PluginServiceFilterQt *PluginServiceFilterQt::GetInstance()
{
    return base::Singleton<PluginServiceFilterQt>::get();
}

bool PluginServiceFilterQt::IsPluginAvailable(int render_process_id, int render_frame_id,
                                              content::BrowserContext *browser_context,
                                              const content::WebPluginInfo &plugin)
{
    Q_UNUSED(plugin);
    Q_UNUSED(browser_context);
    content::RenderFrameHost *frame_host = content::RenderFrameHost::FromID(render_process_id, render_frame_id);
    content::WebContents *web_contents = content::WebContents::FromRenderFrameHost(frame_host);
    if (!web_contents) {
        // Availability checked somewhere before/during WebContents initialization. Let it load and enable
        // all the plugins at this phase. This information will be queried again when receiving the response
        // for the requested content. Postponing our decision and enabling/blocking there makes WebEngineSettings
        // modifiable in runtime without reconstructing WebContents.
        return true;
    }

    if (web_contents->IsInnerWebContentsForGuest())
        web_contents = web_contents->GetOuterWebContents();

    if (auto *delegate = static_cast<WebContentsDelegateQt *>(web_contents->GetDelegate())) {
        const WebEngineSettings *settings = delegate->webEngineSettings();
        if (!settings->testAttribute(QWebEngineSettings::PdfViewerEnabled)
            || !settings->testAttribute(QWebEngineSettings::PluginsEnabled))
            return false;
    }

    return true;
}

bool PluginServiceFilterQt::CanLoadPlugin(int render_process_id,
                                          const base::FilePath &path)
{
    return true;
}

PluginServiceFilterQt::PluginServiceFilterQt()
{
}

PluginServiceFilterQt::~PluginServiceFilterQt()
{
}

} // namespace extensions
