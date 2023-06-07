// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/ui/pdf/chrome_pdf_web_contents_helper_client.cc:

#include "pdf_web_contents_helper_client_qt.h"

#include "content/public/browser/render_process_host.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest.h"
#include "extensions/common/constants.h"

namespace {
bool IsPdfExtensionOrigin(const url::Origin &origin)
{
    return origin.scheme() == extensions::kExtensionScheme &&
           origin.host() == extension_misc::kPdfExtensionId;
}

// from chrome/browser/pdf/pdf_frame_util.cc:
content::RenderFrameHost *FindPdfChildFrame(content::RenderFrameHost *rfh)
{
    if (!IsPdfExtensionOrigin(rfh->GetLastCommittedOrigin()))
        return nullptr;

    content::RenderFrameHost *pdf_rfh = nullptr;
    rfh->ForEachRenderFrameHost(
                    [&pdf_rfh](content::RenderFrameHost *rfh) {
                        if (!rfh->GetProcess()->IsPdf())
                            return;

                        DCHECK(IsPdfExtensionOrigin(rfh->GetParent()->GetLastCommittedOrigin()));
                        DCHECK(!pdf_rfh);
                        pdf_rfh = rfh;
                    });

    return pdf_rfh;
}
}  // namespace

PDFWebContentsHelperClientQt::PDFWebContentsHelperClientQt() = default;
PDFWebContentsHelperClientQt::~PDFWebContentsHelperClientQt() = default;

content::RenderFrameHost *PDFWebContentsHelperClientQt::FindPdfFrame(content::WebContents *contents)
{
    content::RenderFrameHost *main_frame = contents->GetPrimaryMainFrame();
    content::RenderFrameHost *pdf_frame = FindPdfChildFrame(main_frame);
    return pdf_frame ? pdf_frame : main_frame;
}

void PDFWebContentsHelperClientQt::SetPluginCanSave(content::RenderFrameHost *render_frame_host, bool can_save)
{
    auto *guest_view = extensions::MimeHandlerViewGuest::FromRenderFrameHost(render_frame_host);
    if (guest_view)
        guest_view->SetPluginCanSave(can_save);
}

void PDFWebContentsHelperClientQt::UpdateContentRestrictions(content::RenderFrameHost *, int)
{
}
