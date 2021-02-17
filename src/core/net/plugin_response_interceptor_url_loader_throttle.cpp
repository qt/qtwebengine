/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "plugin_response_interceptor_url_loader_throttle.h"

#include "base/bind.h"
#include "base/guid.h"
#include "base/task/post_task.h"
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

#include <string>

namespace QtWebEngineCore {

PluginResponseInterceptorURLLoaderThrottle::PluginResponseInterceptorURLLoaderThrottle(
        content::BrowserContext *browser_context, int resource_type, int frame_tree_node_id)
    : m_browser_context(browser_context), m_resource_type(resource_type), m_frame_tree_node_id(frame_tree_node_id)
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
    if (!settings->testAttribute(WebEngineSettings::PdfViewerEnabled)
        || !settings->testAttribute(WebEngineSettings::PluginsEnabled)) {
        // PluginServiceFilterQt will inform the URLLoader about the disabled state of plugins
        // and we can expect the download to be triggered automatically. It's unnecessary to
        // go further and start the guest view embedding process.
        return;
    }

    // Chrome's PDF Extension does not work properly in the face of a restrictive
    // Content-Security-Policy, and does not currently respect the policy anyway.
    // Ignore CSP served on a PDF response. https://crbug.com/271452
    if (extension_id == extension_misc::kPdfExtensionId && response_head->headers)
        response_head->headers->RemoveHeader("Content-Security-Policy");

    MimeTypesHandler::ReportUsedHandler(extension_id);

    std::string view_id = base::GenerateGUID();
    // The string passed down to the original client with the response body.
    std::string payload = view_id;

    mojo::PendingRemote<network::mojom::URLLoader> dummy_new_loader;
    ignore_result(dummy_new_loader.InitWithNewPipeAndPassReceiver());
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

    mojo::DataPipe data_pipe(data_pipe_size);
    uint32_t len = static_cast<uint32_t>(payload.size());
    CHECK_EQ(MOJO_RESULT_OK,
                data_pipe.producer_handle->WriteData(
                    payload.c_str(), &len, MOJO_WRITE_DATA_FLAG_ALL_OR_NONE));


    new_client->OnStartLoadingResponseBody(std::move(data_pipe.consumer_handle));

    network::URLLoaderCompletionStatus status(net::OK);
    status.decoded_body_length = len;
    new_client->OnComplete(status);

    mojo::PendingRemote<network::mojom::URLLoader> original_loader;
    mojo::PendingReceiver<network::mojom::URLLoaderClient> original_client;
    delegate_->InterceptResponse(std::move(dummy_new_loader),
                                std::move(new_client_receiver), &original_loader,
                                &original_client);

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

    bool embedded = m_resource_type !=
                        static_cast<int>(blink::mojom::ResourceType::kMainFrame);
    content::GetUIThreadTaskRunner({})->PostTask(
      FROM_HERE,
      base::BindOnce(
            &extensions::StreamsPrivateAPI::SendExecuteMimeTypeHandlerEvent,
            extension_id, view_id, embedded, m_frame_tree_node_id,
            -1 /* render_process_id */, -1 /* render_frame_id */,
            std::move(transferrable_loader), response_url));
}

void PluginResponseInterceptorURLLoaderThrottle::ResumeLoad() {
    delegate_->Resume();
}

} // namespace QtWebEngineCore
