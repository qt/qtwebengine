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

// Portions copyright 2015 The Chromium Embedded Framework Authors.
// Portions copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_CLIENT_QT_H_
#define EXTENSIONS_BROWSER_CLIENT_QT_H_

#include "base/compiler_specific.h"
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
    bool IsGuestSession(content::BrowserContext *context) const override;
    bool IsExtensionIncognitoEnabled(const std::string &extension_id, content::BrowserContext *context) const override;
    bool CanExtensionCrossIncognito(const Extension *extension, content::BrowserContext *context) const override;
    bool AllowCrossRendererResourceLoad(const GURL &url,
                                        blink::mojom::ResourceType resource_type,
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
    void RegisterBrowserInterfaceBindersForFrame(service_manager::BinderMapWithContext<content::RenderFrameHost*> *,
                                                 content::RenderFrameHost *, const extensions::Extension *) const override;
    std::unique_ptr<RuntimeAPIDelegate> CreateRuntimeAPIDelegate(content::BrowserContext *context) const override;
    const ComponentExtensionResourceManager *
    GetComponentExtensionResourceManager() override;
    void BroadcastEventToRenderers(events::HistogramValue histogram_value,
                                   const std::string &event_name,
                                   std::unique_ptr<base::ListValue> args,
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
                                        const std::string &content_security_policy,
                                        mojo::PendingRemote<network::mojom::URLLoaderClient> client,
                                        bool send_cors_header) override;

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

    DISALLOW_COPY_AND_ASSIGN(ExtensionsBrowserClientQt);
};

} // namespace extensions

#endif // EXTENSIONS_BROWSER_CLIENT_QT_H_
