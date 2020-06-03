/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef CONTENT_BROWSER_CLIENT_QT_H
#define CONTENT_BROWSER_CLIENT_QT_H

#include "qtwebenginecoreglobal_p.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/content_browser_client.h"

namespace net {
class URLRequestContextGetter;
}

namespace content {
class BrowserContext;
class BrowserMainParts;

#if QT_CONFIG(webengine_pepper_plugins)
class BrowserPpapiHost;
#endif

class DevToolsManagerDelegate;
class RenderFrameHost;
class RenderProcessHost;
class RenderViewHostDelegateView;
class ResourceContext;
class ResourceDispatcherHostDelegate;
class WebContentsViewPort;
class WebContents;
struct MainFunctionParams;
struct Referrer;
}

namespace gl {
class GLShareGroup;
}

namespace QtWebEngineCore {

class BrowserMainPartsQt;
class ProfileQt;
class ShareGroupQtQuick;

class ContentBrowserClientQt : public content::ContentBrowserClient
{
public:
    ContentBrowserClientQt();
    ~ContentBrowserClientQt();
    std::unique_ptr<content::BrowserMainParts> CreateBrowserMainParts(const content::MainFunctionParams&) override;
    void RenderProcessWillLaunch(content::RenderProcessHost *host) override;
    gl::GLShareGroup* GetInProcessGpuShareGroup() override;
    content::MediaObserver* GetMediaObserver() override;
    scoped_refptr<content::QuotaPermissionContext> CreateQuotaPermissionContext() override;
    void OverrideWebkitPrefs(content::RenderViewHost *, content::WebPreferences *) override;
    void AllowCertificateError(content::WebContents *web_contents,
                               int cert_error,
                               const net::SSLInfo &ssl_info,
                               const GURL &request_url,
                               bool is_main_frame_request,
                               bool strict_enforcement,
                               base::OnceCallback<void(content::CertificateRequestResultType)> callback) override;
    base::OnceClosure SelectClientCertificate(content::WebContents* web_contents,
                                              net::SSLCertRequestInfo* cert_request_info,
                                              net::ClientCertIdentityList client_certs,
                                              std::unique_ptr<content::ClientCertificateDelegate> delegate) override;
    std::unique_ptr<net::ClientCertStore> CreateClientCertStore(content::BrowserContext *browser_context) override;
    content::DevToolsManagerDelegate *GetDevToolsManagerDelegate() override;
    content::PlatformNotificationService * GetPlatformNotificationService(content::BrowserContext *browser_context) override;

    std::string GetApplicationLocale() override;
    std::string GetAcceptLangs(content::BrowserContext* context) override;
    void AppendExtraCommandLineSwitches(base::CommandLine* command_line, int child_process_id) override;

    void GetAdditionalViewSourceSchemes(std::vector<std::string>* additional_schemes) override;
    void GetAdditionalWebUISchemes(std::vector<std::string>* additional_schemes) override;
    void GetAdditionalAllowedSchemesForFileSystem(std::vector<std::string>* additional_schemes) override;

    void BindInterfaceRequestFromFrame(content::RenderFrameHost* render_frame_host,
                                       const std::string& interface_name,
                                       mojo::ScopedMessagePipeHandle interface_pipe) override;
    void BindHostReceiverForRenderer(content::RenderProcessHost *render_process_host,
                                     mojo::GenericPendingReceiver receiver) override;
    void RegisterBrowserInterfaceBindersForFrame(content::RenderFrameHost *render_frame_host,
                                                 service_manager::BinderMapWithContext<content::RenderFrameHost *> *map) override;
    void RunServiceInstance(const service_manager::Identity &identity,
                            mojo::PendingReceiver<service_manager::mojom::Service> *receiver) override;
    void ExposeInterfacesToRenderer(service_manager::BinderRegistry *registry,
                                    blink::AssociatedInterfaceRegistry *associated_registry,
                                    content::RenderProcessHost *render_process_host) override;

    std::vector<service_manager::Manifest> GetExtraServiceManifests() override;
    base::Optional<service_manager::Manifest> GetServiceManifestOverlay(base::StringPiece name) override;
    bool CanCreateWindow(content::RenderFrameHost *opener,
                         const GURL &opener_url,
                         const GURL &opener_top_level_frame_url,
                         const url::Origin &source_origin,
                         content::mojom::WindowContainerType container_type,
                         const GURL &target_url,
                         const content::Referrer &referrer,
                         const std::string &frame_name,
                         WindowOpenDisposition disposition,
                         const blink::mojom::WindowFeatures &features,
                         bool user_gesture,
                         bool opener_suppressed,
                         bool *no_javascript_access) override;
    bool ShouldEnableStrictSiteIsolation() override;

    bool WillCreateRestrictedCookieManager(
            network::mojom::RestrictedCookieManagerRole role,
            content::BrowserContext *browser_context,
            const url::Origin &origin,
            const net::SiteForCookies &site_for_cookies,
            const url::Origin &top_frame_origin,
            bool is_service_worker,
            int process_id,
            int routing_id,
            mojo::PendingReceiver<network::mojom::RestrictedCookieManager> *receiver) override;

    bool AllowAppCache(const GURL &manifest_url,
                       const GURL &first_party,
                       content::BrowserContext *context) override;

    bool AllowServiceWorkerOnIO(const GURL &scope,
                                const GURL &site_for_cookies,
                                const base::Optional<url::Origin> &top_frame_origin,
                                const GURL &script_url,
                                content::ResourceContext *context,
                                base::RepeatingCallback<content::WebContents*()> wc_getter) override;

    bool AllowServiceWorkerOnUI(const GURL &scope,
                                const GURL &site_for_cookies,
                                const base::Optional<url::Origin> &top_frame_origin,
                                const GURL &script_url,
                                content::BrowserContext *context,
                                base::RepeatingCallback<content::WebContents*()> wc_getter) override;

    void AllowWorkerFileSystem(const GURL &url,
                               content::BrowserContext *context,
                               const std::vector<content::GlobalFrameRoutingId> &render_frames,
                               base::OnceCallback<void(bool)> callback) override;

    bool AllowWorkerIndexedDB(const GURL &url,
                              content::BrowserContext *context,
                              const std::vector<content::GlobalFrameRoutingId> &render_frames) override;

#if QT_CONFIG(webengine_geolocation)
    std::unique_ptr<device::LocationProvider> OverrideSystemLocationProvider() override;
#endif
    bool ShouldIsolateErrorPage(bool in_main_frame) override;
    bool ShouldUseProcessPerSite(content::BrowserContext *browser_context, const GURL &effective_url) override;
    bool DoesSiteRequireDedicatedProcess(content::BrowserContext *browser_context,
                                         const GURL &effective_site_url) override;
    bool ShouldUseSpareRenderProcessHost(content::BrowserContext *browser_context, const GURL& site_url) override;
    bool ShouldTreatURLSchemeAsFirstPartyWhenTopLevel(base::StringPiece scheme,
                                                      bool is_embedded_origin_secure) override;
    void OverrideURLLoaderFactoryParams(content::BrowserContext *browser_context,
                                        const url::Origin &origin,
                                        bool is_for_isolated_world,
                                        network::mojom::URLLoaderFactoryParams *factory_params) override;
#if defined(Q_OS_LINUX)
    void GetAdditionalMappedFilesForChildProcess(const base::CommandLine& command_line, int child_process_id, content::PosixFileDescriptorInfo* mappings) override;
#endif

#if QT_CONFIG(webengine_pepper_plugins)
    void DidCreatePpapiPlugin(content::BrowserPpapiHost* browser_host) override;
#endif

    std::unique_ptr<content::LoginDelegate> CreateLoginDelegate(
            const net::AuthChallengeInfo &auth_info,
            content::WebContents *web_contents,
            const content::GlobalRequestID& request_id,
            bool is_request_for_main_frame,
            const GURL &url,
            scoped_refptr<net::HttpResponseHeaders> response_headers,
            bool first_auth_attempt,
            LoginAuthRequiredCallback auth_required_callback) override;

    bool HandleExternalProtocol(
            const GURL &url,
            base::OnceCallback<content::WebContents*()> web_contents_getter,
            int child_id,
            content::NavigationUIData *navigation_data,
            bool is_main_frame,
            ui::PageTransition page_transition,
            bool has_user_gesture,
            const base::Optional<url::Origin> &initiating_origin,
            mojo::PendingRemote<network::mojom::URLLoaderFactory> *out_factory) override;

    std::vector<std::unique_ptr<blink::URLLoaderThrottle>> CreateURLLoaderThrottles(
            const network::ResourceRequest &request, content::BrowserContext *browser_context,
            const base::RepeatingCallback<content::WebContents *()> &wc_getter,
            content::NavigationUIData *navigation_ui_data, int frame_tree_node_id) override;

    std::vector<std::unique_ptr<content::NavigationThrottle>> CreateThrottlesForNavigation(
            content::NavigationHandle *navigation_handle) override;

    bool IsHandledURL(const GURL &url) override;

    bool WillCreateURLLoaderFactory(content::BrowserContext *browser_context,
                                    content::RenderFrameHost *frame,
                                    int render_process_id,
                                    URLLoaderFactoryType type,
                                    const url::Origin &request_initiator,
                                    base::Optional<int64_t> navigation_id,
                                    mojo::PendingReceiver<network::mojom::URLLoaderFactory> *factory_receiver,
                                    mojo::PendingRemote<network::mojom::TrustedURLLoaderHeaderClient> *header_client,
                                    bool *bypass_redirect_checks,
                                    bool *disable_secure_dns,
                                    network::mojom::URLLoaderFactoryOverridePtr *factory_override) override;
    scoped_refptr<network::SharedURLLoaderFactory> GetSystemSharedURLLoaderFactory() override;
    network::mojom::NetworkContext *GetSystemNetworkContext() override;
    void OnNetworkServiceCreated(network::mojom::NetworkService *network_service) override;
    mojo::Remote<network::mojom::NetworkContext> CreateNetworkContext(content::BrowserContext *context,
                                                                      bool in_memory,
                                                                      const base::FilePath &relative_partition_path) override;
    std::vector<base::FilePath> GetNetworkContextsParentDirectory() override;
    void RegisterNonNetworkNavigationURLLoaderFactories(int frame_tree_node_id, NonNetworkURLLoaderFactoryMap *factories) override;
    void RegisterNonNetworkSubresourceURLLoaderFactories(int render_process_id, int render_frame_id,
                                                         NonNetworkURLLoaderFactoryMap* factories) override;
    void RegisterNonNetworkWorkerMainResourceURLLoaderFactories(content::BrowserContext* browser_context,
                                                                NonNetworkURLLoaderFactoryMap* factories) override;

    static std::string getUserAgent();

    std::string GetUserAgent() override { return getUserAgent(); }
    std::string GetProduct() override;

private:
    void InitFrameInterfaces();

    scoped_refptr<ShareGroupQtQuick> m_shareGroupQtQuick;
    std::unique_ptr<service_manager::BinderRegistry> m_frameInterfaces;
    std::unique_ptr<service_manager::BinderRegistryWithArgs<content::RenderFrameHost*>> m_frameInterfacesParameterized;
};

} // namespace QtWebEngineCore

#endif // CONTENT_BROWSER_CLIENT_QT_H
