// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Portions copyright 2015 The Chromium Embedded Framework Authors.
// Portions copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions_api_client_qt.h"
#include "file_system_delegate_qt.h"
#include "messaging_delegate_qt.h"

#include <memory>

#include "extension_web_contents_observer_qt.h"
#include "extensions/browser/guest_view/extensions_guest_view_manager_delegate.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest_delegate.h"
#include "mime_handler_view_guest_delegate_qt.h"
#include "pdf/buildflags.h"
#include "printing/buildflags/buildflags.h"

#if BUILDFLAG(ENABLE_PDF)
#include "components/pdf/browser/pdf_web_contents_helper.h"
#include "printing/pdf_web_contents_helper_client_qt.h"
#endif

#if BUILDFLAG(ENABLE_PRINTING) && BUILDFLAG(ENABLE_PRINT_PREVIEW)
#include "printing/print_view_manager_qt.h"
#endif

namespace extensions {

ExtensionsAPIClientQt::ExtensionsAPIClientQt()
{
}

AppViewGuestDelegate *ExtensionsAPIClientQt::CreateAppViewGuestDelegate() const
{
    // TODO(extensions): Implement to support Apps.
    NOTREACHED();
    return nullptr;
}

FileSystemDelegate *ExtensionsAPIClientQt::GetFileSystemDelegate()
{
    if (!m_fileSystemDelegate)
        m_fileSystemDelegate = std::make_unique<FileSystemDelegateQt>();
    return m_fileSystemDelegate.get();
}

std::unique_ptr<guest_view::GuestViewManagerDelegate> ExtensionsAPIClientQt::CreateGuestViewManagerDelegate(content::BrowserContext *context) const
{
    return std::make_unique<extensions::ExtensionsGuestViewManagerDelegate>(context);
}

std::unique_ptr<MimeHandlerViewGuestDelegate> ExtensionsAPIClientQt::CreateMimeHandlerViewGuestDelegate(MimeHandlerViewGuest *guest) const
{
    return std::make_unique<MimeHandlerViewGuestDelegateQt>(guest);
}

void ExtensionsAPIClientQt::AttachWebContentsHelpers(content::WebContents *web_contents) const
{
    // PrefsTabHelper::CreateForWebContents(web_contents);
#if BUILDFLAG(ENABLE_PRINTING) && BUILDFLAG(ENABLE_PRINT_PREVIEW)
    QtWebEngineCore::PrintViewManagerQt::CreateForWebContents(web_contents);
#endif
    ExtensionWebContentsObserverQt::CreateForWebContents(web_contents);

#if BUILDFLAG(ENABLE_PDF)
    pdf::PDFWebContentsHelper::CreateForWebContentsWithClient(web_contents, std::make_unique<PDFWebContentsHelperClientQt>());
#endif
}

MessagingDelegate *ExtensionsAPIClientQt::GetMessagingDelegate()
{
    if (!m_messagingDelegate)
        m_messagingDelegate = std::make_unique<MessagingDelegateQt>();
    return m_messagingDelegate.get();
}

} // namespace extensions
