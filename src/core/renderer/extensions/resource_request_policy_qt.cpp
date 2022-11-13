// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/renderer/extensions/resource_request_policy.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "resource_request_policy_qt.h"

#include "base/strings/stringprintf.h"
#include "chrome/common/url_constants.h"
#include "extensions/common/constants.h"
#include "extensions/common/manifest_handlers/web_accessible_resources_info.h"
#include "extensions/common/manifest_handlers/webview_info.h"
#include "extensions/renderer/dispatcher.h"
#include "third_party/blink/public/mojom/devtools/console_message.mojom-shared.h"
#include "third_party/blink/public/web/web_console_message.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_local_frame.h"

namespace extensions {
ResourceRequestPolicyQt::ResourceRequestPolicyQt(Dispatcher *dispatcher)
        : m_dispatcher(dispatcher)
{
}

void ResourceRequestPolicyQt::OnExtensionLoaded(const Extension &extension)
{
    if (WebAccessibleResourcesInfo::HasWebAccessibleResources(&extension)
        || WebviewInfo::HasWebviewAccessibleResources(extension, m_dispatcher->webview_partition_id())
//          // Hosted app icons are accessible.
//          // TODO(devlin): Should we incorporate this into
//          // WebAccessibleResourcesInfo?
//        || (extension.is_hosted_app() && !IconsInfo::GetIcons(&extension).empty())
       ) {
        m_web_accessible_ids.insert(extension.id());
    }
}

void ResourceRequestPolicyQt::OnExtensionUnloaded(const ExtensionId &extension_id)
{
    m_web_accessible_ids.erase(extension_id);
}

// Returns true if the chrome-extension:// |resource_url| can be requested
// from |frame_url|. In some cases this decision is made based upon how
// this request was generated. Web triggered transitions are more restrictive
// than those triggered through UI.
bool ResourceRequestPolicyQt::CanRequestResource(const GURL &resource_url,
                                                 blink::WebLocalFrame *frame,
                                                 ui::PageTransition transition_type,
                                                 const absl::optional<url::Origin>& initiator_origin)
{
    CHECK(resource_url.SchemeIs(kExtensionScheme));

    GURL frame_url = frame->GetDocument().Url();

    // The page_origin may be GURL("null") for unique origins like data URLs,
    // but this is ok for the checks below.  We only care if it matches the
    // current extension or has a devtools scheme.
    GURL page_origin = url::Origin(frame->Top()->GetSecurityOrigin()).GetURL();

    GURL extension_origin = resource_url.DeprecatedGetOriginAsURL();

    // We always allow loads in the following cases, regardless of web accessible
    // resources:

    // Empty urls (needed for some edge cases when we have empty urls).
    if (frame_url.is_empty())
        return true;

    // Extensions requesting their own resources (frame_url check is for images,
    // page_url check is for iframes).
    // TODO(devlin): We should be checking the ancestor chain, not just the
    // top-level frame. Additionally, we should be checking the security origin
    // of the frame, to account for about:blank subframes being scripted by an
    // extension parent (though we'll still need the frame origin check for
    // sandboxed frames).
    if (frame_url.DeprecatedGetOriginAsURL() == extension_origin || page_origin == extension_origin)
        return true;

    if (!ui::PageTransitionIsWebTriggerable(transition_type))
        return true;

    // Unreachable web page error page (to allow showing the icon of the
    // unreachable app on this page).
    if (frame_url == content::kUnreachableWebDataURL)
        return true;

    bool is_dev_tools = page_origin.SchemeIs(content::kChromeDevToolsScheme);
    // Note: we check |web_accessible_ids_| (rather than first looking up the
    // extension in the registry and checking that) to be more resistant against
    // timing attacks. This way, determining access for an extension that isn't
    // installed takes the same amount of time as determining access for an
    // extension with no web accessible resources. We aren't worried about any
    // extensions with web accessible resources, since those are inherently
    // identifiable.
    if (!is_dev_tools && !m_web_accessible_ids.count(extension_origin.host()))
        return false;

    const Extension* extension = RendererExtensionRegistry::Get()->GetExtensionOrAppByURL(resource_url);
    if (is_dev_tools) {
        // Allow the load in the case of a non-existent extension. We'll just get a
        // 404 from the browser process.
        // TODO(devlin): Can this happen? Does devtools potentially make requests
        // to non-existent extensions?
        if (!extension)
            return true;
//        // Devtools (chrome-extension:// URLs are loaded into frames of devtools to
//        // support the devtools extension APIs).
//        if (!chrome_manifest_urls::GetDevToolsPage(extension).is_empty())
//            return true;
    }

    DCHECK(extension);

    // Disallow loading of packaged resources for hosted apps. We don't allow
    // hybrid hosted/packaged apps. The one exception is access to icons, since
    // some extensions want to be able to do things like create their own
    // launchers.
    /*base::StringPiece resource_root_relative_path =
            resource_url.path_piece().empty() ? base::StringPiece()
                                              : resource_url.path_piece().substr(1);*/
    if (extension->is_hosted_app() /*&& !IconsInfo::GetIcons(extension).ContainsPath(resource_root_relative_path)*/) {
        LOG(ERROR) << "Denying load of " << resource_url.spec() << " from "
                   << "hosted app.";
        return false;
    }

    // Disallow loading of extension resources which are not explicitly listed
    // as web or WebView accessible if the manifest version is 2 or greater.
    if (!WebAccessibleResourcesInfo::IsResourceWebAccessible(extension, resource_url.path(), initiator_origin) &&
        !WebviewInfo::IsResourceWebviewAccessible(extension, m_dispatcher->webview_partition_id(), resource_url.path()))
    {
        std::string message = base::StringPrintf(
                    "Denying load of %s. Resources must be listed in the "
                    "web_accessible_resources manifest key in order to be loaded by "
                    "pages outside the extension.",
                    resource_url.spec().c_str());
        frame->AddMessageToConsole(blink::WebConsoleMessage(blink::mojom::ConsoleMessageLevel::kError, blink::WebString::FromUTF8(message)));
        return false;
    }

    return true;
}

} // namespace extensions
