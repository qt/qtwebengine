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
#include "base/task/post_task.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/download_request_utils.h"
#include "content/public/browser/download_utils.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"

#include "extensions/extension_system_qt.h"
#include "profile_adapter.h"
#include "profile_io_data_qt.h"
#include "profile_qt.h"
#include "web_contents_delegate_qt.h"

#include <string>

namespace QtWebEngineCore {

void onPdfStreamIntercepted(const GURL &original_url, std::string extension_id, int frame_tree_node_id)
{
    content::WebContents *web_contents = content::WebContents::FromFrameTreeNodeId(frame_tree_node_id);
    if (!web_contents)
        return;

    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(web_contents->GetDelegate());
    if (!contentsDelegate)
        return;

    WebEngineSettings *settings = contentsDelegate->webEngineSettings();
    if (!settings->testAttribute(WebEngineSettings::PdfViewerEnabled)
        || !settings->testAttribute(WebEngineSettings::PluginsEnabled)) {
        // If the applications has been set up to always download PDF files to open them in an
        // external viewer, trigger the download.
        std::unique_ptr<download::DownloadUrlParameters> params(
                content::DownloadRequestUtils::CreateDownloadForWebContentsMainFrame(web_contents, original_url,
                                                                                     MISSING_TRAFFIC_ANNOTATION));
        content::BrowserContext::GetDownloadManager(web_contents->GetBrowserContext())->DownloadUrl(std::move(params));
        return;
    }

    // The URL passes the original pdf resource url, that will be requested
    // by the pdf viewer extension page.
    content::NavigationController::LoadURLParams params(
            GURL(base::StringPrintf("%s://%s/index.html?%s", extensions::kExtensionScheme,
                                    extension_id.c_str(), original_url.spec().c_str())));

    params.frame_tree_node_id = frame_tree_node_id;
    web_contents->GetController().LoadURLWithParams(params);
}


PluginResponseInterceptorURLLoaderThrottle::PluginResponseInterceptorURLLoaderThrottle(
        content::ResourceContext *resource_context, int resource_type, int frame_tree_node_id)
    : m_resource_context(resource_context), m_resource_type(resource_type), m_frame_tree_node_id(frame_tree_node_id)
{}

PluginResponseInterceptorURLLoaderThrottle::PluginResponseInterceptorURLLoaderThrottle(
        content::BrowserContext *browser_context, int resource_type, int frame_tree_node_id)
    : m_browser_context(browser_context), m_resource_type(resource_type), m_frame_tree_node_id(frame_tree_node_id)
{}

void PluginResponseInterceptorURLLoaderThrottle::WillProcessResponse(const GURL &response_url,
                                                                     network::mojom::URLResponseHead *response_head,
                                                                     bool *defer)
{
    Q_UNUSED(defer);
    if (content::download_utils::MustDownload(response_url, response_head->headers.get(), response_head->mime_type))
        return;

    if (m_resource_context) {
        DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    } else {
        DCHECK(m_browser_context);
        DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    }

    std::string extension_id;
    // FIXME: We should use extensions::InfoMap in the future:
    if (response_head->mime_type == "application/pdf")
        extension_id = extension_misc::kPdfExtensionId;
    if (extension_id.empty())
        return;

    *defer = true;

    base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                   base::BindOnce(&onPdfStreamIntercepted,
                                  response_url,
                                  extension_id,
                                  m_frame_tree_node_id));
}

} // namespace QtWebEngineCore
