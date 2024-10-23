// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "renderer/content_renderer_client_qt.h"

#include "renderer/content_settings_observer_qt.h"
#include "renderer/render_configuration.h"
#include "renderer/render_frame_observer_qt.h"
#include "renderer/user_resource_controller.h"
#include "renderer/web_engine_page_render_frame.h"
#include "web_engine_library_info.h"

#include "base/task/sequenced_task_runner.h"
#include "components/autofill/content/renderer/password_autofill_agent.h"
#include "components/autofill/content/renderer/password_generation_agent.h"
#include "components/cdm/renderer/external_clear_key_key_system_info.h"
#include "components/cdm/renderer/key_system_support_update.h"
#include "components/cdm/renderer/widevine_key_system_info.h"
#include "components/error_page/common/error.h"
#include "components/error_page/common/localized_error.h"
#include "components/grit/components_resources.h"
#include "components/network_hints/renderer/web_prescient_networking_impl.h"
#include "components/visitedlink/renderer/visitedlink_reader.h"
#include "components/web_cache/renderer/web_cache_impl.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/common/url_constants.h"
#include "content/public/renderer/render_thread.h"
#include "extensions/buildflags/buildflags.h"
#include "media/media_buildflags.h"
#include "mojo/public/cpp/bindings/binder_map.h"
#include "net/base/net_errors.h"
#include "ppapi/buildflags/buildflags.h"
#include "printing/buildflags/buildflags.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "third_party/blink/public/platform/web_url_error.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/webui/jstemplate_builder.h"
#include "ui/base/webui/web_ui_util.h"

#if QT_CONFIG(webengine_spellchecker)
#include "components/spellcheck/renderer/spellcheck.h"
#include "components/spellcheck/renderer/spellcheck_provider.h"
#endif

#if QT_CONFIG(webengine_printing_and_pdf)
#include "renderer/print_web_view_helper_delegate_qt.h"

#include "components/pdf/renderer/internal_plugin_renderer_helpers.h"
#include "components/pdf/renderer/pdf_internal_plugin_delegate.h"
#include "components/printing/renderer/print_render_frame_helper.h"
#endif

#if QT_CONFIG(webengine_webchannel)
#include "renderer/web_channel_ipc_transport.h"
#endif

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "common/extensions/extensions_client_qt.h"
#include "extensions/extensions_renderer_api_provider_qt.h"
#include "extensions/extensions_renderer_client_qt.h"

#include "extensions/common/constants.h"
#include "extensions/renderer/api/core_extensions_renderer_api_provider.h"
#include "extensions/renderer/guest_view/mime_handler_view/mime_handler_view_container_manager.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "third_party/blink/public/web/web_security_policy.h"
#endif // ENABLE_EXTENSIONS

#if BUILDFLAG(ENABLE_PLUGINS)
#include "content/renderer/render_frame_impl.h"
#include "plugins/loadable_plugin_placeholder_qt.h"
#endif // ENABLE_PLUGINS

#if BUILDFLAG(ENABLE_LIBRARY_CDMS)
#include "base/feature_list.h"
#include "content/public/renderer/key_system_support.h"
#include "media/base/media_switches.h"
#include "media/base/video_codecs.h"
#include "media/cdm/clear_key_cdm_common.h"
#include "third_party/widevine/cdm/buildflags.h"
#if BUILDFLAG(ENABLE_WIDEVINE)
#include "third_party/widevine/cdm/widevine_cdm_common.h"
#endif
#endif

#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
#include "chrome/renderer/media/webrtc_logging_agent_impl.h"
#endif

namespace QtWebEngineCore {

ContentRendererClientQt::ContentRendererClientQt()
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    extensions::ExtensionsClient::Set(extensions::ExtensionsClientQt::GetInstance());
    extensions::ExtensionsRendererClient::Set(ExtensionsRendererClientQt::GetInstance());
#endif
}

ContentRendererClientQt::~ContentRendererClientQt() {}

void ContentRendererClientQt::RenderThreadStarted()
{
    base::i18n::SetICUDefaultLocale(WebEngineLibraryInfo::getApplicationLocale());
    content::RenderThread *renderThread = content::RenderThread::Get();
    m_renderConfiguration.reset(new RenderConfiguration());
    m_userResourceController.reset(new UserResourceController());
    m_visitedLinkReader.reset(new visitedlink::VisitedLinkReader);
    m_webCacheImpl.reset(new web_cache::WebCacheImpl());

    renderThread->AddObserver(m_renderConfiguration.data());
    renderThread->AddObserver(m_userResourceController.data());

#if QT_CONFIG(webengine_spellchecker)
    if (!m_spellCheck)
        InitSpellCheck();
#endif

#if BUILDFLAG(ENABLE_EXTENSIONS)
    // Allow the pdf viewer extension to access chrome resources
    blink::WebURL pdfViewerExtension(GURL("chrome-extension://mhjfbmdgcfjbbpaeojofohoefgiehjai"));
    blink::WebString chromeResources(blink::WebString::FromASCII("chrome"));
    blink::WebSecurityPolicy::AddOriginAccessAllowListEntry(
            pdfViewerExtension, chromeResources, blink::WebString(), 0,
            network::mojom::CorsDomainMatchMode::kAllowSubdomains, network::mojom::CorsPortMatchMode::kAllowAnyPort,
            network::mojom::CorsOriginAccessMatchPriority::kDefaultPriority);

    auto *extensions_renderer_client = extensions::ExtensionsRendererClient::Get();
    extensions_renderer_client->AddAPIProvider(
            std::make_unique<extensions::CoreExtensionsRendererAPIProvider>());
    extensions_renderer_client->AddAPIProvider(
            std::make_unique<extensions::ExtensionsRendererAPIProviderQt>());

    extensions_renderer_client->RenderThreadStarted();
#endif
}

void ContentRendererClientQt::ExposeInterfacesToBrowser(mojo::BinderMap* binders)
{
    binders->Add<visitedlink::mojom::VisitedLinkNotificationSink>(
                m_visitedLinkReader->GetBindCallback(), base::SingleThreadTaskRunner::GetCurrentDefault());

    binders->Add<web_cache::mojom::WebCache>(
                base::BindRepeating(&web_cache::WebCacheImpl::BindReceiver,
                                    base::Unretained(m_webCacheImpl.get())),
                base::SingleThreadTaskRunner::GetCurrentDefault());

#if QT_CONFIG(webengine_spellchecker)
    binders->Add<spellcheck::mojom::SpellChecker>(
                 base::BindRepeating(
                         [](ContentRendererClientQt *client,
                            mojo::PendingReceiver<spellcheck::mojom::SpellChecker> receiver) {
                             if (!client->m_spellCheck)
                                 client->InitSpellCheck();
                             client->m_spellCheck->BindReceiver(std::move(receiver));
                         }, this),
                 base::SingleThreadTaskRunner::GetCurrentDefault());
#endif

#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
    binders->Add<chrome::mojom::WebRtcLoggingAgent>(
                 base::BindRepeating(
                         [](ContentRendererClientQt *client,
                            mojo::PendingReceiver<chrome::mojom::WebRtcLoggingAgent> receiver) {
                                client->GetWebRtcLoggingAgent()->AddReceiver(std::move(receiver));
                         }, this),
                 base::SingleThreadTaskRunner::GetCurrentDefault());
#endif
}

void ContentRendererClientQt::RenderFrameCreated(content::RenderFrame *render_frame)
{
    QtWebEngineCore::RenderFrameObserverQt *render_frame_observer =
            new QtWebEngineCore::RenderFrameObserverQt(render_frame, m_webCacheImpl.data());
    if (render_frame->IsMainFrame()) {
#if QT_CONFIG(webengine_webchannel)
        new WebChannelIPCTransport(render_frame);
#endif
        new WebEnginePageRenderFrame(render_frame);
    }

    m_userResourceController->renderFrameCreated(render_frame);

    new QtWebEngineCore::ContentSettingsObserverQt(render_frame);

#if QT_CONFIG(webengine_spellchecker)
    new SpellCheckProvider(render_frame, m_spellCheck.data());
#endif
#if QT_CONFIG(webengine_printing_and_pdf)
    new printing::PrintRenderFrameHelper(render_frame, base::WrapUnique(new PrintWebViewHelperDelegateQt()));
#endif // QT_CONFIG(webengine_printing_and_pdf)

    blink::AssociatedInterfaceRegistry *associated_interfaces = render_frame_observer->associatedInterfaces();

#if BUILDFLAG(ENABLE_EXTENSIONS)
    associated_interfaces->AddInterface<extensions::mojom::MimeHandlerViewContainerManager>(
            base::BindRepeating(&extensions::MimeHandlerViewContainerManager::BindReceiver,
                                base::Unretained(render_frame)));

    auto registry = std::make_unique<service_manager::BinderRegistry>();
    ExtensionsRendererClientQt::GetInstance()->RenderFrameCreated(render_frame, render_frame_observer->registry());
#endif

    auto password_autofill_agent =
            std::make_unique<autofill::PasswordAutofillAgent>(render_frame, associated_interfaces, autofill::PasswordAutofillAgent::EnableHeavyFormDataScraping(false));
    auto password_generation_agent =
            std::make_unique<autofill::PasswordGenerationAgent>(render_frame, password_autofill_agent.get(), associated_interfaces);

    new autofill::AutofillAgent(
            render_frame,
            {
              autofill::AutofillAgent::ExtractAllDatalists(false), autofill::AutofillAgent::FocusRequiresScroll(true),
              autofill::AutofillAgent::QueryPasswordSuggestions(false), autofill::AutofillAgent::SecureContextRequired(false),
              autofill::AutofillAgent::UserGestureRequired(true), autofill::AutofillAgent::UsesKeyboardAccessoryForSuggestions(false)
            },
            std::move(password_autofill_agent), std::move(password_generation_agent),
            associated_interfaces);
}

void ContentRendererClientQt::WebViewCreated(blink::WebView *web_view,
                                             bool was_created_by_renderer,
                                             const url::Origin *outermost_origin)
{
    Q_UNUSED(was_created_by_renderer);
#if BUILDFLAG(ENABLE_EXTENSIONS)
    ExtensionsRendererClientQt::GetInstance()->WebViewCreated(web_view, outermost_origin);
#endif
}

void ContentRendererClientQt::RunScriptsAtDocumentStart(content::RenderFrame *render_frame)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    ExtensionsRendererClientQt::GetInstance()->RunScriptsAtDocumentStart(render_frame);
    // |render_frame| might be dead by now.
#endif
}

void ContentRendererClientQt::RunScriptsAtDocumentEnd(content::RenderFrame *render_frame)
{
    // Check whether the render_frame has been created and has not been detached yet.
    // Otherwise the WebFrame is not available.
    RenderFrameObserverQt *render_frame_observer = RenderFrameObserverQt::Get(render_frame);

    if (render_frame_observer && !render_frame_observer->isFrameDetached())
        m_userResourceController->RunScriptsAtDocumentEnd(render_frame);

#if BUILDFLAG(ENABLE_EXTENSIONS)
    ExtensionsRendererClientQt::GetInstance()->RunScriptsAtDocumentEnd(render_frame);
    // |render_frame| might be dead by now.
#endif
}

void ContentRendererClientQt::RunScriptsAtDocumentIdle(content::RenderFrame *render_frame)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    ExtensionsRendererClientQt::GetInstance()->RunScriptsAtDocumentIdle(render_frame);
    // |render_frame| might be dead by now.
#endif
}

// To tap into the chromium localized strings. Ripped from the chrome layer (highly simplified).
void ContentRendererClientQt::PrepareErrorPage(content::RenderFrame *renderFrame,
                                               const blink::WebURLError &web_error,
                                               const std::string &httpMethod,
                                               content::mojom::AlternativeErrorPageOverrideInfoPtr alternative_error_page_info,
                                               std::string *errorHtml)
{
    GetNavigationErrorStringsInternal(
            renderFrame, httpMethod,
            error_page::Error::NetError((GURL)web_error.url(), web_error.reason(), web_error.extended_reason(),
                                        net::ResolveErrorInfo(), web_error.has_copy_in_cache()),
            errorHtml);
}

void ContentRendererClientQt::PrepareErrorPageForHttpStatusError(content::RenderFrame *renderFrame,
                                                                 const blink::WebURLError &error,
                                                                 const std::string &httpMethod,
                                                                 int http_status,
                                                                 content::mojom::AlternativeErrorPageOverrideInfoPtr alternative_error_page_info,
                                                                 std::string *errorHtml)
{
    GetNavigationErrorStringsInternal(renderFrame, httpMethod,
                                      error_page::Error::HttpError(error.url(), http_status),
                                      errorHtml);
}

void ContentRendererClientQt::GetNavigationErrorStringsInternal(content::RenderFrame *renderFrame,
                                                                const std::string &httpMethod,
                                                                const error_page::Error &error,
                                                                std::string *errorHtml)
{
    Q_UNUSED(renderFrame);
    const bool isPost = QByteArray::fromStdString(httpMethod) == QByteArrayLiteral("POST");

    if (errorHtml) {
        // Use a local error page.
        int resourceId;

        const std::string locale = content::RenderThread::Get()->GetLocale();
        // TODO(elproxy): We could potentially get better diagnostics here by first calling
        // NetErrorHelper::GetErrorStringsForDnsProbe, but that one is harder to untangle.

        base::Value::Dict error_page_params;
        error_page::LocalizedError::PageState errorPageState =
                error_page::LocalizedError::GetPageState(
                        error.reason(), error.domain(), error.url(), isPost, false,
                        error.stale_copy_in_cache(), false,
                        RenderConfiguration::is_incognito_process(), false, false, false, locale, false, &error_page_params);

        resourceId = IDR_NET_ERROR_HTML;

        std::string extracted_string = ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(resourceId);
        const std::string_view template_html(extracted_string.data(), extracted_string.size());
        if (template_html.empty())
            NOTREACHED() << "unable to load template. ID: " << resourceId;
        else // "t" is the id of the templates root node.
            *errorHtml = webui::GetLocalizedHtml(template_html, errorPageState.strings);
    }
}

uint64_t ContentRendererClientQt::VisitedLinkHash(std::string_view canonicalUrl)
{
    return m_visitedLinkReader->ComputeURLFingerprint(canonicalUrl);
}

bool ContentRendererClientQt::IsLinkVisited(uint64_t linkHash)
{
    return m_visitedLinkReader->IsVisited(linkHash);
}

std::unique_ptr<blink::WebPrescientNetworking> ContentRendererClientQt::CreatePrescientNetworking(content::RenderFrame *render_frame)
{
    return std::make_unique<network_hints::WebPrescientNetworkingImpl>(render_frame);
}

namespace {
bool IsPdfExtensionOrigin(const url::Origin &origin)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    return origin.scheme() == extensions::kExtensionScheme &&
           origin.host() == extension_misc::kPdfExtensionId;
#else
    return false;
#endif
}

#if BUILDFLAG(ENABLE_PLUGINS)
void AppendParams(const std::vector<content::WebPluginMimeType::Param> &additional_params,
                  blink::WebVector<blink::WebString> *existing_names,
                  blink::WebVector<blink::WebString> *existing_values)
{
    DCHECK(existing_names->size() == existing_values->size());
    size_t existing_size = existing_names->size();
    size_t total_size = existing_size + additional_params.size();

    blink::WebVector<blink::WebString> names(total_size);
    blink::WebVector<blink::WebString> values(total_size);

    for (size_t i = 0; i < existing_size; ++i) {
        names[i] = (*existing_names)[i];
        values[i] = (*existing_values)[i];
    }

    for (size_t i = 0; i < additional_params.size(); ++i) {
        names[existing_size + i] = blink::WebString::FromUTF16(additional_params[i].name);
        values[existing_size + i] = blink::WebString::FromUTF16(additional_params[i].value);
    }

    existing_names->swap(names);
    existing_values->swap(values);
}
#endif  // BUILDFLAG(ENABLE_PLUGINS)

#if QT_CONFIG(webengine_printing_and_pdf)
// based on chrome/renderer/pdf/chrome_pdf_internal_plugin_delegate.cc:
class PdfInternalPluginDelegateQt final
    : public pdf::PdfInternalPluginDelegate
{
public:
    PdfInternalPluginDelegateQt() = default;
    PdfInternalPluginDelegateQt(const PdfInternalPluginDelegateQt &) = delete;
    PdfInternalPluginDelegateQt& operator=(const PdfInternalPluginDelegateQt &) = delete;
    ~PdfInternalPluginDelegateQt() override = default;

    // `pdf::PdfInternalPluginDelegate`:
    bool IsAllowedOrigin(const url::Origin &origin) const override;
};

bool PdfInternalPluginDelegateQt::IsAllowedOrigin(const url::Origin &origin) const
{
    return IsPdfExtensionOrigin(origin);
}
#endif
} // namespace

bool ContentRendererClientQt::IsPluginHandledExternally(content::RenderFrame *render_frame,
                                   const blink::WebElement &plugin_element,
                                   const GURL &original_url,
                                   const std::string &original_mime_type)
{
#if BUILDFLAG(ENABLE_EXTENSIONS) && BUILDFLAG(ENABLE_PLUGINS)
    bool found = false;
    content::WebPluginInfo plugin_info;
    std::string mime_type;

    static_cast<content::RenderFrameImpl *>(render_frame)->GetPepperHost()->GetPluginInfo(
                original_url, original_mime_type, &found, &plugin_info, &mime_type);
    if (!found)
        return false;
    if (IsPdfExtensionOrigin(render_frame->GetWebFrame()->GetSecurityOrigin()))
        return true;
    return extensions::MimeHandlerViewContainerManager::Get(
                content::RenderFrame::FromWebFrame(
                    plugin_element.GetDocument().GetFrame()),
                true /* create_if_does_not_exist */)
                    ->CreateFrameContainer(plugin_element, original_url, mime_type, plugin_info);
#else
    return false;
#endif
}

bool ContentRendererClientQt::OverrideCreatePlugin(content::RenderFrame *render_frame,
                                                   const blink::WebPluginParams &params,
                                                   blink::WebPlugin **plugin)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    if (!ExtensionsRendererClientQt::GetInstance()->OverrideCreatePlugin(render_frame, params))
        return false;
#endif // ENABLE_EXTENSIONS

#if BUILDFLAG(ENABLE_PLUGINS)
    content::WebPluginInfo info;
    std::string actual_mime_type;
    bool found = false;

    static_cast<content::RenderFrameImpl *>(render_frame)
            ->GetPepperHost()
            ->GetPluginInfo(params.url, params.mime_type.Utf8(), &found, &info, &actual_mime_type);
    if (!found) {
        *plugin = LoadablePluginPlaceholderQt::CreateLoadableMissingPlugin(render_frame, params)->plugin();
        return true;
    }
    if (info.name == u"Chromium PDF Viewer") {
        blink::WebPluginParams new_params(params);
        for (const auto& mime_type : info.mime_types) {
          if (mime_type.mime_type == params.mime_type.Utf8()) {
            AppendParams(mime_type.additional_params, &new_params.attribute_names,
                         &new_params.attribute_values);
            break;
          }
        }

        *plugin = pdf::CreateInternalPlugin(std::move(new_params), render_frame, std::make_unique<PdfInternalPluginDelegateQt>());
        return true;
    }
    *plugin = render_frame->CreatePlugin(info, params);
#endif // BUILDFLAG(ENABLE_PLUGINS)
    return true;
}

bool ContentRendererClientQt::IsOriginIsolatedPepperPlugin(const base::FilePath& plugin_path)
{
    return plugin_path.value() == FILE_PATH_LITERAL("internal-pdf-viewer");
}

#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
chrome::WebRtcLoggingAgentImpl *ContentRendererClientQt::GetWebRtcLoggingAgent()
{
    if (!m_webrtcLoggingAgentImpl) {
        m_webrtcLoggingAgentImpl = std::make_unique<chrome::WebRtcLoggingAgentImpl>();
    }

    return m_webrtcLoggingAgentImpl.get();
}
#endif // QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)

void ContentRendererClientQt::GetInterface(const std::string &interface_name, mojo::ScopedMessagePipeHandle interface_pipe)
{
    content::RenderThread::Get()->BindHostReceiver(mojo::GenericPendingReceiver(interface_name, std::move(interface_pipe)));
}

std::unique_ptr<media::KeySystemSupportRegistration> ContentRendererClientQt::GetSupportedKeySystems(content::RenderFrame *render_frame, media::GetSupportedKeySystemsCB cb)
{
    return cdm::GetSupportedKeySystemsUpdates(render_frame, /*can_persist_data=*/false, std::move(cb));
}

#if QT_CONFIG(webengine_spellchecker)
void ContentRendererClientQt::InitSpellCheck()
{
    m_spellCheck.reset(new SpellCheck(this));
}
#endif

void ContentRendererClientQt::WillSendRequest(blink::WebLocalFrame *frame,
                                              ui::PageTransition transition_type,
                                              const blink::WebURL &upstream_url,
                                              const blink::WebURL &target_url,
                                              const net::SiteForCookies &site_for_cookies,
                                              const url::Origin *initiator_origin,
                                              GURL *new_url)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    ExtensionsRendererClientQt::GetInstance()->WillSendRequest(frame, transition_type, target_url, site_for_cookies,
                                                               initiator_origin, new_url);
    if (!new_url->is_empty())
        return;
#endif
}

} // namespace QtWebEngineCore
