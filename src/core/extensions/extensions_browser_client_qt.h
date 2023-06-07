// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Portions copyright 2015 The Chromium Embedded Framework Authors.
// Portions copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_CLIENT_QT_H_
#define EXTENSIONS_BROWSER_CLIENT_QT_H_

#include "extensions/browser/extensions_browser_client.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"

namespace extensions {

class ExtensionsAPIClient;

// An ExtensionsBrowserClient that supports a single content::BrowserContent
// with no related incognito context.
class ExtensionsBrowserClientQt : public ExtensionsBrowserClient
{
public:
    ExtensionsBrowserClientQt();
    ~ExtensionsBrowserClientQt() override;

    // ExtensionsBrowserClient overrides:
    bool IsShuttingDown() override;
    bool AreExtensionsDisabled(const base::CommandLine &command_line,
                               content::BrowserContext *context) override;
    bool IsValidContext(content::BrowserContext *context) override;
    bool IsSameContext(content::BrowserContext *first,
                       content::BrowserContext *second) override;
    bool HasOffTheRecordContext(content::BrowserContext *context) override;
    content::BrowserContext *GetOffTheRecordContext(content::BrowserContext *context) override;
    content::BrowserContext *GetOriginalContext(content::BrowserContext *context) override;
    content::BrowserContext *GetRedirectedContextInIncognito(content::BrowserContext *context, bool, bool) override;
    content::BrowserContext *GetContextForRegularAndIncognito(content::BrowserContext *context, bool, bool) override;
    content::BrowserContext *GetRegularProfile(content::BrowserContext *context, bool, bool) override;
    bool IsGuestSession(content::BrowserContext *context) const override;
    bool IsExtensionIncognitoEnabled(const std::string &extension_id, content::BrowserContext *context) const override;
    bool CanExtensionCrossIncognito(const Extension *extension, content::BrowserContext *context) const override;
    bool AllowCrossRendererResourceLoad(const network::ResourceRequest &request,
                                        network::mojom::RequestDestination destination,
                                        ui::PageTransition page_transition,
                                        int child_id,
                                        bool is_incognito,
                                        const Extension *extension,
                                        const ExtensionSet &extensions,
                                        const ProcessMap &process_map) override;

    PrefService *GetPrefServiceForContext(content::BrowserContext *context) override;
    void GetEarlyExtensionPrefsObservers(content::BrowserContext *context,
                                         std::vector<EarlyExtensionPrefsObserver *> *observers) const override;
    ProcessManagerDelegate *GetProcessManagerDelegate() const override;
    std::unique_ptr<ExtensionHostDelegate> CreateExtensionHostDelegate() override;
    bool DidVersionUpdate(content::BrowserContext *context) override;
    void PermitExternalProtocolHandler() override;
    bool IsRunningInForcedAppMode() override;
    bool IsLoggedInAsPublicAccount() override;
    ExtensionSystemProvider *GetExtensionSystemFactory() override;
    void RegisterBrowserInterfaceBindersForFrame(mojo::BinderMapWithContext<content::RenderFrameHost*> *,
                                                 content::RenderFrameHost *, const extensions::Extension *) const override;

    std::unique_ptr<RuntimeAPIDelegate> CreateRuntimeAPIDelegate(content::BrowserContext *context) const override;
    const ComponentExtensionResourceManager *
    GetComponentExtensionResourceManager() override;
    void BroadcastEventToRenderers(events::HistogramValue histogram_value,
                                   const std::string &event_name,
                                   base::Value::List args,
                                   bool dispatch_to_off_the_record_profiles) override;
    ExtensionCache *GetExtensionCache() override;
    bool IsBackgroundUpdateAllowed() override;
    bool IsMinBrowserVersionSupported(const std::string &min_version) override;
    ExtensionWebContentsObserver *GetExtensionWebContentsObserver(content::WebContents *web_contents) override;
    KioskDelegate *GetKioskDelegate() override;

    // Whether the browser context is associated with Chrome OS lock screen.
    bool IsLockScreenContext(content::BrowserContext *context) override;

    bool IsAppModeForcedForApp(const ExtensionId &id) override;
    bool IsInDemoMode() override;

    // Return the resource relative path and id for the given request.
    base::FilePath GetBundleResourcePath(const network::ResourceRequest &request,
                                         const base::FilePath &extension_resources_path,
                                         int *resource_id) const override;

    // Creates and starts a URLLoader to load an extension resource from the
    // embedder's resource bundle (.pak) files. Used for component extensions.
    void LoadResourceFromResourceBundle(const network::ResourceRequest &request,
                                        mojo::PendingReceiver<network::mojom::URLLoader> loader,
                                        const base::FilePath &resource_relative_path,
                                        int resource_id,
                                        scoped_refptr<net::HttpResponseHeaders> headers,
                                        mojo::PendingRemote<network::mojom::URLLoaderClient> client) override;

    // Returns the locale used by the application.
    std::string GetApplicationLocale() override;

    bool IsScreensaverInDemoMode(const std::string &app_id) override;

    // Sets the API client.
    void SetAPIClientForTest(ExtensionsAPIClient *api_client);

private:
    // Support for extension APIs.
    std::unique_ptr<ExtensionsAPIClient> api_client_;

    // Resource manager used to supply resources from pak files.
    std::unique_ptr<ComponentExtensionResourceManager> resource_manager_;

    //scoped_refptr<EventRouterForwarder> event_router_forwarder_;
};

} // namespace extensions

#endif // EXTENSIONS_BROWSER_CLIENT_QT_H_
