// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/plugins/plugin_response_interceptor_url_loader_throttle.cc
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plugin_response_interceptor_url_loader_throttle.h"

#include "base/bind.h"
#include "base/guid.h"
#include "chrome/browser/extensions/api/streams_private/streams_private_api.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/download_request_utils.h"
#include "content/public/browser/download_utils.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_attach_helper.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handlers/mime_types_handler.h"
#include "third_party/blink/public/mojom/loader/transferrable_url_loader.mojom.h"

#include "extensions/extension_system_qt.h"
#include "web_contents_delegate_qt.h"
#include "web_engine_settings.h"

#include <string>
#include <tuple>

namespace {
void ClearAllButFrameAncestors(network::mojom::URLResponseHead *response_head)
{
    response_head->headers->RemoveHeader("Content-Security-Policy");
    response_head->headers->RemoveHeader("Content-Security-Policy-Report-Only");

    if (!response_head->parsed_headers)
        return;

    std::vector<network::mojom::ContentSecurityPolicyPtr> &csp =
        response_head->parsed_headers->content_security_policy;
    std::vector<network::mojom::ContentSecurityPolicyPtr> cleared;

    for (auto &policy : csp) {
        auto frame_ancestors = policy->directives.find(network::mojom::CSPDirectiveName::FrameAncestors);
        if (frame_ancestors == policy->directives.end())
            continue;

        auto cleared_policy = network::mojom::ContentSecurityPolicy::New();
        cleared_policy->self_origin = std::move(policy->self_origin);
        cleared_policy->header = std::move(policy->header);
        cleared_policy->header->header_value = "";
        cleared_policy->directives[network::mojom::CSPDirectiveName::FrameAncestors] = std::move(frame_ancestors->second);

        auto raw_frame_ancestors = policy->raw_directives.find(network::mojom::CSPDirectiveName::FrameAncestors);
        DCHECK(raw_frame_ancestors != policy->raw_directives.end());

        cleared_policy->header->header_value = "frame-ancestors " + raw_frame_ancestors->second;
        response_head->headers->AddHeader(
            cleared_policy->header->type == network::mojom::ContentSecurityPolicyType::kEnforce
                ? "Content-Security-Policy"
                : "Content-Security-Policy-Report-Only",
            cleared_policy->header->header_value);
        cleared_policy->raw_directives[network::mojom::CSPDirectiveName::FrameAncestors] =
            std::move(raw_frame_ancestors->second);

        cleared.push_back(std::move(cleared_policy));
    }

    csp.swap(cleared);
}
}  // namespace


namespace QtWebEngineCore {

PluginResponseInterceptorURLLoaderThrottle::PluginResponseInterceptorURLLoaderThrottle(
        network::mojom::RequestDestination request_destination,
        int frame_tree_node_id)
    : m_request_destination(request_destination), m_frame_tree_node_id(frame_tree_node_id)
{}

void PluginResponseInterceptorURLLoaderThrottle::WillProcessResponse(const GURL &response_url,
                                                                     network::mojom::URLResponseHead *response_head,
                                                                     bool *defer)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (content::download_utils::MustDownload(response_url, response_head->headers.get(), response_head->mime_type))
        return;

    content::WebContents *web_contents = content::WebContents::FromFrameTreeNodeId(m_frame_tree_node_id);
    if (!web_contents)
        return;

    std::string extension_id;
    if (response_head->mime_type == "application/pdf")
        extension_id = extension_misc::kPdfExtensionId;
    if (extension_id.empty())
        return;

    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(web_contents->GetDelegate());
    if (!contentsDelegate)
        return;

    WebEngineSettings *settings = contentsDelegate->webEngineSettings();
    if (!settings->testAttribute(QWebEngineSettings::PdfViewerEnabled)
        || !settings->testAttribute(QWebEngineSettings::PluginsEnabled)) {
        // PluginServiceFilterQt will inform the URLLoader about the disabled state of plugins
        // and we can expect the download to be triggered automatically. It's unnecessary to
        // go further and start the guest view embedding process.
        return;
    }

    // Chrome's PDF Extension does not work properly in the face of a restrictive
    // Content-Security-Policy, and does not currently respect the policy anyway.
    // Ignore CSP served on a PDF response. https://crbug.com/271452
    if (extension_id == extension_misc::kPdfExtensionId && response_head->headers)
        ClearAllButFrameAncestors(response_head);

    MimeTypesHandler::ReportUsedHandler(extension_id);

    std::string view_id = base::GenerateGUID();
    // The string passed down to the original client with the response body.
    std::string payload = view_id;

    mojo::PendingRemote<network::mojom::URLLoader> dummy_new_loader;
    std::ignore = dummy_new_loader.InitWithNewPipeAndPassReceiver();
    mojo::Remote<network::mojom::URLLoaderClient> new_client;
    mojo::PendingReceiver<network::mojom::URLLoaderClient> new_client_receiver =
        new_client.BindNewPipeAndPassReceiver();


    uint32_t data_pipe_size = 64U;
    // Provide the MimeHandlerView code a chance to override the payload. This is
    // the case where the resource is handled by frame-based MimeHandlerView.
    *defer = extensions::MimeHandlerViewAttachHelper::OverrideBodyForInterceptedResponse(
        m_frame_tree_node_id, response_url, response_head->mime_type, view_id,
        &payload, &data_pipe_size,
        base::BindOnce(
            &PluginResponseInterceptorURLLoaderThrottle::ResumeLoad,
            weak_factory_.GetWeakPtr()));

    mojo::ScopedDataPipeProducerHandle producer_handle;
    mojo::ScopedDataPipeConsumerHandle consumer_handle;
    CHECK_EQ(MOJO_RESULT_OK, mojo::CreateDataPipe(data_pipe_size, producer_handle, consumer_handle));

    uint32_t len = static_cast<uint32_t>(payload.size());
    CHECK_EQ(MOJO_RESULT_OK,
                producer_handle->WriteData(
                    payload.c_str(), &len, MOJO_WRITE_DATA_FLAG_ALL_OR_NONE));

    network::URLLoaderCompletionStatus status(net::OK);
    status.decoded_body_length = len;
    new_client->OnComplete(status);

    mojo::PendingRemote<network::mojom::URLLoader> original_loader;
    mojo::PendingReceiver<network::mojom::URLLoaderClient> original_client;
    mojo::ScopedDataPipeConsumerHandle body = std::move(consumer_handle);
    delegate_->InterceptResponse(std::move(dummy_new_loader),
                                 std::move(new_client_receiver),
                                 &original_loader, &original_client,
                                 &body);

    // Make a deep copy of URLResponseHead before passing it cross-thread.
    auto deep_copied_response = response_head->Clone();
    if (response_head->headers) {
        deep_copied_response->headers =
            base::MakeRefCounted<net::HttpResponseHeaders>(
                response_head->headers->raw_headers());
    }

    auto transferrable_loader = blink::mojom::TransferrableURLLoader::New();
    transferrable_loader->url = GURL(
        extensions::Extension::GetBaseURLFromExtensionId(extension_id).spec() +
        base::GenerateGUID());
    transferrable_loader->url_loader = std::move(original_loader);
    transferrable_loader->url_loader_client = std::move(original_client);
    transferrable_loader->head = std::move(deep_copied_response);
    transferrable_loader->head->intercepted_by_plugin = true;
    transferrable_loader->body = std::move(body);

    bool embedded = m_request_destination !=
            network::mojom::RequestDestination::kDocument;
    content::GetUIThreadTaskRunner({})->PostTask(
      FROM_HERE,
      base::BindOnce(
            &extensions::StreamsPrivateAPI::SendExecuteMimeTypeHandlerEvent,
            extension_id, view_id, embedded, m_frame_tree_node_id,
            std::move(transferrable_loader), response_url));
}

void PluginResponseInterceptorURLLoaderThrottle::ResumeLoad()
{
    delegate_->Resume();
}

} // namespace QtWebEngineCore
