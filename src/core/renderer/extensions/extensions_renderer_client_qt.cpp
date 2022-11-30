// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/renderer/extensions/chrome_extensions_renderer_client.cc:
// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions_renderer_client_qt.h"

#include "extensions_dispatcher_delegate_qt.h"
#include "renderer/render_configuration.h"
#include "renderer_permissions_policy_delegate_qt.h"
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

extensions::Dispatcher *ExtensionsRendererClientQt::GetDispatcher()
{
    return extension_dispatcher_.get();
}

void ExtensionsRendererClientQt::OnExtensionLoaded(const extensions::Extension &extension)
{
    resource_request_policy_->OnExtensionLoaded(extension);
}

void ExtensionsRendererClientQt::OnExtensionUnloaded(const extensions::ExtensionId &extension_id)
{
    resource_request_policy_->OnExtensionUnloaded(extension_id);
}

bool ExtensionsRendererClientQt::ExtensionAPIEnabledForServiceWorkerScript(const GURL &scope, const GURL &script_url) const
{
    if (!script_url.SchemeIs(extensions::kExtensionScheme))
        return false;

    const extensions::Extension* extension =
            extensions::RendererExtensionRegistry::Get()->GetExtensionOrAppByURL(script_url);

    if (!extension || !extensions::BackgroundInfo::IsServiceWorkerBased(extension))
        return false;

    if (scope != extension->url())
        return false;

    const std::string& sw_script = extensions::BackgroundInfo::GetBackgroundServiceWorkerScript(extension);

    return extension->GetResourceURL(sw_script) == script_url;
}

void ExtensionsRendererClientQt::RenderThreadStarted()
{
    content::RenderThread *thread = content::RenderThread::Get();
    if (!extension_dispatcher_)
        extension_dispatcher_.reset(new extensions::Dispatcher(std::make_unique<ExtensionsDispatcherDelegateQt>()));
    extension_dispatcher_->OnRenderThreadStarted(thread);
    permissions_policy_delegate_.reset(new RendererPermissionsPolicyDelegateQt(extension_dispatcher_.get()));
    resource_request_policy_.reset(new extensions::ResourceRequestPolicyQt(extension_dispatcher_.get()));

    thread->AddObserver(extension_dispatcher_.get());
}

void ExtensionsRendererClientQt::WebViewCreated(blink::WebView *web_view, const url::Origin *outermost_origin)
{
    new extensions::ExtensionWebViewHelper(web_view, outermost_origin);
}

void ExtensionsRendererClientQt::RenderFrameCreated(content::RenderFrame *render_frame,
                                                    service_manager::BinderRegistry *registry)
{
    new extensions::ExtensionsRenderFrameObserver(render_frame, registry);
    new extensions::ExtensionFrameHelper(render_frame,
                                         extension_dispatcher_.get());
    extension_dispatcher_->OnRenderFrameCreated(render_frame);
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
                                                          base::OptionalFromPtr(initiator_origin))) {
        *new_url = GURL(chrome::kExtensionInvalidRequestURL);
    }
}

bool ExtensionsRendererClientQt::ShouldFork(blink::WebLocalFrame *frame,
                                            const GURL &url,
                                            bool is_initial_navigation,
                                            bool is_server_redirect,
                                            bool *send_referrer)
{
    return false; // TODO: Fix this to a sensible value
}

void ExtensionsRendererClientQt::RunScriptsAtDocumentStart(content::RenderFrame *render_frame)
{
    extension_dispatcher_->RunScriptsAtDocumentStart(render_frame);
}

void ExtensionsRendererClientQt::RunScriptsAtDocumentEnd(content::RenderFrame *render_frame)
{
    extension_dispatcher_->RunScriptsAtDocumentEnd(render_frame);
}

void ExtensionsRendererClientQt::RunScriptsAtDocumentIdle(content::RenderFrame *render_frame)
{
    extension_dispatcher_->RunScriptsAtDocumentIdle(render_frame);
}


} // namespace QtWebEngineCore
