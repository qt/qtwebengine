// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Portions copyright 2015 The Chromium Embedded Framework Authors.
// Portions copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIME_HANDLER_VIEW_GUEST_DELEGATE_QT_H_
#define MIME_HANDLER_VIEW_GUEST_DELEGATE_QT_H_

#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest_delegate.h"
#include "api/qtwebenginecoreglobal_p.h"

QT_BEGIN_NAMESPACE
class QWebEngineContextMenuRequest;
QT_END_NAMESPACE

namespace content {
struct ContextMenuParams;
}

namespace extensions {
class MimeHandlerViewGuest;

class MimeHandlerViewGuestDelegateQt : public MimeHandlerViewGuestDelegate
{
public:
    explicit MimeHandlerViewGuestDelegateQt(MimeHandlerViewGuest *guest);
    ~MimeHandlerViewGuestDelegateQt() override;

    bool HandleContextMenu(content::RenderFrameHost &render_frame_host,
                           const content::ContextMenuParams &params) override;

private:
    QWebEngineContextMenuRequest *m_contextMenuRequest;
};

} // namespace extensions

#endif // MIME_HANDLER_VIEW_GUEST_DELEGATE_QT_H_
