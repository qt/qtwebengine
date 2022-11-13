// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on content/shell/browser/shell_devtools_frontend.cc:
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "devtools_frontend_qt.h"

#include "profile_adapter.h"
#include "profile_qt.h"
#include "web_contents_adapter.h"

#include "base/base64.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/json/string_escape.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/post_task.h"
#include "base/values.h"
#include "chrome/browser/devtools/devtools_eye_dropper.h"
#include "chrome/common/url_constants.h"
#include "components/prefs/in_memory_pref_store.h"
#include "components/prefs/json_pref_store.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/devtools_frontend_host.h"
#include "content/public/browser/file_url_loader.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/shared_cors_origin_access_list.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_client.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/url_utils.h"
#include "ipc/ipc_channel.h"
#include "net/http/http_response_headers.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/cpp/simple_url_loader_stream_consumer.h"

using namespace QtWebEngineCore;

namespace {

constexpr char kScreencastEnabled[] = "screencastEnabled";

base::DictionaryValue BuildObjectForResponse(const net::HttpResponseHeaders *rh,
                                             bool success, int net_error)
{
    base::DictionaryValue response;
    int responseCode = 200;
    if (rh) {
        responseCode = rh->response_code();
    } else if (!success) {
        // In case of no headers, assume file:// URL and failed to load
        responseCode = 404;
    }
    response.SetInteger("statusCode", responseCode);
    response.SetInteger("netError", net_error);
    response.SetString("netErrorName", net::ErrorToString(net_error));

    auto headers = std::make_unique<base::DictionaryValue>();
    size_t iterator = 0;
    std::string name;
    std::string value;
    // TODO(caseq): this probably needs to handle duplicate header names
    // correctly by folding them.
    while (rh && rh->EnumerateHeaderLines(&iterator, &name, &value))
        headers->SetString(name, value);

    response.Set("headers", std::move(headers));
    return response;
}

static std::string GetFrontendURL()
{
    return "devtools://devtools/bundled/inspector.html";
}

}  // namespace

namespace QtWebEngineCore {

class DevToolsFrontendQt::NetworkResourceLoader
        : public network::SimpleURLLoaderStreamConsumer {
public:
    NetworkResourceLoader(int stream_id,
                          int request_id,
                          DevToolsFrontendQt *bindings,
                          std::unique_ptr<network::SimpleURLLoader> loader,
                          network::mojom::URLLoaderFactory *url_loader_factory)
        : stream_id_(stream_id),
          request_id_(request_id),
          bindings_(bindings),
          loader_(std::move(loader))
    {
        loader_->SetOnResponseStartedCallback(base::BindOnce(
                                                  &NetworkResourceLoader::OnResponseStarted, base::Unretained(this)));
        loader_->DownloadAsStream(url_loader_factory, this);
    }

private:
    void OnResponseStarted(const GURL &final_url,
                           const network::mojom::URLResponseHead &response_head)
    {
        response_headers_ = response_head.headers;
    }

    void OnDataReceived(base::StringPiece chunk, base::OnceClosure resume) override
    {
        base::Value chunkValue;

        bool encoded = !base::IsStringUTF8(chunk);
        if (encoded) {
            std::string encoded_string;
            base::Base64Encode(chunk, &encoded_string);
            chunkValue = base::Value(std::move(encoded_string));
        } else {
            chunkValue = base::Value(chunk);
        }
        base::Value id(stream_id_);
        base::Value encodedValue(encoded);

        bindings_->CallClientFunction("DevToolsAPI", "streamWrite", std::move(id), std::move(chunkValue), std::move(encodedValue));
        std::move(resume).Run();
    }

    void OnComplete(bool success) override
    {
        auto response = BuildObjectForResponse(response_headers_.get(), success, loader_->NetError());
        bindings_->SendMessageAck(request_id_, std::move(response));
        bindings_->m_loaders.erase(bindings_->m_loaders.find(this));
    }

    void OnRetry(base::OnceClosure start_retry) override { NOTREACHED(); }

    const int stream_id_;
    const int request_id_;
    DevToolsFrontendQt *const bindings_;
    std::unique_ptr<network::SimpleURLLoader> loader_;
    scoped_refptr<net::HttpResponseHeaders> response_headers_;
};

// This constant should be in sync with
// the constant at devtools_ui_bindings.cc.
const size_t kMaxMessageChunkSize = IPC::Channel::kMaximumMessageSize / 4;

// static
DevToolsFrontendQt *DevToolsFrontendQt::Show(QSharedPointer<WebContentsAdapter> frontendAdapter, content::WebContents *inspectedContents)
{
    DCHECK(frontendAdapter);
    DCHECK(inspectedContents);

    if (!frontendAdapter->isInitialized()) {
        scoped_refptr<content::SiteInstance> site =
            content::SiteInstance::CreateForURL(frontendAdapter->profile(), GURL(GetFrontendURL()));
        frontendAdapter->initialize(site.get());
    }

    frontendAdapter->setInspector(true);

    content::WebContents *contents = frontendAdapter->webContents();
    if (contents == inspectedContents) {
        LOG(WARNING) << "You can not inspect yourself";
        return nullptr;
    }

    DevToolsFrontendQt *devtoolsFrontend = new DevToolsFrontendQt(frontendAdapter, inspectedContents);

    if (contents->GetURL() == GURL(GetFrontendURL())) {
        contents->GetController().Reload(content::ReloadType::ORIGINAL_REQUEST_URL, false);
    }  else {
        content::NavigationController::LoadURLParams loadParams((GURL(GetFrontendURL())));
        loadParams.transition_type = ui::PageTransitionFromInt(ui::PAGE_TRANSITION_AUTO_TOPLEVEL | ui::PAGE_TRANSITION_FROM_API);
        contents->GetController().LoadURLWithParams(loadParams);
    }

    return devtoolsFrontend;
}

DevToolsFrontendQt::DevToolsFrontendQt(QSharedPointer<WebContentsAdapter> webContentsAdapter,
                                       content::WebContents *inspectedContents)
    : content::WebContentsObserver(webContentsAdapter->webContents())
    , m_frontendAdapter(webContentsAdapter)
    , m_inspectedAdapter(static_cast<WebContentsDelegateQt *>(inspectedContents->GetDelegate())
                                 ->webContentsAdapter())
    , m_inspectedContents(inspectedContents)
    , m_inspect_element_at_x(-1)
    , m_inspect_element_at_y(-1)
    , m_prefStore(nullptr)
    , m_weakFactory(this)
{
    // We use a separate prefstore than one in ProfileQt, because that one is in-memory only, and this
    // needs to be stored or it will show introduction text on every load.
    if (webContentsAdapter->profileAdapter()->isOffTheRecord())
        m_prefStore = scoped_refptr<PersistentPrefStore>(new InMemoryPrefStore());
    else
        CreateJsonPreferences(false);

    m_frontendDelegate = static_cast<WebContentsDelegateQt *>(webContentsAdapter->webContents()->GetDelegate());
}


DevToolsFrontendQt::~DevToolsFrontendQt()
{
    if (QSharedPointer<WebContentsAdapter> p = m_frontendAdapter)
        p->setInspector(false);
}

void DevToolsFrontendQt::Activate()
{
    m_frontendDelegate->ActivateContents(web_contents());
}

void DevToolsFrontendQt::Focus()
{
    web_contents()->Focus();
}

void DevToolsFrontendQt::InspectElementAt(int x, int y)
{
    if (m_agentHost)
        m_agentHost->InspectElement(m_inspectedContents->GetFocusedFrame(), x, y);
    else {
        m_inspect_element_at_x = x;
        m_inspect_element_at_y = y;
    }
}

void DevToolsFrontendQt::Close()
{
    // Don't close the webContents, it might be reused, but pretend it was
    WebContentsDestroyed();
}

void DevToolsFrontendQt::DisconnectFromTarget()
{
    if (!m_agentHost)
        return;
    m_agentHost->DetachClient(this);
    m_agentHost = nullptr;
}

void DevToolsFrontendQt::ReadyToCommitNavigation(content::NavigationHandle *navigationHandle)
{
    // ShellDevToolsFrontend does this in RenderViewCreated,
    // but that doesn't work for us for some reason.
    content::RenderFrameHost *frame = navigationHandle->GetRenderFrameHost();
    if (navigationHandle->IsInMainFrame()) {
        // If the frontend for some reason goes to some place other than devtools, stop the bindings
        if (navigationHandle->GetURL() != GetFrontendURL())
            m_frontendHost.reset(nullptr);
        else if (!m_frontendHost)
            m_frontendHost = content::DevToolsFrontendHost::Create(
                        frame,
                        base::BindRepeating(&DevToolsFrontendQt::HandleMessageFromDevToolsFrontend,
                                            base::Unretained(this)));
    }
}

void DevToolsFrontendQt::DocumentOnLoadCompletedInPrimaryMainFrame()
{
    if (!m_inspectedContents)
        return;
    // Don't call AttachClient multiple times for the same DevToolsAgentHost.
    // Otherwise it will call AgentHostClosed which closes the DevTools window.
    // This may happen in cases where the DevTools content fails to load.
    scoped_refptr<content::DevToolsAgentHost> agent_host =
            content::DevToolsAgentHost::GetOrCreateFor(m_inspectedContents);
    if (agent_host != m_agentHost) {
        if (m_agentHost)
            m_agentHost->DetachClient(this);
        m_agentHost = agent_host;
        m_agentHost->AttachClient(this);
        if (m_inspect_element_at_x != -1) {
            m_agentHost->InspectElement(m_inspectedContents->GetFocusedFrame(), m_inspect_element_at_x, m_inspect_element_at_y);
            m_inspect_element_at_x = -1;
            m_inspect_element_at_y = -1;
        }
    }
}

void DevToolsFrontendQt::WebContentsDestroyed()
{
    if (m_inspectedAdapter)
        m_inspectedAdapter->devToolsFrontendDestroyed(this);

    if (m_agentHost) {
        m_agentHost->DetachClient(this);
        m_agentHost = nullptr;
    }
    delete this;
}

void DevToolsFrontendQt::SetPreference(const std::string &name, const std::string &value)
{
    DCHECK(m_prefStore);
    m_prefStore->SetValue(name, base::WrapUnique(new base::Value(value)), 0);
}

void DevToolsFrontendQt::RemovePreference(const std::string &name)
{
    DCHECK(m_prefStore);
    m_prefStore->RemoveValue(name, 0);
}

void DevToolsFrontendQt::ClearPreferences()
{
    ProfileQt *profile = static_cast<ProfileQt *>(web_contents()->GetBrowserContext());
    if (profile->IsOffTheRecord() || profile->profileAdapter()->storageName().isEmpty())
        m_prefStore = scoped_refptr<PersistentPrefStore>(new InMemoryPrefStore());
    else
        CreateJsonPreferences(true);
}

void DevToolsFrontendQt::CreateJsonPreferences(bool clear)
{
    content::BrowserContext *browserContext = web_contents()->GetBrowserContext();
    DCHECK(!browserContext->IsOffTheRecord());
    JsonPrefStore *jsonPrefStore = new JsonPrefStore(
                browserContext->GetPath().Append(FILE_PATH_LITERAL("devtoolsprefs.json")));
    // We effectively clear the preferences by not calling ReadPrefs
    base::ScopedAllowBlockingForTesting allowBlocking;
    if (!clear)
        jsonPrefStore->ReadPrefs();

    m_prefStore = scoped_refptr<PersistentPrefStore>(jsonPrefStore);
}

void DevToolsFrontendQt::HandleMessageFromDevToolsFrontend(base::Value message)
{
    const std::string *method_ptr = nullptr;
    base::Value *params_value = nullptr;
    if (message.is_dict()) {
        method_ptr = message.FindStringKey("method");
        params_value = message.FindKey("params");
    }
    if (!method_ptr || (params_value && !params_value->is_list())) {
        LOG(ERROR) << "Invalid message was sent to embedder: " << message;
        return;
    }
    base::Value empty_params(base::Value::Type::LIST);
    if (!params_value)
        params_value = &empty_params;

    int request_id = message.FindIntKey("id").value_or(0);
    const std::string &method = *method_ptr;
    base::Value::List *paramsPtr;
    if (params_value)
        paramsPtr = params_value->GetIfList();
    base::Value::List &params = *paramsPtr;

    if (method == "dispatchProtocolMessage" && params.size() == 1) {
        const std::string *protocol_message = params[0].GetIfString();
        if (!protocol_message)
            return;
        if (m_agentHost)
            m_agentHost->DispatchProtocolMessage(this, base::as_bytes(base::make_span(*protocol_message)));
    } else if (method == "loadCompleted") {
        web_contents()->GetMainFrame()->ExecuteJavaScript(u"DevToolsAPI.setUseSoftMenu(true);",
                                                          base::NullCallback());
    } else if (method == "loadNetworkResource" && params.size() == 3) {
        // TODO(pfeldman): handle some of the embedder messages in content.
        const std::string *url = params[0].GetIfString();
        const std::string *headers = params[1].GetIfString();
        absl::optional<int> stream_id = params[2].GetIfInt();
        if (!url || !headers || !stream_id.has_value()) {
            return;
        }

        GURL gurl(*url);
        if (!gurl.is_valid()) {
            base::DictionaryValue response;
            response.SetInteger("statusCode", 404);
            response.SetBoolean("urlValid", false);
            SendMessageAck(request_id, std::move(response));
            return;
        }

        net::NetworkTrafficAnnotationTag traffic_annotation =
            net::DefineNetworkTrafficAnnotation(
                "devtools_handle_front_end_messages", R"(
                semantics {
                  sender: "Developer Tools"
                  description:
                    "When user opens Developer Tools, the browser may fetch "
                    "additional resources from the network to enrich the debugging "
                    "experience (e.g. source map resources)."
                  trigger: "User opens Developer Tools to debug a web page."
                  data: "Any resources requested by Developer Tools."
                  destination: OTHER
                }
                policy {
                  cookies_allowed: YES
                  cookies_store: "user"
                  setting:
                    "It's not possible to disable this feature from settings."
                  chrome_policy {
                    DeveloperToolsAvailability {
                      policy_options {mode: MANDATORY}
                      DeveloperToolsAvailability: 2
                    }
                  }
                })");
        auto resource_request = std::make_unique<network::ResourceRequest>();
        resource_request->url = gurl;
        // TODO(caseq): this preserves behavior of URLFetcher-based implementation.
        // We really need to pass proper first party origin from the front-end.
        resource_request->site_for_cookies = net::SiteForCookies::FromUrl(gurl);
        resource_request->headers.AddHeadersFromString(*headers);

        mojo::Remote<network::mojom::URLLoaderFactory> file_url_loader_factory;
        scoped_refptr<network::SharedURLLoaderFactory> network_url_loader_factory;
        network::mojom::URLLoaderFactory *url_loader_factory;
        if (gurl.SchemeIsFile()) {
            file_url_loader_factory.Bind(content::CreateFileURLLoaderFactory(base::FilePath(), nullptr));
            url_loader_factory = file_url_loader_factory.get();
        } else if (content::HasWebUIScheme(gurl)) {
            base::DictionaryValue response;
            response.SetInteger("statusCode", 403);
            SendMessageAck(request_id, std::move(response));
            return;
        } else {
            auto *partition = web_contents()->GetBrowserContext()->GetStoragePartitionForUrl(gurl);
            network_url_loader_factory = partition->GetURLLoaderFactoryForBrowserProcess();
            url_loader_factory = network_url_loader_factory.get();
        }
        auto simple_url_loader = network::SimpleURLLoader::Create(
                    std::move(resource_request), traffic_annotation);
        auto resource_loader = std::make_unique<NetworkResourceLoader>(
                    *stream_id, request_id, this, std::move(simple_url_loader),
                    url_loader_factory);
        m_loaders.insert(std::move(resource_loader));
        return;
    } else if (method == "getPreferences") {
        // Screencast is enabled by default if it's not present in the preference store.
        if (!m_prefStore->GetValue(kScreencastEnabled, NULL))
            SetPreference(kScreencastEnabled, "false");

        m_preferences = std::move(*m_prefStore->GetValues());
        SendMessageAck(request_id, m_preferences.Clone());
        return;
    } else if (method == "setPreference" && params.size() >= 2) {
        const std::string *name = params[0].GetIfString();
        const std::string *value = params[1].GetIfString();
        if (!name || !value)
            return;
        SetPreference(*name, *value);
    } else if (method == "removePreference" && params.size() >= 1) {
        const std::string *name = params[0].GetIfString();
        if (!name)
            return;
        RemovePreference(*name);
    } else if (method == "clearPreferences") {
        ClearPreferences();
    } else if (method == "requestFileSystems") {
        web_contents()->GetMainFrame()->ExecuteJavaScript(u"DevToolsAPI.fileSystemsLoaded([]);",
                                                          base::NullCallback());
    } else if (method == "reattach") {
        if (!m_agentHost)
            return;
        m_agentHost->DetachClient(this);
        m_agentHost->AttachClient(this);
    } else if (method == "inspectedURLChanged" && params.size() >= 1) {
        const std::string *url = params[0].GetIfString();
        if (!url)
            return;
        const std::string kHttpPrefix = "http://";
        const std::string kHttpsPrefix = "https://";
        const std::string simplified_url =
            base::StartsWith(*url, kHttpsPrefix, base::CompareCase::SENSITIVE)
                ? url->substr(kHttpsPrefix.length())
                : base::StartsWith(*url, kHttpPrefix, base::CompareCase::SENSITIVE)
                      ? url->substr(kHttpPrefix.length())
                      : *url;
        // DevTools UI is not localized.
        web_contents()->UpdateTitleForEntry(web_contents()->GetController().GetActiveEntry(),
                                            base::UTF8ToUTF16(
                                                base::StringPrintf("DevTools - %s", simplified_url.c_str())));
    } else if (method == "openInNewTab" && params.size() >= 1) {
        const std::string *urlString = params[0].GetIfString();
        if (!urlString)
            return;
        GURL url(*urlString);
        if (!url.is_valid())
            return;
        content::OpenURLParams openParams(GURL(url),
                                          content::Referrer(),
                                          WindowOpenDisposition::NEW_FOREGROUND_TAB,
                                          ui::PAGE_TRANSITION_LINK,
                                          false);
        // OpenURL will (via WebContentsDelegateQt::OpenURLFromTab) call
        // application code, which may decide to close this devtools view (see
        // quicknanobrowser for example).
        //
        // Chromium always calls SendMessageAck through a callback bound to a
        // WeakPtr, we do the same here, except without the callback.
        base::WeakPtr<DevToolsFrontendQt> weakThis = m_weakFactory.GetWeakPtr();
        web_contents()->OpenURL(openParams);
        if (!weakThis)
            return;
    } else if (method == "bringToFront") {
        Activate();
    } else if (method == "closeWindow") {
        web_contents()->Close();
    } else if (method == "setEyeDropperActive" && params.size() == 1) {
        absl::optional<bool> active = params[0].GetIfBool();
        if (!active)
            return;
        SetEyeDropperActive(*active);
    } else {
        VLOG(1) << "Unimplemented devtools method: " << message;
        return;
    }

    if (request_id)
        SendMessageAck(request_id, base::Value());
}

void DevToolsFrontendQt::SetEyeDropperActive(bool active)
{
    if (!m_inspectedContents)
        return;
    if (active) {
        m_eyeDropper.reset(new DevToolsEyeDropper(
                               m_inspectedContents,
                               base::BindRepeating(&DevToolsFrontendQt::ColorPickedInEyeDropper,
                                                   base::Unretained(this))));
    } else {
        m_eyeDropper.reset();
    }
}

void DevToolsFrontendQt::ColorPickedInEyeDropper(int r, int g, int b, int a)
{
    base::DictionaryValue color;
    color.SetInteger("r", r);
    color.SetInteger("g", g);
    color.SetInteger("b", b);
    color.SetInteger("a", a);
    CallClientFunction("DevToolsAPI", "eyeDropperPickedColor", std::move(color));
}

void DevToolsFrontendQt::DispatchProtocolMessage(content::DevToolsAgentHost *agentHost, base::span<const uint8_t> message)
{
    Q_UNUSED(agentHost);
    base::StringPiece str_message(reinterpret_cast<const char*>(message.data()), message.size());

    if (str_message.length() < kMaxMessageChunkSize) {
        CallClientFunction("DevToolsAPI", "dispatchMessage",
                           base::Value(std::string(str_message)));
    } else {
        size_t total_size = str_message.length();
        for (size_t pos = 0; pos < str_message.length(); pos += kMaxMessageChunkSize) {
            base::StringPiece str_message_chunk = str_message.substr(pos, kMaxMessageChunkSize);

            CallClientFunction("DevToolsAPI", "dispatchMessageChunk",
                               base::Value(std::string(str_message_chunk)),
                               base::Value(base::NumberToString(pos ? 0 : total_size)));
        }
    }
}

void DevToolsFrontendQt::CallClientFunction(const std::string &object_name,
                                            const std::string &method_name,
                                            base::Value arg1, base::Value arg2, base::Value arg3,
                                            base::OnceCallback<void(base::Value)> cb)

{
    base::Value::List arguments;
    if (!arg1.is_none()) {
        arguments.Append(std::move(arg1));
        if (!arg2.is_none()) {
            arguments.Append(std::move(arg2));
            if (!arg3.is_none()) {
                arguments.Append(std::move(arg3));
            }
        }
    }
    web_contents()->GetMainFrame()->ExecuteJavaScriptMethod(base::ASCIIToUTF16(object_name),
                                                            base::ASCIIToUTF16(method_name),
                                                            std::move(arguments),
                                                            std::move(cb));

}

void DevToolsFrontendQt::SendMessageAck(int request_id, base::Value arg)
{
    base::Value id_value(request_id);
    CallClientFunction("DevToolsAPI", "embedderMessageAck", std::move(id_value), std::move(arg));
}

void DevToolsFrontendQt::AgentHostClosed(content::DevToolsAgentHost *agentHost)
{
    DCHECK(agentHost == m_agentHost.get());
    m_agentHost = nullptr;
    m_inspectedContents = nullptr;
    m_inspectedAdapter = nullptr;
    Close();
}

} // namespace QtWebEngineCore
