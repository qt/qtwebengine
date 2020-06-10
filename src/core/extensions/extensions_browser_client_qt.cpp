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

#include "extensions_browser_client_qt.h"

#include <utility>

#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/task/post_task.h"
#include "base/memory/ref_counted_memory.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "extensions/browser/api/extensions_api_client.h"
#include "extensions/browser/api/runtime/runtime_api_delegate.h"
#include "extensions/browser/app_sorting.h"
#include "extensions/browser/core_extensions_browser_api_provider.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_host_delegate.h"
#include "extensions/browser/extension_protocols.h"
#include "extensions/browser/extensions_browser_interface_binders.h"
#include "extensions/browser/url_request_util.h"
#include "extensions/common/file_util.h"
#include "net/base/completion_once_callback.h"
#include "net/base/mime_util.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "third_party/zlib/google/compression_utils.h"
#include "ui/base/resource/resource_bundle.h"

#include "component_extension_resource_manager_qt.h"
#include "extension_system_factory_qt.h"
#include "extension_web_contents_observer_qt.h"
#include "extensions_api_client_qt.h"
#include "extensions_browser_client_qt.h"
#include "web_engine_library_info.h"

using content::BrowserContext;
using content::BrowserThread;

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

    bool is_gzipped = rb.IsGzipped(resource_id);
    if (!bytes->size() || (!replacements && !is_gzipped)) {
        return bytes;
    }

    base::StringPiece input(reinterpret_cast<const char *>(bytes->front()), bytes->size());

    std::string temp_str;

    base::StringPiece source = input;
    if (is_gzipped) {
        temp_str.resize(compression::GetUncompressedSize(input));
        source = temp_str;
        CHECK(compression::GzipUncompress(input, source));
    }

    if (replacements) {
        temp_str = ui::ReplaceTemplateExpressions(source, *replacements);
    }

    DCHECK(!temp_str.empty());

    return base::RefCountedString::TakeString(&temp_str);
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
                               const std::string &content_security_policy, bool send_cors_header)
    {
        // Owns itself. Will live as long as its URLLoader and URLLoaderClientPtr
        // bindings are alive - essentially until either the client gives up or all
        // file data has been sent to it.
        auto *bundle_loader = new ResourceBundleFileLoader(content_security_policy, send_cors_header);
        bundle_loader->Start(request, std::move(loader), std::move(client_info), filename, resource_id);
    }

    // mojom::URLLoader implementation:
    void FollowRedirect(const std::vector<std::string> &removed_headers,
                        const net::HttpRequestHeaders &modified_headers, const base::Optional<GURL> &new_url) override
    {
        NOTREACHED() << "No redirects for local file loads.";
    }
    // Current implementation reads all resource data at start of resource
    // load, so priority, and pausing is not currently implemented.
    void SetPriority(net::RequestPriority priority, int32_t intra_priority_value) override {}
    void PauseReadingBodyFromNet() override {}
    void ResumeReadingBodyFromNet() override {}

private:
    ResourceBundleFileLoader(const std::string &content_security_policy, bool send_cors_header)
    {
        response_headers_ = extensions::BuildHttpHeaders(content_security_policy, send_cors_header, base::Time());
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
        base::PostTaskAndReplyWithResult(
                FROM_HERE, { base::ThreadPool(), base::MayBlock() },
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
        mojo::DataPipe pipe(data->size());
        if (!pipe.consumer_handle.is_valid()) {
            client_->OnComplete(network::URLLoaderCompletionStatus(net::ERR_FAILED));
            client_.reset();
            MaybeDeleteSelf();
            return;
        }
        head->headers = response_headers_;
        head->headers->AddHeader(base::StringPrintf("%s: %s", net::HttpRequestHeaders::kContentLength,
                                                   base::NumberToString(head->content_length).c_str()));
        if (!head->mime_type.empty()) {
            head->headers->AddHeader(
                    base::StringPrintf("%s: %s", net::HttpRequestHeaders::kContentType, head->mime_type.c_str()));
        }
        client_->OnReceiveResponse(std::move(head));
        client_->OnStartLoadingResponseBody(std::move(pipe.consumer_handle));

        uint32_t write_size = data->size();
        MojoResult result = pipe.producer_handle->WriteData(data->front(), &write_size, MOJO_WRITE_DATA_FLAG_NONE);
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

    DISALLOW_COPY_AND_ASSIGN(ResourceBundleFileLoader);
};

} // namespace

namespace extensions {

ExtensionsBrowserClientQt::ExtensionsBrowserClientQt()
    : api_client_(new ExtensionsAPIClientQt)
    , resource_manager_(new ComponentExtensionResourceManagerQt)
{
    AddAPIProvider(std::make_unique<CoreExtensionsBrowserAPIProvider>());
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

bool ExtensionsBrowserClientQt::IsGuestSession(BrowserContext *context) const
{
    return false;
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
                                                               const std::string &content_security_policy,
                                                               mojo::PendingRemote<network::mojom::URLLoaderClient> client,
                                                               bool send_cors_header)
{
    ResourceBundleFileLoader::CreateAndStart(request, std::move(loader), std::move(client), resource_relative_path,
                                             resource_id, content_security_policy, send_cors_header);
}


bool ExtensionsBrowserClientQt::AllowCrossRendererResourceLoad(const GURL &url,
                                                               blink::mojom::ResourceType resource_type,
                                                               ui::PageTransition page_transition,
                                                               int child_id,
                                                               bool is_incognito,
                                                               const Extension *extension,
                                                               const ExtensionSet &extensions,
                                                               const ProcessMap &process_map)
{

    if (extension && extension->id() == extension_misc::kPdfExtensionId)
        return true;

    bool allowed = false;
    if (url_request_util::AllowCrossRendererResourceLoad(url, resource_type,
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
    // TODO(extensions): Implement to support Apps.
    NOTREACHED();
    return std::unique_ptr<ExtensionHostDelegate>();
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
        service_manager::BinderMapWithContext<content::RenderFrameHost*> *binder_map,
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
                                                          std::unique_ptr<base::ListValue> args, bool dispatch_to_off_the_record_profiles)
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
    NOTREACHED();
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
