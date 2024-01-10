// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Portions copyright 2015 The Chromium Embedded Framework Authors.
// Portions copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions_browser_client_qt.h"

#include <utility>

#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "base/path_service.h"
#include "base/task/thread_pool.h"
#include "base/memory/ref_counted_memory.h"
#include "chrome/browser/extensions/api/generated_api_registration.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/render_frame_host.h"
#include "extensions/browser/api/core_extensions_browser_api_provider.h"
#include "extensions/browser/api/extensions_api_client.h"
#include "extensions/browser/api/runtime/runtime_api_delegate.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_host_delegate.h"
#include "extensions/browser/extension_protocols.h"
#include "extensions/browser/extensions_browser_api_provider.h"
#include "extensions/browser/extensions_browser_interface_binders.h"
#include "extensions/browser/url_request_util.h"
#include "extensions/common/file_util.h"
#include "net/base/mime_util.h"
#include "qtwebengine/browser/extensions/api/generated_api_registration.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "third_party/zlib/google/compression_utils.h"
#include "ui/base/resource/resource_bundle.h"

#include "component_extension_resource_manager_qt.h"
#include "extension_system_factory_qt.h"
#include "extension_web_contents_observer_qt.h"
#include "extensions_api_client_qt.h"
#include "extensions_browser_client_qt.h"
#include "extension_host_delegate_qt.h"
#include "web_engine_library_info.h"

using content::BrowserContext;

namespace {

// helpers based on implementation in chrome_url_request_util.cc:
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

void DetermineCharset(const std::string &mime_type,
                      const base::RefCountedMemory *data,
                      std::string *out_charset)
{
    if (base::StartsWith(mime_type, "text/", base::CompareCase::INSENSITIVE_ASCII)) {
        // All of our HTML files should be UTF-8 and for other resource types
        // (like images), charset doesn't matter.
        DCHECK(base::IsStringUTF8(base::StringPiece(reinterpret_cast<const char *>(data->front()), data->size())));
        *out_charset = "utf-8";
    }
}

scoped_refptr<base::RefCountedMemory> GetResource(int resource_id, const std::string &extension_id)
{
    const ui::ResourceBundle &rb = ui::ResourceBundle::GetSharedInstance();
    scoped_refptr<base::RefCountedMemory> bytes = rb.LoadDataResourceBytes(resource_id);
    auto *replacements = extensions::ExtensionsBrowserClient::Get()->GetComponentExtensionResourceManager()
            ? extensions::ExtensionsBrowserClient::Get()->GetComponentExtensionResourceManager()->GetTemplateReplacementsForExtension(
                      extension_id)
            : nullptr;

    if (replacements) {
        base::StringPiece input(reinterpret_cast<const char *>(bytes->front()), bytes->size());
        std::string temp_str = ui::ReplaceTemplateExpressions(input, *replacements);
        DCHECK(!temp_str.empty());
        return base::MakeRefCounted<base::RefCountedString>(std::move(temp_str));
    }
    return bytes;
}

// Loads an extension resource in a Chrome .pak file. These are used by
// component extensions.
class ResourceBundleFileLoader : public network::mojom::URLLoader
{
public:
    static void CreateAndStart(const network::ResourceRequest &request,
                               mojo::PendingReceiver<network::mojom::URLLoader> loader,
                               mojo::PendingRemote<network::mojom::URLLoaderClient> client_info,
                               const base::FilePath &filename, int resource_id,
                               scoped_refptr<net::HttpResponseHeaders> headers)
    {
        // Owns itself. Will live as long as its URLLoader and URLLoaderClientPtr
        // bindings are alive - essentially until either the client gives up or all
        // file data has been sent to it.
        auto *bundle_loader = new ResourceBundleFileLoader(std::move(headers));
        bundle_loader->Start(request, std::move(loader), std::move(client_info), filename, resource_id);
    }

    // mojom::URLLoader implementation:
    void FollowRedirect(const std::vector<std::string> &removed_headers,
                        const net::HttpRequestHeaders &modified_headers,
                        const net::HttpRequestHeaders &modified_cors_exempt_headers,
                        const absl::optional<GURL> &new_url) override
    {
        NOTREACHED() << "No redirects for local file loads.";
    }
    // Current implementation reads all resource data at start of resource
    // load, so priority, and pausing is not currently implemented.
    void SetPriority(net::RequestPriority priority, int32_t intra_priority_value) override {}
    void PauseReadingBodyFromNet() override {}
    void ResumeReadingBodyFromNet() override {}

private:
    ResourceBundleFileLoader(scoped_refptr<net::HttpResponseHeaders> headers)
        : response_headers_(std::move(headers))
    {
    }
    ~ResourceBundleFileLoader() override = default;

    void Start(const network::ResourceRequest &request,
               mojo::PendingReceiver<network::mojom::URLLoader> loader,
               mojo::PendingRemote<network::mojom::URLLoaderClient> client_info_remote,
               const base::FilePath &filename, int resource_id)
    {
        client_.Bind(std::move(client_info_remote));
        receiver_.Bind(std::move(loader));
        receiver_.set_disconnect_handler(base::BindOnce(
                &ResourceBundleFileLoader::OnReceiverError, base::Unretained(this)));
        client_.set_disconnect_handler(base::BindOnce(
                &ResourceBundleFileLoader::OnMojoDisconnect, base::Unretained(this)));
        auto data = GetResource(resource_id, request.url.host());

        std::string *read_mime_type = new std::string;
        base::ThreadPool::PostTaskAndReplyWithResult(
                FROM_HERE, { base::MayBlock() },
                base::BindOnce(&net::GetMimeTypeFromFile, filename, base::Unretained(read_mime_type)),
                base::BindOnce(&ResourceBundleFileLoader::OnMimeTypeRead, weak_factory_.GetWeakPtr(), std::move(data),
                               base::Owned(read_mime_type)));
    }

    void OnMimeTypeRead(scoped_refptr<base::RefCountedMemory> data, std::string *read_mime_type, bool read_result)
    {
        auto head = network::mojom::URLResponseHead::New();
        head->request_start = base::TimeTicks::Now();
        head->response_start = base::TimeTicks::Now();
        head->content_length = data->size();
        head->mime_type = *read_mime_type;
        DetermineCharset(head->mime_type, data.get(), &head->charset);
        mojo::ScopedDataPipeProducerHandle producer_handle;
        mojo::ScopedDataPipeConsumerHandle consumer_handle;
        if (mojo::CreateDataPipe(data->size(), producer_handle, consumer_handle) != MOJO_RESULT_OK) {
            client_->OnComplete(network::URLLoaderCompletionStatus(net::ERR_FAILED));
            client_.reset();
            MaybeDeleteSelf();
            return;
        }
        head->headers = response_headers_;
        head->headers->AddHeader(net::HttpRequestHeaders::kContentLength,
                                 base::NumberToString(head->content_length).c_str());
        if (!head->mime_type.empty()) {
            head->headers->AddHeader(net::HttpRequestHeaders::kContentType, head->mime_type.c_str());
        }
        client_->OnReceiveResponse(std::move(head), std::move(consumer_handle), absl::nullopt);

        uint32_t write_size = data->size();
        MojoResult result = producer_handle->WriteData(data->front(), &write_size, MOJO_WRITE_DATA_FLAG_NONE);
        OnFileWritten(result);
    }

    void OnMojoDisconnect()
    {
        client_.reset();
        MaybeDeleteSelf();
    }

    void OnReceiverError()
    {
        receiver_.reset();
        MaybeDeleteSelf();
    }

    void MaybeDeleteSelf()
    {
        if (!receiver_.is_bound() && !client_.is_bound())
            delete this;
    }

    void OnFileWritten(MojoResult result)
    {
        // All the data has been written now. The consumer will be notified that
        // there will be no more data to read from now.
        if (result == MOJO_RESULT_OK)
            client_->OnComplete(network::URLLoaderCompletionStatus(net::OK));
        else
            client_->OnComplete(network::URLLoaderCompletionStatus(net::ERR_FAILED));
        client_.reset();
        MaybeDeleteSelf();
    }

    mojo::Receiver<network::mojom::URLLoader> receiver_{this};
    mojo::Remote<network::mojom::URLLoaderClient> client_;
    scoped_refptr<net::HttpResponseHeaders> response_headers_;
    base::WeakPtrFactory<ResourceBundleFileLoader> weak_factory_{this};
};

} // namespace

namespace extensions {

// Copied from chrome/browser/extensions/chrome_extensions_browser_api_provider.(h|cc)
class ChromeExtensionsBrowserAPIProvider : public ExtensionsBrowserAPIProvider
{
public:
    ChromeExtensionsBrowserAPIProvider() = default;
    ~ChromeExtensionsBrowserAPIProvider() override = default;

    void RegisterExtensionFunctions(ExtensionFunctionRegistry *registry) override
    {
        // Generated APIs from Chrome.
        api::ChromeGeneratedFunctionRegistry::RegisterAll(registry);
    }

};

class QtWebEngineExtensionsBrowserAPIProvider : public ExtensionsBrowserAPIProvider
{
public:
    QtWebEngineExtensionsBrowserAPIProvider() = default;
    ~QtWebEngineExtensionsBrowserAPIProvider() override = default;

    void RegisterExtensionFunctions(ExtensionFunctionRegistry *registry) override
    {
        // Generated APIs from QtWebEngine.
        api::QtWebEngineGeneratedFunctionRegistry::RegisterAll(registry);
    }
};

ExtensionsBrowserClientQt::ExtensionsBrowserClientQt()
    : api_client_(new ExtensionsAPIClientQt)
    , resource_manager_(new ComponentExtensionResourceManagerQt)
{
    AddAPIProvider(std::make_unique<CoreExtensionsBrowserAPIProvider>());
    AddAPIProvider(std::make_unique<ChromeExtensionsBrowserAPIProvider>());
    AddAPIProvider(std::make_unique<QtWebEngineExtensionsBrowserAPIProvider>());
}

ExtensionsBrowserClientQt::~ExtensionsBrowserClientQt()
{
}

bool ExtensionsBrowserClientQt::IsShuttingDown()
{
    return false;
}

bool ExtensionsBrowserClientQt::AreExtensionsDisabled(const base::CommandLine &command_line, BrowserContext *context)
{
    return false;
}

bool ExtensionsBrowserClientQt::IsValidContext(BrowserContext *context)
{
    return true;
}

bool ExtensionsBrowserClientQt::IsSameContext(BrowserContext *first,
                                              BrowserContext *second)
{
    return first == second;
}

bool ExtensionsBrowserClientQt::HasOffTheRecordContext(BrowserContext *context)
{
    return false;
}

BrowserContext *ExtensionsBrowserClientQt::GetOffTheRecordContext(BrowserContext *context)
{
    // TODO(extensions): Do we need to support this?
    return nullptr;
}

BrowserContext *ExtensionsBrowserClientQt::GetOriginalContext(BrowserContext *context)
{
    return context;
}

BrowserContext *ExtensionsBrowserClientQt::GetRedirectedContextInIncognito(BrowserContext *context, bool, bool)
{
    // like in ShellExtensionsBrowserClient:
    return context;
}

BrowserContext *ExtensionsBrowserClientQt::GetContextForRegularAndIncognito(BrowserContext *context, bool, bool)
{
    // like in ShellExtensionsBrowserClient:
    return context;
}

BrowserContext *ExtensionsBrowserClientQt::GetRegularProfile(BrowserContext *context, bool, bool)
{
    // like in ShellExtensionsBrowserClient:
    return context;
}

bool ExtensionsBrowserClientQt::IsGuestSession(BrowserContext *context) const
{
    return context->IsOffTheRecord();
}

bool ExtensionsBrowserClientQt::IsExtensionIncognitoEnabled(const std::string &extension_id,
                                                            content::BrowserContext *context) const
{
    return false;
}

bool ExtensionsBrowserClientQt::CanExtensionCrossIncognito(const Extension *extension,
                                                           content::BrowserContext *context) const
{
    return false;
}

// Return the resource relative path and id for the given request.
base::FilePath ExtensionsBrowserClientQt::GetBundleResourcePath(const network::ResourceRequest &request,
                                                                const base::FilePath &extension_resources_path,
                                                                int *resource_id) const
{
    *resource_id = 0;
    // |chrome_resources_path| corresponds to src/chrome/browser/resources in
    // source tree.
    base::FilePath resources_path;
    if (!base::PathService::Get(base::DIR_QT_LIBRARY_DATA, &resources_path))
        return base::FilePath();

    // Since component extension resources are included in
    // component_extension_resources.pak file in |chrome_resources_path|,
    // calculate the extension |request_relative_path| against
    // |chrome_resources_path|.
    if (!resources_path.IsParent(extension_resources_path))
        return base::FilePath();

    const base::FilePath request_relative_path =
            extensions::file_util::ExtensionURLToRelativeFilePath(request.url);
    if (!ExtensionsBrowserClient::Get()->GetComponentExtensionResourceManager()->IsComponentExtensionResource(
                extension_resources_path, request_relative_path, resource_id)) {
        return base::FilePath();
    }
    DCHECK_NE(0, *resource_id);

    return request_relative_path;
}

// Creates and starts a URLLoader to load an extension resource from the
// embedder's resource bundle (.pak) files. Used for component extensions.
void ExtensionsBrowserClientQt::LoadResourceFromResourceBundle(const network::ResourceRequest &request,
                                                               mojo::PendingReceiver<network::mojom::URLLoader> loader,
                                                               const base::FilePath &resource_relative_path,
                                                               int resource_id,
                                                               scoped_refptr<net::HttpResponseHeaders> headers,
                                                               mojo::PendingRemote<network::mojom::URLLoaderClient> client)
{
    ResourceBundleFileLoader::CreateAndStart(request, std::move(loader), std::move(client), resource_relative_path,
                                             resource_id, headers);
}


bool ExtensionsBrowserClientQt::AllowCrossRendererResourceLoad(const network::ResourceRequest &request,
                                                               network::mojom::RequestDestination destination,
                                                               ui::PageTransition page_transition,
                                                               int child_id,
                                                               bool is_incognito,
                                                               const Extension *extension,
                                                               const ExtensionSet &extensions,
                                                               const ProcessMap &process_map)
{
    if (extension && extension->id() == extension_misc::kPdfExtensionId)
        return true;

    // hangout services id
    if (extension && extension->id() == "nkeimhogjdpnpccoofpliimaahmaaome")
        return true;

    bool allowed = false;
    if (url_request_util::AllowCrossRendererResourceLoad(request, destination,
                                                         page_transition, child_id,
                                                         is_incognito, extension, extensions,
                                                         process_map, &allowed)) {
        return allowed;
    }
    // Couldn't determine if resource is allowed. Block the load.
    return false;
}

PrefService *ExtensionsBrowserClientQt::GetPrefServiceForContext(BrowserContext *context)
{
    return static_cast<Profile *>(context)->GetPrefs();
}

void ExtensionsBrowserClientQt::GetEarlyExtensionPrefsObservers(content::BrowserContext *context,
                                                                std::vector<EarlyExtensionPrefsObserver *> *observers) const
{
}

ProcessManagerDelegate *ExtensionsBrowserClientQt::GetProcessManagerDelegate() const
{
    return nullptr;
}

std::unique_ptr<ExtensionHostDelegate> ExtensionsBrowserClientQt::CreateExtensionHostDelegate()
{
    return std::unique_ptr<ExtensionHostDelegate>(new ExtensionHostDelegateQt);
}

bool ExtensionsBrowserClientQt::DidVersionUpdate(BrowserContext *context)
{
    // TODO(jamescook): We might want to tell extensions when app_shell updates.
    return false;
}

void ExtensionsBrowserClientQt::PermitExternalProtocolHandler()
{
}

bool ExtensionsBrowserClientQt::IsRunningInForcedAppMode()
{
    return false;
}

bool ExtensionsBrowserClientQt::IsLoggedInAsPublicAccount()
{
    return false;
}

ExtensionSystemProvider *ExtensionsBrowserClientQt::GetExtensionSystemFactory()
{
    return ExtensionSystemFactoryQt::GetInstance();
}

void ExtensionsBrowserClientQt::RegisterBrowserInterfaceBindersForFrame(
        mojo::BinderMapWithContext<content::RenderFrameHost*> *binder_map,
        content::RenderFrameHost* render_frame_host,
        const Extension* extension) const
{
    PopulateExtensionFrameBinders(binder_map, render_frame_host, extension);
}

std::unique_ptr<RuntimeAPIDelegate> ExtensionsBrowserClientQt::CreateRuntimeAPIDelegate(content::BrowserContext *context) const
{
    // TODO(extensions): Implement to support Apps.
    NOTREACHED();
    return std::unique_ptr<RuntimeAPIDelegate>();
}

const ComponentExtensionResourceManager *ExtensionsBrowserClientQt::GetComponentExtensionResourceManager()
{
    return resource_manager_.get();
}

void ExtensionsBrowserClientQt::BroadcastEventToRenderers(events::HistogramValue histogram_value,
                                                          const std::string &event_name,
                                                          base::Value::List args,
                                                          bool dispatch_to_off_the_record_profiles)
{
    NOTIMPLEMENTED();
    // TODO : do the event routing
    // event_router_forwarder_->BroadcastEventToRenderers(
    //     histogram_value, event_name, std::move(args), GURL());
}

ExtensionCache *ExtensionsBrowserClientQt::GetExtensionCache()
{
    // Only used by Chrome via ExtensionService.
    NOTREACHED();
    return nullptr;
}

bool ExtensionsBrowserClientQt::IsBackgroundUpdateAllowed()
{
    return true;
}

bool ExtensionsBrowserClientQt::IsMinBrowserVersionSupported(const std::string &min_version)
{
    return true;
}

bool ExtensionsBrowserClientQt::IsLockScreenContext(content::BrowserContext *context)
{
    return false;
}

// Returns the locale used by the application.
std::string ExtensionsBrowserClientQt::GetApplicationLocale()
{
    return WebEngineLibraryInfo::getApplicationLocale();
}

bool ExtensionsBrowserClientQt::IsAppModeForcedForApp(const ExtensionId &id)
{
    return false;
}

bool ExtensionsBrowserClientQt::IsInDemoMode()
{
    return false;
}

ExtensionWebContentsObserver *ExtensionsBrowserClientQt::GetExtensionWebContentsObserver(content::WebContents *web_contents)
{
    return ExtensionWebContentsObserverQt::FromWebContents(web_contents);
}

KioskDelegate *ExtensionsBrowserClientQt::GetKioskDelegate()
{
    return nullptr;
}

bool ExtensionsBrowserClientQt::IsScreensaverInDemoMode(const std::string &app_id)
{
    return false;
}

void ExtensionsBrowserClientQt::SetAPIClientForTest(ExtensionsAPIClient *api_client)
{
    api_client_.reset(api_client);
}

} // namespace extensions
