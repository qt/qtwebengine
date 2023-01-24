// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CONTENT_BROWSER_CLIENT_QT_H
#define CONTENT_BROWSER_CLIENT_QT_H

#include "qtwebenginecoreglobal_p.h"
#include "content/public/browser/content_browser_client.h"

namespace content {
class BrowserContext;
class BrowserMainParts;

#if QT_CONFIG(webengine_pepper_plugins)
class BrowserPpapiHost;
#endif

class DevToolsManagerDelegate;
class RenderFrameHost;
class RenderProcessHost;
class ResourceContext;
class WebContents;
struct MainFunctionParams;
struct Referrer;
} // namespace content

namespace device {
class GeolocationManager;
} // namespace device

namespace QtWebEngineCore {

class BrowserMainPartsQt;

class ContentBrowserClientQt : public content::ContentBrowserClient
{
public:
    ContentBrowserClientQt();
    ~ContentBrowserClientQt();
    std::unique_ptr<content::BrowserMainParts> CreateBrowserMainParts(bool is_integration_test) override;
    void RenderProcessWillLaunch(content::RenderProcessHost *host) override;
    content::MediaObserver* GetMediaObserver() override;
    void OverrideWebkitPrefs(content::WebContents *web_contents,
                             blink::web_pref::WebPreferences *prefs) override;
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
    std::unique_ptr<content::DevToolsManagerDelegate> CreateDevToolsManagerDelegate() override;

    std::string GetApplicationLocale() override;
    std::string GetAcceptLangs(content::BrowserContext* context) override;
    void AppendExtraCommandLineSwitches(base::CommandLine* command_line, int child_process_id) override;

    void GetAdditionalViewSourceSchemes(std::vector<std::string>* additional_schemes) override;
    void GetAdditionalWebUISchemes(std::vector<std::string>* additional_schemes) override;
    void GetAdditionalAllowedSchemesForFileSystem(std::vector<std::string>* additional_schemes) override;

    std::unique_ptr<ui::SelectFilePolicy>
    CreateSelectFilePolicy(content::WebContents *web_contents) override;
    void BindHostReceiverForRenderer(content::RenderProcessHost *render_process_host,
                                     mojo::GenericPendingReceiver receiver) override;
    void RegisterBrowserInterfaceBindersForFrame(content::RenderFrameHost *render_frame_host,
                                                 mojo::BinderMapWithContext<content::RenderFrameHost *> *map) override;
    void ExposeInterfacesToRenderer(service_manager::BinderRegistry *registry,
                                    blink::AssociatedInterfaceRegistry *associated_registry,
                                    content::RenderProcessHost *render_process_host) override;
    void RegisterAssociatedInterfaceBindersForRenderFrameHost(content::RenderFrameHost &render_frame_host,
                                                              blink::AssociatedInterfaceRegistry &associated_registry) override;

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
            const net::IsolationInfo &isolation_info,
            bool is_service_worker,
            int process_id,
            int routing_id,
            mojo::PendingReceiver<network::mojom::RestrictedCookieManager> *receiver) override;
    bool WillInterceptWebSocket(content::RenderFrameHost *frame) override;
    void CreateWebSocket(
            content::RenderFrameHost *frame,
            WebSocketFactory factory,
            const GURL &url,
            const net::SiteForCookies &site_for_cookies,
            const absl::optional<std::string> &user_agent,
            mojo::PendingRemote<network::mojom::WebSocketHandshakeClient> handshake_client) override;

    content::AllowServiceWorkerResult AllowServiceWorker(
            const GURL &scope,
            const net::SiteForCookies &site_for_cookies,
            const absl::optional<url::Origin> &top_frame_origin,
            const GURL &script_url,
            content::BrowserContext *context) override;

    void AllowWorkerFileSystem(const GURL &url,
                               content::BrowserContext *context,
                               const std::vector<content::GlobalRenderFrameHostId> &render_frames,
                               base::OnceCallback<void(bool)> callback) override;

    bool AllowWorkerIndexedDB(const GURL &url,
                              content::BrowserContext *context,
                              const std::vector<content::GlobalRenderFrameHostId> &render_frames) override;
    AllowWebBluetoothResult AllowWebBluetooth(content::BrowserContext *browser_context,
                                              const url::Origin &requesting_origin,
                                              const url::Origin &embedding_origin) override;

#if QT_CONFIG(webengine_geolocation)
    std::unique_ptr<device::LocationProvider> OverrideSystemLocationProvider() override;
#endif
    device::GeolocationManager *GetGeolocationManager() override;

    bool ShouldIsolateErrorPage(bool in_main_frame) override;
    bool ShouldUseProcessPerSite(content::BrowserContext *browser_context, const GURL &effective_url) override;
    bool DoesSiteRequireDedicatedProcess(content::BrowserContext *browser_context,
                                         const GURL &effective_site_url) override;
    bool ShouldUseSpareRenderProcessHost(content::BrowserContext *browser_context, const GURL& site_url) override;
    bool ShouldTreatURLSchemeAsFirstPartyWhenTopLevel(base::StringPiece scheme,
                                                      bool is_embedded_origin_secure) override;
    bool DoesSchemeAllowCrossOriginSharedWorker(const std::string &scheme) override;
    void OverrideURLLoaderFactoryParams(content::BrowserContext *browser_context,
                                        const url::Origin &origin,
                                        bool is_for_isolated_world,
                                        network::mojom::URLLoaderFactoryParams *factory_params) override;
#if defined(Q_OS_LINUX)
    void GetAdditionalMappedFilesForChildProcess(const base::CommandLine& command_line, int child_process_id, content::PosixFileDescriptorInfo* mappings) override;
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
            base::RepeatingCallback<content::WebContents*()> web_contents_getter,
            int frame_tree_node_id,
            content::NavigationUIData *navigation_data,
            bool is_primary_main_frame,
            bool is_in_fenced_frame_tree,
            network::mojom::WebSandboxFlags sandbox_flags,
            ui::PageTransition page_transition,
            bool has_user_gesture,
            const absl::optional<url::Origin> &initiating_origin,
            content::RenderFrameHost *initiator_document,
            mojo::PendingRemote<network::mojom::URLLoaderFactory> *out_factory) override;

    std::vector<std::unique_ptr<blink::URLLoaderThrottle>> CreateURLLoaderThrottles(
            const network::ResourceRequest &request, content::BrowserContext *browser_context,
            const base::RepeatingCallback<content::WebContents *()> &wc_getter,
            content::NavigationUIData *navigation_ui_data, int frame_tree_node_id) override;

    std::vector<std::unique_ptr<content::NavigationThrottle>> CreateThrottlesForNavigation(
            content::NavigationHandle *navigation_handle) override;

    bool IsHandledURL(const GURL &url) override;
    bool HasErrorPage(int http_status_code, content::WebContents *contents) override;
    bool HasCustomSchemeHandler(content::BrowserContext *browser_context,
                                const std::string &scheme) override;
    std::vector<std::unique_ptr<content::URLLoaderRequestInterceptor>>
    WillCreateURLLoaderRequestInterceptors(content::NavigationUIData *navigation_ui_data,
                                           int frame_tree_node_id) override;
    bool WillCreateURLLoaderFactory(content::BrowserContext *browser_context,
                                    content::RenderFrameHost *frame,
                                    int render_process_id,
                                    URLLoaderFactoryType type,
                                    const url::Origin &request_initiator,
                                    absl::optional<int64_t> navigation_id,
                                    ukm::SourceIdObj ukm_source_id,
                                    mojo::PendingReceiver<network::mojom::URLLoaderFactory> *factory_receiver,
                                    mojo::PendingRemote<network::mojom::TrustedURLLoaderHeaderClient> *header_client,
                                    bool *bypass_redirect_checks,
                                    bool *disable_secure_dns,
                                    network::mojom::URLLoaderFactoryOverridePtr *factory_override) override;
    scoped_refptr<network::SharedURLLoaderFactory> GetSystemSharedURLLoaderFactory() override;
    network::mojom::NetworkContext *GetSystemNetworkContext() override;
    void OnNetworkServiceCreated(network::mojom::NetworkService *network_service) override;
    void ConfigureNetworkContextParams(content::BrowserContext *context,
                                       bool in_memory,
                                       const base::FilePath &relative_partition_path,
                                       network::mojom::NetworkContextParams *network_context_params,
                                       cert_verifier::mojom::CertVerifierCreationParams *cert_verifier_creation_params) override;

    std::vector<base::FilePath> GetNetworkContextsParentDirectory() override;
    void RegisterNonNetworkNavigationURLLoaderFactories(int frame_tree_node_id,
                                                        ukm::SourceIdObj ukm_source_id,
                                                        NonNetworkURLLoaderFactoryMap *factories) override;
    void RegisterNonNetworkSubresourceURLLoaderFactories(int render_process_id, int render_frame_id,
                                                         const absl::optional<url::Origin>& request_initiator_origin,
                                                         NonNetworkURLLoaderFactoryMap *factories) override;
    void RegisterNonNetworkWorkerMainResourceURLLoaderFactories(content::BrowserContext* browser_context,
                                                                NonNetworkURLLoaderFactoryMap* factories) override;
    void RegisterNonNetworkServiceWorkerUpdateURLLoaderFactories(content::BrowserContext* browser_context,
                                                                 NonNetworkURLLoaderFactoryMap* factories) override;
    void SiteInstanceGotProcess(content::SiteInstance *site_instance) override;
    void SiteInstanceDeleting(content::SiteInstance *site_instance) override;
    base::flat_set<std::string> GetPluginMimeTypesWithExternalHandlers(content::BrowserContext *browser_context) override;

    std::unique_ptr<content::WebContentsViewDelegate> GetWebContentsViewDelegate(content::WebContents *web_contents) override;

    static std::string getUserAgent();
    static blink::UserAgentMetadata getUserAgentMetadata();

    std::string GetUserAgent() override { return getUserAgent(); }
    blink::UserAgentMetadata GetUserAgentMetadata() override { return getUserAgentMetadata(); }
    std::string GetProduct() override;

private:
    BrowserMainPartsQt *m_browserMainParts = nullptr;
};

} // namespace QtWebEngineCore

#endif // CONTENT_BROWSER_CLIENT_QT_H
