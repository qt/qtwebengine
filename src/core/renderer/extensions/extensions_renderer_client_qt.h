// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef EXTENSIONSRENDERERCLIENTQT_H
#define EXTENSIONSRENDERERCLIENTQT_H

#include <memory>

#include "extensions/renderer/extensions_renderer_client.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "ui/base/page_transition_types.h"

class GURL;

namespace blink {
class WebLocalFrame;
struct WebPluginParams;
class WebURL;
class WebView;
}

namespace content {
class RenderFrame;
}

namespace net {
class SiteForCookies;
}

namespace url {
class Origin;
}

namespace extensions {
class Dispatcher;
class ResourceRequestPolicyQt;
}

namespace QtWebEngineCore {

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
    void RenderThreadStarted() override;

    // Match ContentRendererClientQt's method names...
    void WebViewCreated(blink::WebView *web_view,
                        const url::Origin *outermost_origin);
    void RenderFrameCreated(content::RenderFrame *, service_manager::BinderRegistry *);
    bool OverrideCreatePlugin(content::RenderFrame *render_frame,
                              const blink::WebPluginParams &params);
    void WillSendRequest(blink::WebLocalFrame *frame,
                         ui::PageTransition transition_type,
                         const blink::WebURL &url,
                         const net::SiteForCookies &site_for_cookies,
                         const url::Origin *initiator_origin,
                         GURL *new_url);

    void RunScriptsAtDocumentStart(content::RenderFrame *render_frame);
    void RunScriptsAtDocumentEnd(content::RenderFrame *render_frame);
    void RunScriptsAtDocumentIdle(content::RenderFrame *render_frame);

    extensions::Dispatcher *extension_dispatcher()
    { return extension_dispatcher_.get(); }

    static ExtensionsRendererClientQt *GetInstance();

private:
    std::unique_ptr<extensions::Dispatcher> extension_dispatcher_;
    std::unique_ptr<extensions::ResourceRequestPolicyQt> resource_request_policy_;
};

} // namespace QtWebEngineCore

#endif // EXTENSIONSRENDERERCLIENTQT_H
