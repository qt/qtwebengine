// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "content_browser_client_qt.h"

#include "base/files/file_util.h"
#include "chrome/browser/tab_contents/form_interaction_tab_helper.h"
#include "components/autofill/content/browser/content_autofill_driver_factory.h"
#include "components/custom_handlers/protocol_handler_registry.h"
#include "components/embedder_support/user_agent_utils.h"
#include "components/error_page/common/error.h"
#include "components/error_page/common/localized_error.h"
#include "components/navigation_interception/intercept_navigation_throttle.h"
#include "components/network_hints/browser/simple_network_hints_handler_impl.h"
#include "components/performance_manager/embedder/performance_manager_lifetime.h"
#include "components/performance_manager/embedder/performance_manager_registry.h"
#include "components/performance_manager/public/performance_manager.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/child_process_security_policy.h"
#include "content/public/browser/client_certificate_delegate.h"
#include "content/public/browser/file_url_loader.h"
#include "content/public/browser/media_observer.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/shared_cors_origin_access_list.h"
#include "content/public/browser/url_loader_request_interceptor.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_user_data.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/public/browser/web_ui_url_loader_factory.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/user_agent.h"
#include "extensions/buildflags/buildflags.h"
#include "mojo/public/cpp/bindings/self_owned_associated_receiver.h"
#include "pdf/buildflags.h"
#include "net/ssl/client_cert_identity.h"
#include "net/ssl/client_cert_store.h"
#include "net/ssl/ssl_private_key.h"
#include "printing/buildflags/buildflags.h"
#include "services/device/public/cpp/geolocation/geolocation_manager.h"
#include "services/network/network_service.h"
#include "services/network/public/cpp/web_sandbox_flags.h"
#include "services/network/public/mojom/websocket.mojom.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_switches.h"
#include "url/url_util_qt.h"

#include "qtwebengine/common/renderer_configuration.mojom.h"

#include "profile_adapter.h"
#include "browser_main_parts_qt.h"
#include "browser_message_filter_qt.h"
#include "certificate_error_controller.h"
#include "client_cert_select_controller.h"
#include "custom_handlers/protocol_handler_registry_factory.h"
#include "devtools_manager_delegate_qt.h"
#include "file_system_access/file_system_access_permission_request_manager_qt.h"
#include "login_delegate_qt.h"
#include "media_capture_devices_dispatcher.h"
#include "net/cookie_monster_delegate_qt.h"
#include "net/custom_url_loader_factory.h"
#include "net/proxying_restricted_cookie_manager_qt.h"
#include "net/proxying_url_loader_factory_qt.h"
#include "net/system_network_context_manager.h"
#include "platform_notification_service_qt.h"
#include "profile_qt.h"
#include "profile_io_data_qt.h"
#include "renderer_host/user_resource_controller_host.h"
#include "select_file_dialog_factory_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_adapter.h"
#include "web_contents_delegate_qt.h"
#include "web_contents_view_qt.h"
#include "web_engine_context.h"
#include "web_engine_library_info.h"
#include "web_engine_settings.h"
#include "api/qwebenginecookiestore.h"
#include "api/qwebenginecookiestore_p.h"
#include "api/qwebengineurlrequestinfo_p.h"

#if QT_CONFIG(webengine_geolocation)
#include "base/memory/ptr_util.h"
#include "location_provider_qt.h"
#endif

#if QT_CONFIG(webengine_spellchecker)
#include "chrome/browser/spellchecker/spell_check_host_chrome_impl.h"
#include "chrome/browser/spellchecker/spellcheck_factory.h"
#include "chrome/browser/spellchecker/spellcheck_service.h"
#include "components/spellcheck/browser/pref_names.h"
#include "components/spellcheck/common/spellcheck.mojom.h"
#include "components/spellcheck/common/spellcheck_features.h"
#endif

#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
#include "chrome/browser/media/webrtc/webrtc_logging_controller.h"
#endif

#if defined(Q_OS_LINUX)
#include "global_descriptors_qt.h"
#endif

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "common/extensions/extensions_client_qt.h"
#include "components/guest_view/browser/guest_view_base.h"
#include "extensions/browser/api/messaging/messaging_api_message_filter.h"
#include "extensions/browser/api/mime_handler_private/mime_handler_private.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_message_filter.h"
#include "extensions/browser/extension_protocols.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/guest_view/extensions_guest_view.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest.h"
#include "extensions/browser/guest_view/web_view/web_view_guest.h"
#include "extensions/browser/process_map.h"
#include "extensions/browser/url_loader_factory_manager.h"
#include "extensions/browser/view_type_utils.h"
#include "extensions/common/constants.h"
#include "extensions/common/manifest_handlers/mime_types_handler.h"
#include "extensions/extension_web_contents_observer_qt.h"
#include "extensions/extensions_browser_client_qt.h"
#include "net/plugin_response_interceptor_url_loader_throttle.h"
#endif

#if QT_CONFIG(webengine_webchannel)
#include "qtwebengine/browser/qtwebchannel.mojom.h"
#include "renderer_host/web_channel_ipc_transport_host.h"
#endif

#if BUILDFLAG(ENABLE_PRINTING) && BUILDFLAG(ENABLE_PRINT_PREVIEW)
#include "printing/pdf_stream_delegate_qt.h"
#include "printing/print_view_manager_qt.h"
#endif

#if BUILDFLAG(ENABLE_PDF)
#include "components/pdf/browser/pdf_navigation_throttle.h"
#include "components/pdf/browser/pdf_url_loader_request_interceptor.h"
#include "components/pdf/browser/pdf_web_contents_helper.h"
#endif

#if BUILDFLAG(ENABLE_PDF) && BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/pdf_iframe_navigation_throttle_qt.h"
#endif

#include <QGuiApplication>
#include <QStandardPaths>

// Implement IsHandledProtocol as declared in //url/url_util_qt.h.
namespace url {
bool IsHandledProtocol(base::StringPiece scheme)
{
    static const char *const kProtocolList[] = {
        url::kHttpScheme,
        url::kHttpsScheme,
#if BUILDFLAG(ENABLE_WEBSOCKETS)
        url::kWsScheme,
        url::kWssScheme,
#endif  // BUILDFLAG(ENABLE_WEBSOCKETS)
        url::kFileScheme,
        content::kChromeDevToolsScheme,
#if BUILDFLAG(ENABLE_EXTENSIONS)
        extensions::kExtensionScheme,
#endif
        content::kChromeUIScheme,
        url::kDataScheme,
        url::kAboutScheme,
        url::kBlobScheme,
        url::kFileSystemScheme,
        url::kQrcScheme,
    };

    for (const char *protocol : kProtocolList) {
        if (scheme == protocol)
            return true;
    }
    if (const auto cs = url::CustomScheme::FindScheme(scheme))
        return true;
    return false;
}
}

namespace QtWebEngineCore {

void MaybeAddThrottle(
    std::unique_ptr<content::NavigationThrottle> maybe_throttle,
    std::vector<std::unique_ptr<content::NavigationThrottle>>* throttles) {
    if (maybe_throttle)
        throttles->push_back(std::move(maybe_throttle));
}

ContentBrowserClientQt::ContentBrowserClientQt()
{
}

ContentBrowserClientQt::~ContentBrowserClientQt()
{
}

std::unique_ptr<content::BrowserMainParts> ContentBrowserClientQt::CreateBrowserMainParts(bool)
{
    Q_ASSERT(!m_browserMainParts);
    auto browserMainParts = std::make_unique<BrowserMainPartsQt>();
    m_browserMainParts = browserMainParts.get();
    return browserMainParts;
}

void ContentBrowserClientQt::RenderProcessWillLaunch(content::RenderProcessHost *host)
{
    const int id = host->GetID();
    Profile *profile = Profile::FromBrowserContext(host->GetBrowserContext());

#if QT_CONFIG(webengine_spellchecker)
    if (spellcheck::UseBrowserSpellChecker() && !profile->GetPrefs()->GetBoolean(spellcheck::prefs::kSpellCheckEnable))
        SpellcheckServiceFactory::GetForContext(profile)->InitForRenderer(host);
#endif

#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
    WebRtcLoggingController::AttachToRenderProcessHost(host, WebEngineContext::current()->webRtcLogUploader());
#endif

    // Allow requesting custom schemes.
    const auto policy = content::ChildProcessSecurityPolicy::GetInstance();
    const auto profileAdapter = static_cast<ProfileQt *>(profile)->profileAdapter();
    for (const QByteArray &scheme : profileAdapter->customUrlSchemes())
        policy->GrantRequestScheme(id, scheme.toStdString());

    // FIXME: Add a settings variable to enable/disable the file scheme.
    policy->GrantRequestScheme(id, url::kFileScheme);
    profileAdapter->userResourceController()->renderProcessStartedWithHost(host);
    host->AddFilter(new BrowserMessageFilterQt(id, profile));
#if BUILDFLAG(ENABLE_EXTENSIONS)
    host->AddFilter(new extensions::ExtensionMessageFilter(id, profile));
    host->AddFilter(new extensions::MessagingAPIMessageFilter(id, profile));
#endif //ENABLE_EXTENSIONS

    bool is_incognito_process = profile->IsOffTheRecord();
    mojo::AssociatedRemote<qtwebengine::mojom::RendererConfiguration> renderer_configuration;
    host->GetChannel()->GetRemoteAssociatedInterface(&renderer_configuration);
    renderer_configuration->SetInitialConfiguration(is_incognito_process);
}

content::MediaObserver *ContentBrowserClientQt::GetMediaObserver()
{
    return MediaCaptureDevicesDispatcher::GetInstance();
}

void ContentBrowserClientQt::OverrideWebkitPrefs(content::WebContents *webContents, blink::web_pref::WebPreferences *web_prefs)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    if (guest_view::GuestViewBase::IsGuest(webContents))
        return;

    WebContentsViewQt *view = WebContentsViewQt::from(static_cast<content::WebContentsImpl *>(webContents)->GetView());
    if (!view->client())
        return;
#endif // BUILDFLAG(ENABLE_EXTENSIONS)
    WebContentsDelegateQt* delegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());
    if (delegate)
        delegate->overrideWebPreferences(webContents, web_prefs);
}

void ContentBrowserClientQt::AllowCertificateError(content::WebContents *webContents,
                                                   int cert_error,
                                                   const net::SSLInfo &ssl_info,
                                                   const GURL &request_url,
                                                   bool /* is_main_frame_request */,
                                                   bool strict_enforcement,
                                                   base::OnceCallback<void(content::CertificateRequestResultType)> callback)
{
    WebContentsDelegateQt* contentsDelegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());

    QSharedPointer<CertificateErrorController> errorController(new CertificateErrorController(
            cert_error, ssl_info, request_url, strict_enforcement, std::move(callback)));
    contentsDelegate->allowCertificateError(errorController);
}


base::OnceClosure ContentBrowserClientQt::SelectClientCertificate(content::WebContents *webContents,
                                                                  net::SSLCertRequestInfo *certRequestInfo,
                                                                  net::ClientCertIdentityList clientCerts,
                                                                  std::unique_ptr<content::ClientCertificateDelegate> delegate)
{
    if (!clientCerts.empty()) {
        WebContentsDelegateQt* contentsDelegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());

        QSharedPointer<ClientCertSelectController> certSelectController(
                new ClientCertSelectController(certRequestInfo, std::move(clientCerts), std::move(delegate)));

        contentsDelegate->selectClientCert(certSelectController);
    } else {
        delegate->ContinueWithCertificate(nullptr, nullptr);
    }
    // This is consistent with AwContentBrowserClient and CastContentBrowserClient:
    return base::OnceClosure();
}

std::unique_ptr<net::ClientCertStore> ContentBrowserClientQt::CreateClientCertStore(content::BrowserContext *browser_context)
{
    if (!browser_context)
        return nullptr;

    return ProfileIODataQt::FromBrowserContext(browser_context)->CreateClientCertStore();
}

std::string ContentBrowserClientQt::GetApplicationLocale()
{
    return WebEngineLibraryInfo::getApplicationLocale();
}

std::string ContentBrowserClientQt::GetAcceptLangs(content::BrowserContext *context)
{
    return static_cast<ProfileQt*>(context)->profileAdapter()->httpAcceptLanguage().toStdString();
}

void ContentBrowserClientQt::AppendExtraCommandLineSwitches(base::CommandLine* command_line, int child_process_id)
{
    Q_UNUSED(child_process_id);

    url::CustomScheme::SaveSchemes(command_line);

    std::string processType = command_line->GetSwitchValueASCII(switches::kProcessType);
    if (processType == switches::kZygoteProcess)
        command_line->AppendSwitchASCII(switches::kLang, WebEngineLibraryInfo::getApplicationLocale());
}

void ContentBrowserClientQt::GetAdditionalWebUISchemes(std::vector<std::string>* additional_schemes)
{
    ContentBrowserClient::GetAdditionalWebUISchemes(additional_schemes);
    additional_schemes->push_back(content::kChromeDevToolsScheme);
}

void ContentBrowserClientQt::GetAdditionalViewSourceSchemes(std::vector<std::string>* additional_schemes)
{
    ContentBrowserClient::GetAdditionalViewSourceSchemes(additional_schemes);

#if BUILDFLAG(ENABLE_EXTENSIONS)
    additional_schemes->push_back(extensions::kExtensionScheme);
#endif
}

void ContentBrowserClientQt::GetAdditionalAllowedSchemesForFileSystem(std::vector<std::string>* additional_schemes)
{
    ContentBrowserClient::GetAdditionalAllowedSchemesForFileSystem(additional_schemes);
    additional_schemes->push_back(content::kChromeDevToolsScheme);
    additional_schemes->push_back(content::kChromeUIScheme);
}

std::unique_ptr<ui::SelectFilePolicy>
ContentBrowserClientQt::CreateSelectFilePolicy(content::WebContents *web_contents)
{
    return std::make_unique<SelectFilePolicyQt>(web_contents);
}

#if defined(Q_OS_LINUX)
void ContentBrowserClientQt::GetAdditionalMappedFilesForChildProcess(const base::CommandLine& command_line, int child_process_id, content::PosixFileDescriptorInfo* mappings)
{
    const base::FilePath &locale_file_path = ui::ResourceBundle::GetSharedInstance().GetLocaleFilePath(WebEngineLibraryInfo::getResolvedLocale());
    if (locale_file_path.empty() || !base::PathExists(locale_file_path))
        return;

    // Open pak file of the current locale in the Browser process and pass its file descriptor to the sandboxed
    // Renderer Process. FileDescriptorInfo is responsible for closing the file descriptor.
    int flags = base::File::FLAG_OPEN | base::File::FLAG_READ;
    base::File locale_file = base::File(locale_file_path, flags);
    mappings->Transfer(kWebEngineLocale, base::ScopedFD(locale_file.TakePlatformFile()));
}
#endif

std::unique_ptr<content::DevToolsManagerDelegate> ContentBrowserClientQt::CreateDevToolsManagerDelegate()
{
    return std::make_unique<DevToolsManagerDelegateQt>();
}

void ContentBrowserClientQt::BindHostReceiverForRenderer(content::RenderProcessHost *render_process_host,
                                                         mojo::GenericPendingReceiver receiver)
{
#if QT_CONFIG(webengine_spellchecker)
    if (auto host_receiver = receiver.As<spellcheck::mojom::SpellCheckHost>()) {
        SpellCheckHostChromeImpl::Create(render_process_host->GetID(), std::move(host_receiver));
        return;
    }
#endif
}

static void BindNetworkHintsHandler(content::RenderFrameHost *frame_host,
                                    mojo::PendingReceiver<network_hints::mojom::NetworkHintsHandler> receiver)
{
    network_hints::SimpleNetworkHintsHandlerImpl::Create(frame_host, std::move(receiver));
}

#if BUILDFLAG(ENABLE_EXTENSIONS)
static void BindMimeHandlerService(content::RenderFrameHost *frame_host,
                                   mojo::PendingReceiver<extensions::mime_handler::MimeHandlerService>
                                   receiver) {
    auto *web_contents = content::WebContents::FromRenderFrameHost(frame_host);
    if (!web_contents)
        return;
    auto *guest_view = extensions::MimeHandlerViewGuest::FromWebContents(web_contents);
    if (!guest_view)
        return;
    extensions::MimeHandlerServiceImpl::Create(guest_view->GetStreamWeakPtr(), std::move(receiver));
}

static void BindBeforeUnloadControl(content::RenderFrameHost *frame_host,
                                    mojo::PendingReceiver<extensions::mime_handler::BeforeUnloadControl>
                                    receiver) {
    auto *web_contents = content::WebContents::FromRenderFrameHost(frame_host);
    if (!web_contents)
        return;
    auto *guest_view = extensions::MimeHandlerViewGuest::FromWebContents(web_contents);
    if (!guest_view)
        return;
    guest_view->FuseBeforeUnloadControl(std::move(receiver));
}
#endif

void ContentBrowserClientQt::RegisterBrowserInterfaceBindersForFrame(
        content::RenderFrameHost *render_frame_host,
        mojo::BinderMapWithContext<content::RenderFrameHost *> *map)
{
    map->Add<network_hints::mojom::NetworkHintsHandler>(base::BindRepeating(&BindNetworkHintsHandler));
#if BUILDFLAG(ENABLE_EXTENSIONS)
    map->Add<extensions::mime_handler::MimeHandlerService>(base::BindRepeating(&BindMimeHandlerService));
    map->Add<extensions::mime_handler::BeforeUnloadControl>(base::BindRepeating(&BindBeforeUnloadControl));
    const GURL &site = render_frame_host->GetSiteInstance()->GetSiteURL();
    if (!site.SchemeIs(extensions::kExtensionScheme))
        return;
    content::BrowserContext *browser_context = render_frame_host->GetProcess()->GetBrowserContext();
    auto *extension = extensions::ExtensionRegistry::Get(browser_context)
                        ->enabled_extensions()
                        .GetByID(site.host());
    if (!extension)
        return;
    extensions::ExtensionsBrowserClient::Get()->RegisterBrowserInterfaceBindersForFrame(map,
                                                                                        render_frame_host,
                                                                                        extension);
#else
    Q_UNUSED(render_frame_host);
#endif
}

void ContentBrowserClientQt::ExposeInterfacesToRenderer(service_manager::BinderRegistry *registry,
                                                        blink::AssociatedInterfaceRegistry *associated_registry,
                                                        content::RenderProcessHost *render_process_host)
{
    if (auto *manager = performance_manager::PerformanceManagerRegistry::GetInstance())
        manager->CreateProcessNodeAndExposeInterfacesToRendererProcess(registry, render_process_host);
#if BUILDFLAG(ENABLE_EXTENSIONS)
    associated_registry->AddInterface<extensions::mojom::EventRouter>(
                base::BindRepeating(&extensions::EventRouter::BindForRenderer, render_process_host->GetID()));
    associated_registry->AddInterface<guest_view::mojom::GuestViewHost>(
                base::BindRepeating(&extensions::ExtensionsGuestView::CreateForComponents, render_process_host->GetID()));
    associated_registry->AddInterface<extensions::mojom::GuestView>(
                base::BindRepeating(&extensions::ExtensionsGuestView::CreateForExtensions, render_process_host->GetID()));
#else
    Q_UNUSED(associated_registry);
#endif
}

void ContentBrowserClientQt::RegisterAssociatedInterfaceBindersForRenderFrameHost(
        content::RenderFrameHost &rfh,
        blink::AssociatedInterfaceRegistry &associated_registry)
{
#if QT_CONFIG(webengine_webchannel)
    associated_registry.AddInterface<qtwebchannel::mojom::WebChannelTransportHost>(
                base::BindRepeating(
                    [](content::RenderFrameHost *render_frame_host,
                       mojo::PendingAssociatedReceiver<qtwebchannel::mojom::WebChannelTransportHost> receiver) {
                        auto *web_contents = content::WebContents::FromRenderFrameHost(render_frame_host);
                        auto *adapter = static_cast<WebContentsDelegateQt *>(web_contents->GetDelegate())->webContentsAdapter();
                        adapter->webChannelTransport()->BindReceiver(std::move(receiver), render_frame_host);
                    }, &rfh));
#endif
#if BUILDFLAG(ENABLE_PRINTING) && BUILDFLAG(ENABLE_PRINT_PREVIEW)
    associated_registry.AddInterface<printing::mojom::PrintManagerHost>(
                base::BindRepeating(
                    [](content::RenderFrameHost* render_frame_host,
                       mojo::PendingAssociatedReceiver<printing::mojom::PrintManagerHost> receiver) {
                        PrintViewManagerQt::BindPrintManagerHost(std::move(receiver), render_frame_host);
                    }, &rfh));
#endif
#if BUILDFLAG(ENABLE_EXTENSIONS)
    associated_registry.AddInterface<extensions::mojom::LocalFrameHost>(
                base::BindRepeating(
                    [](content::RenderFrameHost *render_frame_host,
                       mojo::PendingAssociatedReceiver<extensions::mojom::LocalFrameHost> receiver) {
                        extensions::ExtensionWebContentsObserverQt::BindLocalFrameHost(std::move(receiver), render_frame_host);
                    }, &rfh));
#endif
    associated_registry.AddInterface<autofill::mojom::AutofillDriver>(
                base::BindRepeating(
                    [](content::RenderFrameHost *render_frame_host,
                       mojo::PendingAssociatedReceiver<autofill::mojom::AutofillDriver> receiver) {
                        autofill::ContentAutofillDriverFactory::BindAutofillDriver(std::move(receiver), render_frame_host);
                    }, &rfh));
#if BUILDFLAG(ENABLE_PDF)
    associated_registry.AddInterface<pdf::mojom::PdfService>(
                base::BindRepeating(
                    [](content::RenderFrameHost *render_frame_host,
                       mojo::PendingAssociatedReceiver<pdf::mojom::PdfService> receiver) {
                        pdf::PDFWebContentsHelper::BindPdfService(std::move(receiver), render_frame_host);
                    }, &rfh));
#endif  // BUILDFLAG(ENABLE_PDF)
    ContentBrowserClient::RegisterAssociatedInterfaceBindersForRenderFrameHost(rfh, associated_registry);
}

bool ContentBrowserClientQt::CanCreateWindow(
        content::RenderFrameHost* opener,
        const GURL& opener_url,
        const GURL& opener_top_level_frame_url,
        const url::Origin& source_origin,
        content::mojom::WindowContainerType container_type,
        const GURL& target_url,
        const content::Referrer& referrer,
        const std::string& frame_name,
        WindowOpenDisposition disposition,
        const blink::mojom::WindowFeatures& features,
        bool user_gesture,
        bool opener_suppressed,
        bool* no_javascript_access)
{
    Q_UNUSED(opener_url);
    Q_UNUSED(opener_top_level_frame_url);
    Q_UNUSED(source_origin);
    Q_UNUSED(container_type);
    Q_UNUSED(target_url);
    Q_UNUSED(referrer);
    Q_UNUSED(frame_name);
    Q_UNUSED(disposition);
    Q_UNUSED(features);
    Q_UNUSED(opener_suppressed);

    if (no_javascript_access)
        *no_javascript_access = false;

    content::WebContents* webContents = content::WebContents::FromRenderFrameHost(opener);

    WebEngineSettings *settings = nullptr;
    if (webContents) {
        WebContentsDelegateQt* delegate =
                static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());
        if (delegate)
            settings = delegate->webEngineSettings();
    }

    return (settings && settings->getJavaScriptCanOpenWindowsAutomatically()) || user_gesture;
}

#if QT_CONFIG(webengine_geolocation)
std::unique_ptr<device::LocationProvider> ContentBrowserClientQt::OverrideSystemLocationProvider()
{
    return base::WrapUnique(new LocationProviderQt());
}
#endif

device::GeolocationManager *ContentBrowserClientQt::GetGeolocationManager()
{
#if BUILDFLAG(IS_MAC)
    return m_browserMainParts->GetGeolocationManager();
#else
    return nullptr;
#endif
}

bool ContentBrowserClientQt::ShouldEnableStrictSiteIsolation()
{
    // mirroring AwContentBrowserClient, CastContentBrowserClient and
    // HeadlessContentBrowserClient
    return false;
}

bool ContentBrowserClientQt::WillCreateRestrictedCookieManager(network::mojom::RestrictedCookieManagerRole role,
        content::BrowserContext *browser_context,
        const url::Origin & /*origin*/,
        const net::IsolationInfo & /*isolation_info*/,
        bool /*is_service_worker*/,
        int /*process_id*/,
        int /*routing_id*/,
        mojo::PendingReceiver<network::mojom::RestrictedCookieManager> *receiver)
{
    mojo::PendingReceiver<network::mojom::RestrictedCookieManager> orig_receiver = std::move(*receiver);

    mojo::PendingRemote<network::mojom::RestrictedCookieManager> target_rcm_remote;
    *receiver = target_rcm_remote.InitWithNewPipeAndPassReceiver();

    ProxyingRestrictedCookieManagerQt::CreateAndBind(
                ProfileIODataQt::FromBrowserContext(browser_context),
                std::move(target_rcm_remote),
                std::move(orig_receiver));

    return false;  // only made a proxy, still need the actual impl to be made.
}

content::AllowServiceWorkerResult
ContentBrowserClientQt::AllowServiceWorker(const GURL &scope,
                                           const net::SiteForCookies &site_for_cookies,
                                           const absl::optional<url::Origin> & /*top_frame_origin*/,
                                           const GURL & /*script_url*/,
                                           content::BrowserContext *context)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (!context || context->ShutdownStarted())
        return content::AllowServiceWorkerResult::No();
    // FIXME: Chrome also checks if javascript is enabled here to check if has been disabled since the service worker
    // was started.
    return static_cast<ProfileQt *>(context)->profileAdapter()->cookieStore()->d_func()->canAccessCookies(toQt(site_for_cookies.first_party_url()), toQt(scope))
         ? content::AllowServiceWorkerResult::Yes()
         : content::AllowServiceWorkerResult::No();
}

// We control worker access to FS and indexed-db using cookie permissions, this is mirroring Chromium's logic.
void ContentBrowserClientQt::AllowWorkerFileSystem(const GURL &url,
                                                   content::BrowserContext *context,
                                                   const std::vector<content::GlobalRenderFrameHostId> &/*render_frames*/,
                                                   base::OnceCallback<void(bool)> callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (!context || context->ShutdownStarted())
        return std::move(callback).Run(false);
    std::move(callback).Run(
            static_cast<ProfileQt *>(context)->profileAdapter()->cookieStore()->d_func()->canAccessCookies(toQt(url), toQt(url)));
}


bool ContentBrowserClientQt::AllowWorkerIndexedDB(const GURL &url,
                                                  content::BrowserContext *context,
                                                  const std::vector<content::GlobalRenderFrameHostId> &/*render_frames*/)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (!context || context->ShutdownStarted())
        return false;
    return static_cast<ProfileQt *>(context)->profileAdapter()->cookieStore()->d_func()->canAccessCookies(toQt(url), toQt(url));
}

static void LaunchURL(const GURL& url,
                      base::RepeatingCallback<content::WebContents*()> web_contents_getter,
                      ui::PageTransition page_transition,
                      network::mojom::WebSandboxFlags sandbox_flags,
                      bool is_main_frame, bool has_user_gesture)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    content::WebContents* webContents = std::move(web_contents_getter).Run();
    if (!webContents)
        return;

    custom_handlers::ProtocolHandlerRegistry *protocolHandlerRegistry =
            ProtocolHandlerRegistryFactory::GetForBrowserContext(webContents->GetBrowserContext());
    if (protocolHandlerRegistry && protocolHandlerRegistry->IsHandledProtocol(url.scheme()))
        return;

    // Sandbox flag logic from chrome/browser/chrome_content_browser_client.cc:
    if (!is_main_frame) {
        using SandboxFlags = network::mojom::WebSandboxFlags;
        auto allow = [&](SandboxFlags flag) {
          return (sandbox_flags & flag) == SandboxFlags::kNone;
        };
        bool allowed = (allow(SandboxFlags::kPopups)) ||
                       (allow(SandboxFlags::kTopNavigation)) ||
                       (allow(SandboxFlags::kTopNavigationByUserActivation) &&
                        has_user_gesture);

        if (!allowed) {
            content::RenderFrameHost *rfh = webContents->GetPrimaryMainFrame();
            if (!base::CommandLine::ForCurrentProcess()->HasSwitch("disable-sandbox-external-protocols")) {
                rfh->AddMessageToConsole(blink::mojom::ConsoleMessageLevel::kError,
                                         "Navigation to external protocol blocked by sandbox.");
                return;
            }
        }
    }

    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());
    contentsDelegate->launchExternalURL(toQt(url), page_transition, is_main_frame, has_user_gesture);
}


bool ContentBrowserClientQt::HandleExternalProtocol(const GURL &url,
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
        mojo::PendingRemote<network::mojom::URLLoaderFactory> *out_factory)
{
    Q_UNUSED(frame_tree_node_id);
    Q_UNUSED(is_in_fenced_frame_tree);
    Q_UNUSED(navigation_data);
    Q_UNUSED(initiating_origin);
    Q_UNUSED(initiator_document);
    Q_UNUSED(out_factory);

    content::GetUIThreadTaskRunner({})->PostTask(FROM_HERE,
                   base::BindOnce(&LaunchURL,
                                  url,
                                  std::move(web_contents_getter),
                                  page_transition,
                                  sandbox_flags,
                                  is_primary_main_frame,
                                  has_user_gesture));
    return true;
}

namespace {
// Copied from chrome/browser/chrome_content_browser_client.cc
class ProtocolHandlerThrottle : public blink::URLLoaderThrottle
{
public:
    explicit ProtocolHandlerThrottle(custom_handlers::ProtocolHandlerRegistry *protocol_handler_registry)
        : protocol_handler_registry_(protocol_handler_registry)
    {
    }
    ~ProtocolHandlerThrottle() override = default;

    void WillStartRequest(network::ResourceRequest *request, bool *defer) override
    {
        TranslateUrl(&request->url);
    }

    void WillRedirectRequest(net::RedirectInfo *redirect_info,
                             const network::mojom::URLResponseHead &response_head,
                             bool *defer,
                             std::vector<std::string> *to_be_removed_headers,
                             net::HttpRequestHeaders *modified_headers,
                             net::HttpRequestHeaders *modified_cors_exempt_headers) override
    {
        TranslateUrl(&redirect_info->new_url);
    }

private:
    void TranslateUrl(GURL *url)
    {
        if (!protocol_handler_registry_->IsHandledProtocol(url->scheme()))
            return;
        GURL translated_url = protocol_handler_registry_->Translate(*url);
        if (!translated_url.is_empty())
            *url = translated_url;
    }

    custom_handlers::ProtocolHandlerRegistry *protocol_handler_registry_;
};
} // namespace

std::vector<std::unique_ptr<blink::URLLoaderThrottle>>
ContentBrowserClientQt::CreateURLLoaderThrottles(
        const network::ResourceRequest &request, content::BrowserContext *browser_context,
        const base::RepeatingCallback<content::WebContents *()> & /*wc_getter*/,
        content::NavigationUIData * /*navigation_ui_data*/, int frame_tree_node_id)
{
    std::vector<std::unique_ptr<blink::URLLoaderThrottle>> result;
    result.push_back(std::make_unique<ProtocolHandlerThrottle>(
                         ProtocolHandlerRegistryFactory::GetForBrowserContext(browser_context)));
#if BUILDFLAG(ENABLE_EXTENSIONS)
    result.push_back(std::make_unique<PluginResponseInterceptorURLLoaderThrottle>(
                         request.destination, frame_tree_node_id));
#endif
    return result;
}

WebContentsAdapterClient::NavigationType pageTransitionToNavigationType(ui::PageTransition transition)
{
    if (ui::PageTransitionIsRedirect(transition))
        return WebContentsAdapterClient::RedirectNavigation;

    int32_t qualifier = ui::PageTransitionGetQualifier(transition);

    if (qualifier & ui::PAGE_TRANSITION_FORWARD_BACK)
        return WebContentsAdapterClient::BackForwardNavigation;

    ui::PageTransition strippedTransition = ui::PageTransitionStripQualifier(transition);

    switch (strippedTransition) {
    case ui::PAGE_TRANSITION_LINK:
        return WebContentsAdapterClient::LinkNavigation;
    case ui::PAGE_TRANSITION_TYPED:
        return WebContentsAdapterClient::TypedNavigation;
    case ui::PAGE_TRANSITION_FORM_SUBMIT:
        return WebContentsAdapterClient::FormSubmittedNavigation;
    case ui::PAGE_TRANSITION_RELOAD:
        return WebContentsAdapterClient::ReloadNavigation;
    default:
        return WebContentsAdapterClient::OtherNavigation;
    }
}

static bool navigationThrottleCallback(content::NavigationHandle *handle)
{
    // We call navigationRequested later in launchExternalUrl for external protocols.
    // The is_external_protocol parameter here is not fully accurate though,
    // and doesn't know about profile specific custom URL schemes.
    content::WebContents *source = handle->GetWebContents();
    ProfileQt *profile = static_cast<ProfileQt *>(source->GetBrowserContext());
    if (handle->IsExternalProtocol() && !profile->profileAdapter()->urlSchemeHandler(toQByteArray(handle->GetURL().scheme())))
        return false;

    bool navigationAccepted = true;

    WebContentsAdapterClient *client =
        WebContentsViewQt::from(static_cast<content::WebContentsImpl *>(source)->GetView())->client();
    if (!client)
        return false;

    // Redirects might not be reflected in transition_type at this point (see also chrome/.../web_navigation_api_helpers.cc)
    auto transition_type = handle->GetPageTransition();
    if (handle->WasServerRedirect())
        transition_type = ui::PageTransitionFromInt(transition_type | ui::PAGE_TRANSITION_SERVER_REDIRECT);

    client->navigationRequested(pageTransitionToNavigationType(transition_type),
                                toQt(handle->GetURL()),
                                navigationAccepted,
                                handle->IsInPrimaryMainFrame());
    return !navigationAccepted;
}

std::vector<std::unique_ptr<content::NavigationThrottle>> ContentBrowserClientQt::CreateThrottlesForNavigation(
        content::NavigationHandle *navigation_handle)
{
    std::vector<std::unique_ptr<content::NavigationThrottle>> throttles;
    throttles.push_back(std::make_unique<navigation_interception::InterceptNavigationThrottle>(
                            navigation_handle,
                            base::BindRepeating(&navigationThrottleCallback),
                            navigation_interception::SynchronyMode::kSync));

#if BUILDFLAG(ENABLE_PDF) && BUILDFLAG(ENABLE_EXTENSIONS)
    MaybeAddThrottle(
            extensions::PDFIFrameNavigationThrottleQt::MaybeCreateThrottleFor(navigation_handle),
            &throttles);
    MaybeAddThrottle(pdf::PdfNavigationThrottle::MaybeCreateThrottleFor(navigation_handle, std::make_unique<PdfStreamDelegateQt>()), &throttles);
#endif // BUILDFLAG(ENABLE_PDF) && BUIDLFLAG(ENABLE_EXTENSIONS)

    return throttles;
}

bool ContentBrowserClientQt::IsHandledURL(const GURL &url)
{
    return url::IsHandledProtocol(url.scheme());
}

bool ContentBrowserClientQt::HasCustomSchemeHandler(content::BrowserContext *browser_context,
                                                    const std::string &scheme)
{
    if (custom_handlers::ProtocolHandlerRegistry *protocol_handler_registry =
              ProtocolHandlerRegistryFactory::GetForBrowserContext(browser_context)) {
        return protocol_handler_registry->IsHandledProtocol(scheme);
    }

    return false;
}

bool ContentBrowserClientQt::HasErrorPage(int httpStatusCode, content::WebContents *contents)
{
    if (contents) {
        WebEngineSettings *settings = nullptr;
        WebContentsDelegateQt *delegate =
                    static_cast<WebContentsDelegateQt*>(contents->GetDelegate());
        if (delegate)
            settings = delegate->webEngineSettings();
        if (settings && !settings->testAttribute(QWebEngineSettings::ErrorPageEnabled))
            return false;
    }
    // Use an internal error page, if we have one for the status code.
    return error_page::LocalizedError::HasStrings(error_page::Error::kHttpErrorDomain, httpStatusCode);
}

std::unique_ptr<content::LoginDelegate> ContentBrowserClientQt::CreateLoginDelegate(
        const net::AuthChallengeInfo &authInfo,
        content::WebContents *web_contents,
        const content::GlobalRequestID & /*request_id*/,
        bool /*is_main_frame*/,
        const GURL &url,
        scoped_refptr<net::HttpResponseHeaders> /*response_headers*/,
        bool first_auth_attempt,
        LoginAuthRequiredCallback auth_required_callback)
{
    auto loginDelegate = std::make_unique<LoginDelegateQt>(authInfo, web_contents, url, first_auth_attempt, std::move(auth_required_callback));
    return loginDelegate;
}

bool ContentBrowserClientQt::ShouldIsolateErrorPage(bool in_main_frame)
{
    Q_UNUSED(in_main_frame);
    return false;
}

bool ContentBrowserClientQt::ShouldUseProcessPerSite(content::BrowserContext* browser_context, const GURL& effective_url)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
     if (effective_url.SchemeIs(extensions::kExtensionScheme))
        return true;
#endif
    return ContentBrowserClient::ShouldUseProcessPerSite(browser_context, effective_url);
}

bool ContentBrowserClientQt::DoesSiteRequireDedicatedProcess(content::BrowserContext *browser_context,
                                                             const GURL &effective_site_url)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    if (effective_site_url.SchemeIs(extensions::kExtensionScheme))
       return true;
#endif
    return ContentBrowserClient::DoesSiteRequireDedicatedProcess(browser_context, effective_site_url);
}

bool ContentBrowserClientQt::ShouldUseSpareRenderProcessHost(content::BrowserContext *browser_context,
                                                             const GURL &site_url)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    if (site_url.SchemeIs(extensions::kExtensionScheme))
       return false;
#endif
    return ContentBrowserClient::ShouldUseSpareRenderProcessHost(browser_context, site_url);
}

bool ContentBrowserClientQt::ShouldTreatURLSchemeAsFirstPartyWhenTopLevel(base::StringPiece scheme, bool is_embedded_origin_secure)
{
    if (is_embedded_origin_secure && scheme == content::kChromeUIScheme)
        return true;
#if BUILDFLAG(ENABLE_EXTENSIONS)
    return scheme == extensions::kExtensionScheme;
#else
    return false;
#endif
}

bool ContentBrowserClientQt::DoesSchemeAllowCrossOriginSharedWorker(const std::string &scheme)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    // Extensions are allowed to start cross-origin shared workers.
    return scheme == extensions::kExtensionScheme;
#else
    return false;
#endif
}

void ContentBrowserClientQt::OverrideURLLoaderFactoryParams(content::BrowserContext *browser_context,
                                                            const url::Origin &origin,
                                                            bool is_for_isolated_world,
                                                            network::mojom::URLLoaderFactoryParams *factory_params)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    extensions::URLLoaderFactoryManager::OverrideURLLoaderFactoryParams(
                browser_context, origin, is_for_isolated_world, factory_params);
#endif
}

std::string ContentBrowserClientQt::getUserAgent()
{
    // Mention the Chromium version we're based on to get passed stupid UA-string-based feature detection (several WebRTC demos need this)
    return content::BuildUserAgentFromProduct("QtWebEngine/" + std::string(qWebEngineVersion())
                                              + " Chrome/"
                                              + std::string(qWebEngineChromiumVersion()));
}

blink::UserAgentMetadata ContentBrowserClientQt::getUserAgentMetadata()
{
    static blink::UserAgentMetadata userAgentMetadata(embedder_support::GetUserAgentMetadata());
    return userAgentMetadata;
}

std::string ContentBrowserClientQt::GetProduct()
{
    QString productName(qApp->applicationName() % '/' % qApp->applicationVersion());
    return productName.toStdString();
}

scoped_refptr<network::SharedURLLoaderFactory> ContentBrowserClientQt::GetSystemSharedURLLoaderFactory()
{
    if (!SystemNetworkContextManager::GetInstance())
        return nullptr;
    return SystemNetworkContextManager::GetInstance()->GetSharedURLLoaderFactory();
}

network::mojom::NetworkContext *ContentBrowserClientQt::GetSystemNetworkContext()
{
    if (!SystemNetworkContextManager::GetInstance())
        return nullptr;
    return SystemNetworkContextManager::GetInstance()->GetContext();
}

void ContentBrowserClientQt::OnNetworkServiceCreated(network::mojom::NetworkService *network_service)
{
    if (!SystemNetworkContextManager::GetInstance())
        SystemNetworkContextManager::CreateInstance();

    // Need to set up global NetworkService state before anything else uses it.
    SystemNetworkContextManager::GetInstance()->OnNetworkServiceCreated(network_service);
}

void ContentBrowserClientQt::ConfigureNetworkContextParams(
        content::BrowserContext *context,
        bool in_memory,
        const base::FilePath &relative_partition_path,
        network::mojom::NetworkContextParams *network_context_params,
        cert_verifier::mojom::CertVerifierCreationParams *cert_verifier_creation_params)
{
    ProfileIODataQt::FromBrowserContext(context)->ConfigureNetworkContextParams(in_memory, relative_partition_path,
                                                                                network_context_params, cert_verifier_creation_params);

    mojo::PendingRemote<network::mojom::CookieManager> cookie_manager_remote;
    network_context_params->cookie_manager = cookie_manager_remote.InitWithNewPipeAndPassReceiver();
    ProfileIODataQt::FromBrowserContext(context)->cookieDelegate()->setMojoCookieManager(std::move(cookie_manager_remote));
}

std::vector<base::FilePath> ContentBrowserClientQt::GetNetworkContextsParentDirectory()
{
    return {
        toFilePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)),
        toFilePath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)) };
}

void ContentBrowserClientQt::RegisterNonNetworkNavigationURLLoaderFactories(int frame_tree_node_id,
                                                                            ukm::SourceIdObj ukm_source_id,
                                                                            NonNetworkURLLoaderFactoryMap *factories)
{
    content::WebContents *web_contents = content::WebContents::FromFrameTreeNodeId(frame_tree_node_id);
    Profile *profile = Profile::FromBrowserContext(web_contents->GetBrowserContext());
    ProfileAdapter *profileAdapter = static_cast<ProfileQt *>(profile)->profileAdapter();

    for (const QByteArray &scheme : profileAdapter->customUrlSchemes())
        factories->emplace(scheme.toStdString(), CreateCustomURLLoaderFactory(profileAdapter));

#if BUILDFLAG(ENABLE_EXTENSIONS)
    factories->emplace(
        extensions::kExtensionScheme,
        extensions::CreateExtensionNavigationURLLoaderFactory(profile, ukm_source_id,
                                                              !!extensions::WebViewGuest::FromWebContents(web_contents)));
#endif
}

void ContentBrowserClientQt::RegisterNonNetworkWorkerMainResourceURLLoaderFactories(content::BrowserContext *browser_context,
                                                                                    NonNetworkURLLoaderFactoryMap *factories)
{
    Profile *profile = Profile::FromBrowserContext(browser_context);
    ProfileAdapter *profileAdapter = static_cast<ProfileQt *>(profile)->profileAdapter();

    for (const QByteArray &scheme : profileAdapter->customUrlSchemes())
        factories->emplace(scheme.toStdString(), CreateCustomURLLoaderFactory(profileAdapter));

#if BUILDFLAG(ENABLE_EXTENSIONS)
    factories->emplace(
        extensions::kExtensionScheme,
        extensions::CreateExtensionWorkerMainResourceURLLoaderFactory(browser_context));
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)
}

void ContentBrowserClientQt::RegisterNonNetworkServiceWorkerUpdateURLLoaderFactories(content::BrowserContext* browser_context,
                                                                                     NonNetworkURLLoaderFactoryMap* factories)
{
    Profile *profile = Profile::FromBrowserContext(browser_context);
    ProfileAdapter *profileAdapter = static_cast<ProfileQt *>(profile)->profileAdapter();

    for (const QByteArray &scheme : profileAdapter->customUrlSchemes()) {
        if (const url::CustomScheme *cs = url::CustomScheme::FindScheme(scheme.toStdString())) {
            if (cs->flags & url::CustomScheme::ServiceWorkersAllowed)
                factories->emplace(scheme.toStdString(), CreateCustomURLLoaderFactory(profileAdapter));
        }
    }

#if BUILDFLAG(ENABLE_EXTENSIONS)
    factories->emplace(
        extensions::kExtensionScheme,
        extensions::CreateExtensionServiceWorkerScriptURLLoaderFactory(browser_context));
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)
}

void ContentBrowserClientQt::RegisterNonNetworkSubresourceURLLoaderFactories(int render_process_id, int render_frame_id,
                                                                             const absl::optional<url::Origin> &request_initiator_origin,
                                                                             NonNetworkURLLoaderFactoryMap *factories)
{
    Q_UNUSED(request_initiator_origin);
    content::RenderProcessHost *process_host = content::RenderProcessHost::FromID(render_process_id);
    Profile *profile = Profile::FromBrowserContext(process_host->GetBrowserContext());
    ProfileAdapter *profileAdapter = static_cast<ProfileQt *>(profile)->profileAdapter();

    for (const QByteArray &scheme : profileAdapter->customUrlSchemes())
        factories->emplace(scheme.toStdString(), CreateCustomURLLoaderFactory(profileAdapter));

    content::RenderFrameHost *frame_host = content::RenderFrameHost::FromID(render_process_id, render_frame_id);
    content::WebContents *web_contents = content::WebContents::FromRenderFrameHost(frame_host);
    GURL url;
    if (web_contents)
        url = web_contents->GetVisibleURL();

    bool is_background_page = false;
#if BUILDFLAG(ENABLE_EXTENSIONS)
    is_background_page = extensions::GetViewType(web_contents) == extensions::mojom::ViewType::kExtensionBackgroundPage;
#endif // BUILDFLAG(ENABLE_EXTENSIONS)

    // Install file scheme if necessary:
    bool install_file_scheme = false;
    if (web_contents && !is_background_page) {
        const std::string scheme = url.scheme();
        install_file_scheme = base::Contains(url::GetLocalSchemes(), scheme);
        if (const url::CustomScheme *cs = url::CustomScheme::FindScheme(scheme))
            install_file_scheme = cs->flags & (url::CustomScheme::LocalAccessAllowed | url::CustomScheme::Local);
    }

    if (install_file_scheme && factories->find(url::kFileScheme) == factories->end()) {
        auto file_factory = content::CreateFileURLLoaderFactory(profile->GetPath(),
                                                                profile->GetSharedCorsOriginAccessList());
        factories->emplace(url::kFileScheme, std::move(file_factory));
    }

#if BUILDFLAG(ENABLE_EXTENSIONS)
    auto factory = extensions::CreateExtensionURLLoaderFactory(render_process_id, render_frame_id);
    if (factory)
        factories->emplace(extensions::kExtensionScheme, std::move(factory));

    if (!web_contents)
        return;

    extensions::ExtensionWebContentsObserverQt *web_observer =
            extensions::ExtensionWebContentsObserverQt::FromWebContents(web_contents);
    if (!web_observer)
        return;

    const extensions::Extension *extension = web_observer->GetExtensionFromFrame(frame_host, false);
    if (!extension)
        return;

    std::vector<std::string> allowed_webui_hosts;
    // Support for chrome:// scheme if appropriate.
    if ((extension->is_extension() || extension->is_platform_app()) &&
            extensions::Manifest::IsComponentLocation(extension->location())) {
        // Components of chrome that are implemented as extensions or platform apps
        // are allowed to use chrome://resources/ and chrome://theme/ URLs.
        allowed_webui_hosts.emplace_back(content::kChromeUIResourcesHost);
    }
    if (!allowed_webui_hosts.empty()) {
        factories->emplace(content::kChromeUIScheme,
                           content::CreateWebUIURLLoaderFactory(frame_host,
                                                                content::kChromeUIScheme,
                                                                std::move(allowed_webui_hosts)));
    }
#endif
}

base::flat_set<std::string> ContentBrowserClientQt::GetPluginMimeTypesWithExternalHandlers(
        content::BrowserContext *browser_context)
{
    base::flat_set<std::string> mime_types;
#if BUILDFLAG(ENABLE_EXTENSIONS)
    ProfileQt *profile = static_cast<ProfileQt *>(browser_context);
    for (const std::string &extension_id : MimeTypesHandler::GetMIMETypeAllowlist()) {
        const extensions::Extension *extension =
            extensions::ExtensionRegistry::Get(browser_context)
                ->enabled_extensions()
                .GetByID(extension_id);
        // The allowed extension may not be installed, so we have to nullptr
        // check |extension|.
        if (!extension ||
            (profile->IsOffTheRecord() && !extensions::util::IsIncognitoEnabled(
                                              extension_id, browser_context))) {
            continue;
        }
        if (MimeTypesHandler *handler = MimeTypesHandler::GetHandler(extension)) {
            for (const auto &supported_mime_type : handler->mime_type_set())
                mime_types.insert(supported_mime_type);
        }
    }
#endif
#if BUILDFLAG(ENABLE_PDF)
    mime_types.insert("application/x-google-chrome-pdf");
#endif
    return mime_types;
}

bool ContentBrowserClientQt::WillCreateURLLoaderFactory(
        content::BrowserContext *browser_context,
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
        network::mojom::URLLoaderFactoryOverridePtr *factory_override)
{
    Q_UNUSED(render_process_id);
    Q_UNUSED(type);
    Q_UNUSED(request_initiator);
    Q_UNUSED(navigation_id);
    Q_UNUSED(ukm_source_id);
    Q_UNUSED(header_client);
    Q_UNUSED(bypass_redirect_checks);
    Q_UNUSED(disable_secure_dns);
    Q_UNUSED(factory_override);
    auto adapter = static_cast<ProfileQt *>(browser_context)->profileAdapter();
    auto proxied_receiver = std::move(*factory_receiver);
    mojo::PendingRemote<network::mojom::URLLoaderFactory> pending_url_loader_factory;
    *factory_receiver = pending_url_loader_factory.InitWithNewPipeAndPassReceiver();
    // Will manage its own lifetime
    new ProxyingURLLoaderFactoryQt(adapter,
                                   frame ? frame->GetFrameTreeNodeId() : content::RenderFrameHost::kNoFrameTreeNodeId,
                                   std::move(proxied_receiver), std::move(pending_url_loader_factory));
    return true;
}

std::vector<std::unique_ptr<content::URLLoaderRequestInterceptor>>
ContentBrowserClientQt::WillCreateURLLoaderRequestInterceptors(content::NavigationUIData* navigation_ui_data,
                                       int frame_tree_node_id)
{
    std::vector<std::unique_ptr<content::URLLoaderRequestInterceptor>> interceptors;
#if BUILDFLAG(ENABLE_PDF) && BUILDFLAG(ENABLE_EXTENSIONS)
    {
        std::unique_ptr<content::URLLoaderRequestInterceptor> pdf_interceptor =
                pdf::PdfURLLoaderRequestInterceptor::MaybeCreateInterceptor(
                    frame_tree_node_id, std::make_unique<PdfStreamDelegateQt>());
        if (pdf_interceptor)
            interceptors.push_back(std::move(pdf_interceptor));
    }
#endif // BUILDFLAG(ENABLE_PDF) && BUIDLFLAG(ENABLE_EXTENSIONS)

    return interceptors;
}

bool ContentBrowserClientQt::WillInterceptWebSocket(content::RenderFrameHost *frame)
{
    return frame != nullptr;
}

QWebEngineUrlRequestInterceptor *getProfileInterceptorFromFrame(content::RenderFrameHost *frame)
{
    ProfileQt *profile = static_cast<ProfileQt *>(frame->GetBrowserContext());
    if (profile)
        return profile->profileAdapter()->requestInterceptor();
    return nullptr;
}

QWebEngineUrlRequestInterceptor *getPageInterceptor(content::WebContents *web_contents)
{
    if (web_contents) {
        auto view = static_cast<content::WebContentsImpl *>(web_contents)->GetView();
        if (WebContentsAdapterClient *client = WebContentsViewQt::from(view)->client())
            return client->webContentsAdapter()->requestInterceptor();
    }
    return nullptr;
}

void ContentBrowserClientQt::CreateWebSocket(
        content::RenderFrameHost *frame,
        WebSocketFactory factory,
        const GURL &url,
        const net::SiteForCookies &site_for_cookies,
        const absl::optional<std::string> &user_agent,
        mojo::PendingRemote<network::mojom::WebSocketHandshakeClient> handshake_client)
{
    QWebEngineUrlRequestInterceptor *profileInterceptor = getProfileInterceptorFromFrame(frame);
    content::WebContents *web_contents = content::WebContents::FromRenderFrameHost(frame);
    QWebEngineUrlRequestInterceptor *pageInterceptor = getPageInterceptor(web_contents);
    std::vector<network::mojom::HttpHeaderPtr> headers;
    GURL to_url = url;
    bool addedUserAgent = false;
    if (profileInterceptor || pageInterceptor) {
        QUrl initiator = web_contents ? toQt(web_contents->GetURL()) : QUrl();
        auto *infoPrivate = new QWebEngineUrlRequestInfoPrivate(
                    QWebEngineUrlRequestInfo::ResourceTypeWebSocket,
                    QWebEngineUrlRequestInfo::NavigationTypeOther,
                    toQt(url), toQt(site_for_cookies.first_party_url()), initiator,
                    QByteArrayLiteral("GET"));
        QWebEngineUrlRequestInfo requestInfo(infoPrivate);
        if (profileInterceptor) {
            profileInterceptor->interceptRequest(requestInfo);
            pageInterceptor = getPageInterceptor(web_contents);
        }
        if (pageInterceptor && !requestInfo.changed())
            pageInterceptor->interceptRequest(requestInfo);
        if (infoPrivate->shouldBlockRequest)
            return; // ### should we call OnFailure on handshake_client?
        if (infoPrivate->shouldRedirectRequest)
            to_url = toGurl(infoPrivate->url);
        for (auto header = infoPrivate->extraHeaders.constBegin(); header != infoPrivate->extraHeaders.constEnd(); ++header) {
            std::string h = header.key().toStdString();
            if (base::EqualsCaseInsensitiveASCII(h, net::HttpRequestHeaders::kUserAgent))
                addedUserAgent = true;
            headers.push_back(network::mojom::HttpHeader::New(h, header.value().toStdString()));
        }
    }
    if (!addedUserAgent && user_agent)
        headers.push_back(network::mojom::HttpHeader::New(net::HttpRequestHeaders::kUserAgent, *user_agent));

    std::move(factory).Run(to_url, std::move(headers), std::move(handshake_client), mojo::NullRemote(), mojo::NullRemote());
}

void ContentBrowserClientQt::SiteInstanceGotProcess(content::SiteInstance *site_instance)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    content::BrowserContext *context = site_instance->GetBrowserContext();
    extensions::ExtensionRegistry *registry = extensions::ExtensionRegistry::Get(context);
    const extensions::Extension *extension = registry->enabled_extensions().GetExtensionOrAppByURL(site_instance->GetSiteURL());
    if (!extension)
        return;

    extensions::ProcessMap *processMap = extensions::ProcessMap::Get(context);
    processMap->Insert(extension->id(), site_instance->GetProcess()->GetID(), site_instance->GetId());
#endif
}

void ContentBrowserClientQt::SiteInstanceDeleting(content::SiteInstance *site_instance)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    // Don't do anything if we're shutting down.
    if (content::BrowserMainRunner::ExitedMainMessageLoop() || !site_instance->HasProcess())
       return;

    content::BrowserContext *context = site_instance->GetBrowserContext();
    extensions::ExtensionRegistry *registry = extensions::ExtensionRegistry::Get(context);
    const extensions::Extension *extension = registry->enabled_extensions().GetExtensionOrAppByURL(site_instance->GetSiteURL());
    if (!extension)
        return;

    extensions::ProcessMap *processMap = extensions::ProcessMap::Get(context);
    processMap->Remove(extension->id(), site_instance->GetProcess()->GetID(), site_instance->GetId());
#endif
}

std::unique_ptr<content::WebContentsViewDelegate> ContentBrowserClientQt::GetWebContentsViewDelegate(content::WebContents *web_contents)
{
    FormInteractionTabHelper::CreateForWebContents(web_contents);
    FileSystemAccessPermissionRequestManagerQt::CreateForWebContents(web_contents);
    if (auto *registry = performance_manager::PerformanceManagerRegistry::GetInstance())
        registry->MaybeCreatePageNodeForWebContents(web_contents);

     return nullptr;
}

content::ContentBrowserClient::AllowWebBluetoothResult
ContentBrowserClientQt::AllowWebBluetooth(content::BrowserContext *browser_context,
                                          const url::Origin &requesting_origin,
                                          const url::Origin &embedding_origin)
{
    DCHECK(browser_context);
    return content::ContentBrowserClient::AllowWebBluetoothResult::BLOCK_GLOBALLY_DISABLED;
}

} // namespace QtWebEngineCore
