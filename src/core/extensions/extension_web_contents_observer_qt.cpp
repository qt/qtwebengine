// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extension_web_contents_observer_qt.h"

#include "content/browser/renderer_host/render_frame_host_impl.h"
#include "content/public/browser/child_process_security_policy.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/manifest.h"
#include "third_party/blink/public/common/chrome_debug_urls.h"

#include "render_widget_host_view_qt.h"

namespace extensions {

ExtensionWebContentsObserverQt::ExtensionWebContentsObserverQt(content::WebContents *web_contents)
    : ExtensionWebContentsObserver(web_contents)
    , content::WebContentsUserData<ExtensionWebContentsObserverQt>(*web_contents)
{
}

ExtensionWebContentsObserverQt::~ExtensionWebContentsObserverQt()
{
}

// static
void ExtensionWebContentsObserverQt::CreateForWebContents(content::WebContents *web_contents)
{
    content::WebContentsUserData<ExtensionWebContentsObserverQt>::CreateForWebContents(web_contents);

    // Initialize this instance if necessary.
    FromWebContents(web_contents)->Initialize();
}

void ExtensionWebContentsObserverQt::RenderFrameCreated(content::RenderFrameHost *render_frame_host)
{
    ExtensionWebContentsObserver::RenderFrameCreated(render_frame_host);

    if (web_contents()->IsInnerWebContentsForGuest() && static_cast<content::RenderFrameHostImpl *>(render_frame_host)->is_local_root_subframe()) {
        content::WebContents *parent = web_contents()->GetOutermostWebContents();
        QtWebEngineCore::RenderWidgetHostViewQt *main_rwhv = static_cast<QtWebEngineCore::RenderWidgetHostViewQt *>(parent->GetRenderWidgetHostView());
        // Main frame of guest WebContents
        content::RenderWidgetHost *guest_render_widget_host = web_contents()->GetRenderViewHost()->GetWidget();
        main_rwhv->addGuest(guest_render_widget_host);
        // The frame which holds the actual PDF content inside the guest
        content::RenderWidgetHost *pdf_render_widget_host = render_frame_host->GetRenderWidgetHost();
        main_rwhv->addGuest(pdf_render_widget_host);
    }

    const Extension *extension = GetExtensionFromFrame(render_frame_host, false);
    if (!extension)
        return;

    int process_id = render_frame_host->GetProcess()->GetID();
    auto *policy = content::ChildProcessSecurityPolicy::GetInstance();

    if (extension->is_extension() && Manifest::IsComponentLocation(extension->location()))
        policy->GrantRequestOrigin(process_id, url::Origin::Create(GURL(blink::kChromeUIResourcesURL)));
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(ExtensionWebContentsObserverQt);

} // namespace extensions
