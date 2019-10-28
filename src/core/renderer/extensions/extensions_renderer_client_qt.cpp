/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// based on chrome/renderer/extensions/chrome_extensions_renderer_client.cc:
// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions_renderer_client_qt.h"

#include "extensions_dispatcher_delegate_qt.h"
#include "renderer/render_thread_observer_qt.h"
#include "renderer_permissions_policy_delegate_qt.h"
#include "resource_request_policy_qt.h"

#include "base/command_line.h"
#include "base/lazy_instance.h"
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
#include "extensions/renderer/extensions_render_frame_observer.h"
#include "extensions/renderer/guest_view/extensions_guest_view_container.h"
#include "extensions/renderer/guest_view/extensions_guest_view_container_dispatcher.h"
#include "extensions/renderer/guest_view/mime_handler_view/mime_handler_view_container.h"
#include "extensions/renderer/renderer_extension_registry.h"
#include "extensions/renderer/script_context.h"
#include "third_party/blink/public/platform/web_url.h"
#include "third_party/blink/public/web/web_plugin_params.h"

namespace chrome {
const char kExtensionInvalidRequestURL[] = "chrome-extension://invalid/";
const char kExtensionResourceInvalidRequestURL[] = "chrome-extension-resource://invalid/";
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
    return RenderThreadObserverQt::is_incognito_process();
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

    if (!extensions::ExtensionsClient::Get()->ExtensionAPIEnabledInExtensionServiceWorkers())
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
    // ChromeRenderViewTest::SetUp() creates its own ExtensionDispatcher and
    // injects it using SetExtensionDispatcher(). Don't overwrite it.
    if (!extension_dispatcher_)
        extension_dispatcher_.reset(new extensions::Dispatcher(std::make_unique<ExtensionsDispatcherDelegateQt>()));
    extension_dispatcher_->OnRenderThreadStarted(thread);
    permissions_policy_delegate_.reset(new RendererPermissionsPolicyDelegateQt(extension_dispatcher_.get()));
    resource_request_policy_.reset(new extensions::ResourceRequestPolicyQt(extension_dispatcher_.get()));
    guest_view_container_dispatcher_.reset(new extensions::ExtensionsGuestViewContainerDispatcher());

    thread->AddObserver(extension_dispatcher_.get());
    thread->AddObserver(guest_view_container_dispatcher_.get());
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
                                                 const url::Origin *initiator_origin,
                                                 GURL *new_url,
                                                 bool *attach_same_site_cookies)
{
    if (url.ProtocolIs(extensions::kExtensionScheme) &&
            !resource_request_policy_->CanRequestResource(url, frame, transition_type)) {
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

content::BrowserPluginDelegate *ExtensionsRendererClientQt::CreateBrowserPluginDelegate(content::RenderFrame *render_frame,
                                                                                        const content::WebPluginInfo &info,
                                                                                        const std::string &mime_type,
                                                                                        const GURL &original_url)
{
    if (mime_type == content::kBrowserPluginMimeType)
        return new extensions::ExtensionsGuestViewContainer(render_frame);
    return new extensions::MimeHandlerViewContainer(render_frame, info, mime_type, original_url);
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
