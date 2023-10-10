// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Portions copyright 2015 The Chromium Embedded Framework Authors.
// Portions copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mime_handler_view_guest_delegate_qt.h"

#include "content/public/browser/context_menu_params.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest.h"

#include "profile_adapter.h"
#include "qwebenginecontextmenurequest.h"
#include "qwebenginecontextmenurequest_p.h"
#include "render_widget_host_view_qt.h"
#include "touch_selection_controller_client_qt.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

namespace extensions {

MimeHandlerViewGuestDelegateQt::MimeHandlerViewGuestDelegateQt(MimeHandlerViewGuest *)
    : MimeHandlerViewGuestDelegate()
    , m_contextMenuRequest(new QWebEngineContextMenuRequest(new QWebEngineContextMenuRequestPrivate))
{
}

MimeHandlerViewGuestDelegateQt::~MimeHandlerViewGuestDelegateQt()
{
    delete m_contextMenuRequest;
}

bool MimeHandlerViewGuestDelegateQt::HandleContextMenu(content::RenderFrameHost &render_frame_host, const content::ContextMenuParams &params)
{
    content::WebContents *web_contents = content::WebContents::FromRenderFrameHost(&render_frame_host);
    content::WebContents *parent_web_contents = guest_view::GuestViewBase::GetTopLevelWebContents(web_contents);
    if (auto rwhv = static_cast<QtWebEngineCore::RenderWidgetHostViewQt *>(parent_web_contents->GetRenderWidgetHostView())) {
        if (rwhv->getTouchSelectionControllerClient()->handleContextMenu(params))
            return true;

        QtWebEngineCore::WebContentsAdapterClient *adapterClient = rwhv->adapterClient();
        QtWebEngineCore::WebContentsViewQt::update(m_contextMenuRequest, params, adapterClient->profileAdapter()->isSpellCheckEnabled());
        adapterClient->contextMenuRequested(m_contextMenuRequest);
        return true;
    }

    return false;
}

} // namespace extensions
