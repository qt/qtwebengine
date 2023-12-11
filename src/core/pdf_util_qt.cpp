// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "pdf_util_qt.h"

#include <QtGlobal>

#include "base/check.h"
#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/buildflags/buildflags.h"
#include "url/gurl.h"
#include "url/origin.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest.h"
#include "extensions/common/constants.h"
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)

namespace QtWebEngineCore {

bool IsPdfExtensionOrigin(const url::Origin &origin)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    return origin.scheme() == extensions::kExtensionScheme
        && origin.host() == extension_misc::kPdfExtensionId;
#else
    Q_UNUSED(origin);
    return false;
#endif
}

bool IsPdfInternalPluginAllowedOrigin(const url::Origin &origin)
{
    if (IsPdfExtensionOrigin(origin))
        return true;

    // Allow embedding the internal PDF plugin in chrome://print.
    if (origin == url::Origin::Create(GURL(chrome::kChromeUIPrintURL)))
        return true;

    // Only allow the PDF plugin in the known, trustworthy origins that are
    // allowlisted above.  See also https://crbug.com/520422 and
    // https://crbug.com/1027173.
    return false;
}

content::RenderFrameHost *GetFullPagePlugin(content::WebContents *contents)
{
    content::RenderFrameHost *full_page_plugin = nullptr;
#if BUILDFLAG(ENABLE_EXTENSIONS)
    contents->ForEachRenderFrameHostWithAction([&full_page_plugin](content::RenderFrameHost *rfh) {
        auto* guest_view = extensions::MimeHandlerViewGuest::FromRenderFrameHost(rfh);
        if (guest_view && guest_view->is_full_page_plugin()) {
            DCHECK_EQ(guest_view->GetGuestMainFrame(), rfh);
            full_page_plugin = rfh;
            return content::RenderFrameHost::FrameIterationAction::kStop;
        }
        return content::RenderFrameHost::FrameIterationAction::kContinue;
    });
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)
    return full_page_plugin;
}

content::RenderFrameHost *FindPdfChildFrame(content::RenderFrameHost *rfh)
{
    if (!rfh)
        return nullptr;

    if (!IsPdfExtensionOrigin(rfh->GetLastCommittedOrigin()))
        return nullptr;

    content::RenderFrameHost *pdf_rfh = nullptr;
    rfh->ForEachRenderFrameHost([&pdf_rfh](content::RenderFrameHost *rfh) {
        if (!rfh->GetProcess()->IsPdf())
            return;

        DCHECK(IsPdfExtensionOrigin(rfh->GetParent()->GetLastCommittedOrigin()));
        DCHECK(!pdf_rfh);
        pdf_rfh = rfh;
    });

    return pdf_rfh;
}

} // namespace QtWebEngineCore
