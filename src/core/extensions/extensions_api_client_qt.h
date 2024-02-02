// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Portions copyright 2015 The Chromium Embedded Framework Authors.
// Portions copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_API_CLIENT_QT_H_
#define EXTENSIONS_API_CLIENT_QT_H_

#include "extensions/browser/api/extensions_api_client.h"

namespace extensions {

class FileSystemDelegate;
class MessagingDelegate;

class ExtensionsAPIClientQt : public ExtensionsAPIClient
{
public:
    ExtensionsAPIClientQt();

    // ExtensionsAPIClient implementation.
    AppViewGuestDelegate *CreateAppViewGuestDelegate() const override;
    FileSystemDelegate *GetFileSystemDelegate() override;
    std::unique_ptr<guest_view::GuestViewManagerDelegate>
    CreateGuestViewManagerDelegate(content::BrowserContext *context) const override;
    std::unique_ptr<MimeHandlerViewGuestDelegate>
    CreateMimeHandlerViewGuestDelegate(MimeHandlerViewGuest *guest) const override;
    void AttachWebContentsHelpers(content::WebContents *web_contents) const override;
    MessagingDelegate *GetMessagingDelegate() override;

private:
    std::unique_ptr<FileSystemDelegate> m_fileSystemDelegate;
    std::unique_ptr<MessagingDelegate> m_messagingDelegate;
};

} // namespace extensions

#endif // EXTENSIONS_API_CLIENT_QT_H_
