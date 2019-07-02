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

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "resource_dispatcher_host_delegate_qt.h"

#include "base/bind.h"
#include "base/guid.h"
#include "base/strings/stringprintf.h"
#include "base/task/post_task.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/download_request_utils.h"
#include "content/public/browser/navigation_controller.h"

#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/stream_info.h"
#include "content/public/browser/web_contents.h"

#include "extensions/extension_system_qt.h"
#include "extensions/browser/info_map.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handlers/mime_types_handler.h"

#include "net/base/escape.h"
#include "net/url_request/url_request.h"

#include "profile_io_data_qt.h"
#include "type_conversion.h"
#include "web_contents_delegate_qt.h"
#include "web_engine_settings.h"

namespace QtWebEngineCore {

void OnPdfStreamIntercepted(
    const GURL& original_url,
    std::string extension_id,
    int frame_tree_node_id,
    const content::ResourceRequestInfo::WebContentsGetter&
        web_contents_getter) {
    content::WebContents* web_contents = web_contents_getter.Run();
    if (!web_contents)
        return;

    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt*>(web_contents->GetDelegate());
    if (!contentsDelegate)
        return;

    WebEngineSettings *settings = contentsDelegate->webEngineSettings();
    if (!settings->testAttribute(WebEngineSettings::PdfViewerEnabled)
        || !settings->testAttribute(WebEngineSettings::PluginsEnabled)) {
        // If the applications has been set up to always download PDF files to open them in an
        // external viewer, trigger the download.
        std::unique_ptr<download::DownloadUrlParameters> params(
                    content::DownloadRequestUtils::CreateDownloadForWebContentsMainFrame(
                        web_contents, original_url, NO_TRAFFIC_ANNOTATION_YET));
        content::BrowserContext::GetDownloadManager(web_contents->GetBrowserContext())
                ->DownloadUrl(std::move(params));
        return;
    }

    // The URL passes the original pdf resource url, that will be requested
    // by the pdf viewer extension page.
    content::NavigationController::LoadURLParams params(
              GURL(base::StringPrintf("%s://%s/index.html?%s", extensions::kExtensionScheme,
                   extension_id.c_str(),
                   original_url.spec().c_str())));

    params.frame_tree_node_id = frame_tree_node_id;
    web_contents->GetController().LoadURLWithParams(params);
}

bool ResourceDispatcherHostDelegateQt::ShouldInterceptResourceAsStream(net::URLRequest *request,
                                                                       const std::string &mime_type,
                                                                       GURL *origin,
                                                                       std::string *payload)
{
    const content::ResourceRequestInfo* info =
        content::ResourceRequestInfo::ForRequest(request);

    int render_process_host_id = -1;
    int render_frame_id = -1;
    if (!content::ResourceRequestInfo::GetRenderFrameForRequest(request, &render_process_host_id, &render_frame_id))
        return false;

    std::vector<std::string> whitelist = MimeTypesHandler::GetMIMETypeWhitelist();

    extensions::ExtensionSystemQt *extensionSystem = ProfileIODataQt::FromResourceContext(info->GetContext())->GetExtensionSystem();
    if (!extensionSystem)
        return false;

    const scoped_refptr<const extensions::InfoMap> extension_info_map(extensionSystem->info_map());

    for (const std::string &extension_id : whitelist) {
        const extensions::Extension *extension = extension_info_map->extensions().GetByID(extension_id);
        if (!extension)
            continue;

        MimeTypesHandler* handler = MimeTypesHandler::GetHandler(extension);
        if (!handler)
            continue;
        if (handler->CanHandleMIMEType(mime_type)) {
            StreamTargetInfo target_info;
            *origin = extensions::Extension::GetBaseURLFromExtensionId(extension_id);
            target_info.extension_id = extension_id;
            target_info.view_id = base::GenerateGUID();
            *payload = target_info.view_id;
            stream_target_info_[request] = target_info;
            return true;
        }
    }
    return false;
}

// Informs the delegate that a Stream was created. The Stream can be read from
// the blob URL of the Stream, but can only be read once.
void ResourceDispatcherHostDelegateQt::OnStreamCreated(net::URLRequest *request,
                                                       std::unique_ptr<content::StreamInfo> stream)
{
    const content::ResourceRequestInfo *info = content::ResourceRequestInfo::ForRequest(request);
    std::map<net::URLRequest *, StreamTargetInfo>::iterator ix = stream_target_info_.find(request);
    CHECK(ix != stream_target_info_.end());
    int render_frame_id = -1;
    int render_process_id = -1;
    if (!content::ResourceRequestInfo::GetRenderFrameForRequest(request, &render_process_id, &render_frame_id)) {
        stream_target_info_.erase(request);
        request->Cancel();
        return;
    }

    base::PostTaskWithTraits(
          FROM_HERE, {content::BrowserThread::UI},
                base::BindOnce(&OnPdfStreamIntercepted,
                               request->url(), ix->second.extension_id,
                               info->GetFrameTreeNodeId(), info->GetWebContentsGetterForRequest()
                              )
                );
    stream_target_info_.erase(request);
}

} // namespace QtWebEngineCore
