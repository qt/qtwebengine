// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/renderer/extensions/chrome_extensions_renderer_client.cc:
// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions_renderer_client_qt.h"

#include "renderer/render_configuration.h"
#include "resource_request_policy_qt.h"

#include "base/command_line.h"
#include "base/lazy_instance.h"
#include "base/stl_util.h"
#include "base/types/optional_util.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_thread.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handlers/background_info.h"
#include "extensions/common/switches.h"
#include "extensions/renderer/dispatcher.h"
#include "extensions/renderer/extension_frame_helper.h"
#include "extensions/renderer/extension_web_view_helper.h"
#include "extensions/renderer/extensions_render_frame_observer.h"
#include "extensions/renderer/renderer_extension_registry.h"
#include "extensions/renderer/script_context.h"
#include "extensions/renderer/extensions_renderer_api_provider.h"
#include "third_party/blink/public/platform/web_url.h"
#include "third_party/blink/public/web/web_plugin_params.h"

namespace chrome {
const char kExtensionInvalidRequestURL[] = "chrome-extension://invalid/";
}

namespace QtWebEngineCore {

ExtensionsRendererClientQt::ExtensionsRendererClientQt()
{
}

ExtensionsRendererClientQt::~ExtensionsRendererClientQt()
{
}

// Returns true if the current render process was launched incognito.
bool ExtensionsRendererClientQt::IsIncognitoProcess() const
{
    return RenderConfiguration::is_incognito_process();
}

// Returns the lowest isolated world ID available to extensions.
// Must be greater than 0. See blink::WebFrame::executeScriptInIsolatedWorld
// (third_party/WebKit/public/web/WebFrame.h) for additional context.
int ExtensionsRendererClientQt::GetLowestIsolatedWorldId() const
{
    return 257;
}

// static
ExtensionsRendererClientQt *ExtensionsRendererClientQt::GetInstance()
{
    static base::LazyInstance<ExtensionsRendererClientQt>::Leaky client =
            LAZY_INSTANCE_INITIALIZER;
    return client.Pointer();
}

void ExtensionsRendererClientQt::FinishInitialization()
{
    resource_request_policy_.reset(new extensions::ResourceRequestPolicyQt(dispatcher()));
}

void ExtensionsRendererClientQt::WebViewCreated(blink::WebView *web_view, const url::Origin *outermost_origin)
{
    new extensions::ExtensionWebViewHelper(web_view, outermost_origin);
}

bool ExtensionsRendererClientQt::OverrideCreatePlugin(content::RenderFrame *render_frame,
                                                      const blink::WebPluginParams &params)
{
    if (params.mime_type.Utf8() != content::kBrowserPluginMimeType)
        return true;
    bool guest_view_api_available = false;
    return !guest_view_api_available;
}

void ExtensionsRendererClientQt::WillSendRequest(blink::WebLocalFrame *frame,
                                                 ui::PageTransition transition_type,
                                                 const blink::WebURL &url,
                                                 const net::SiteForCookies &site_for_cookies,
                                                 const url::Origin *initiator_origin,
                                                 GURL *new_url)
{
    if (url.ProtocolIs(extensions::kExtensionScheme) &&
            !resource_request_policy_->CanRequestResource(url, frame,
                                                          transition_type,
                                                          initiator_origin)) {
        *new_url = GURL(chrome::kExtensionInvalidRequestURL);
    }
}

void ExtensionsRendererClientQt::RunScriptsAtDocumentStart(content::RenderFrame *render_frame)
{
    dispatcher()->RunScriptsAtDocumentStart(render_frame);
}

void ExtensionsRendererClientQt::RunScriptsAtDocumentEnd(content::RenderFrame *render_frame)
{
    dispatcher()->RunScriptsAtDocumentEnd(render_frame);
}

void ExtensionsRendererClientQt::RunScriptsAtDocumentIdle(content::RenderFrame *render_frame)
{
    dispatcher()->RunScriptsAtDocumentIdle(render_frame);
}


} // namespace QtWebEngineCore
