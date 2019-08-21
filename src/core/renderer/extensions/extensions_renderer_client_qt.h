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

#ifndef EXTENSIONSRENDERERCLIENTQT_H
#define EXTENSIONSRENDERERCLIENTQT_H

#include <memory>
#include <string>

#include "base/macros.h"
#include "extensions/renderer/extensions_renderer_client.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "ui/base/page_transition_types.h"

class GURL;

namespace blink {
class WebLocalFrame;
struct WebPluginParams;
class WebURL;
}

namespace content {
class BrowserPluginDelegate;
class RenderFrame;
class RenderView;
struct WebPluginInfo;
}

namespace url {
class Origin;
}

namespace extensions {
class Dispatcher;
class ExtensionsGuestViewContainerDispatcher;
class ResourceRequestPolicyQt;
}

namespace QtWebEngineCore {

class ExtensionsDispatcherDelegateQt;
class RendererPermissionsPolicyDelegateQt;

class ExtensionsRendererClientQt : public extensions::ExtensionsRendererClient
{
public:
    ExtensionsRendererClientQt();
    ~ExtensionsRendererClientQt() override;

    // extensions::ExtensionsRendererClient implementation.
    bool IsIncognitoProcess() const override;
    int GetLowestIsolatedWorldId() const override;
    extensions::Dispatcher *GetDispatcher() override;
    void OnExtensionLoaded(const extensions::Extension &extension) override;
    void OnExtensionUnloaded(const extensions::ExtensionId &extension_id) override;

    // Match ContentRendererClientQt's method names...
    void RenderThreadStarted();
    void RenderFrameCreated(content::RenderFrame *, service_manager::BinderRegistry *);
    bool OverrideCreatePlugin(content::RenderFrame *render_frame,
                              const blink::WebPluginParams &params);
    void WillSendRequest(blink::WebLocalFrame *frame,
                         ui::PageTransition transition_type,
                         const blink::WebURL &url,
                         const url::Origin *initiator_origin,
                         GURL *new_url,
                         bool *attach_same_site_cookies);

    static bool ShouldFork(blink::WebLocalFrame *frame,
                           const GURL &url,
                           bool is_initial_navigation,
                           bool is_server_redirect,
                           bool *send_referrer);
    static content::BrowserPluginDelegate *CreateBrowserPluginDelegate(content::RenderFrame *render_frame,
                                                                       const content::WebPluginInfo &info,
                                                                       const std::string &mime_type,
                                                                       const GURL &original_url);

    bool ExtensionAPIEnabledForServiceWorkerScript(const GURL &scope, const GURL &script_url) const override;

    void RunScriptsAtDocumentStart(content::RenderFrame *render_frame);
    void RunScriptsAtDocumentEnd(content::RenderFrame *render_frame);
    void RunScriptsAtDocumentIdle(content::RenderFrame *render_frame);

    extensions::Dispatcher *extension_dispatcher()
    { return extension_dispatcher_.get(); }

    static ExtensionsRendererClientQt *GetInstance();

private:
    std::unique_ptr<ExtensionsDispatcherDelegateQt> extension_dispatcher_delegate_;
    std::unique_ptr<RendererPermissionsPolicyDelegateQt> permissions_policy_delegate_;
    std::unique_ptr<extensions::Dispatcher> extension_dispatcher_;
    std::unique_ptr<extensions::ExtensionsGuestViewContainerDispatcher> guest_view_container_dispatcher_;
    std::unique_ptr<extensions::ResourceRequestPolicyQt> resource_request_policy_;
};

} // namespace QtWebEngineCore

#endif // EXTENSIONSRENDERERCLIENTQT_H
