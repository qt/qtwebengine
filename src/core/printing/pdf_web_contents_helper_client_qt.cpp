// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/ui/pdf/chrome_pdf_web_contents_helper_client.cc:

#include "pdf_web_contents_helper_client_qt.h"

#include "content/public/browser/render_process_host.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest.h"
#include "extensions/common/constants.h"

#include "pdf_util_qt.h"

PDFWebContentsHelperClientQt::PDFWebContentsHelperClientQt() = default;
PDFWebContentsHelperClientQt::~PDFWebContentsHelperClientQt() = default;

content::RenderFrameHost *PDFWebContentsHelperClientQt::FindPdfFrame(content::WebContents *contents)
{
    content::RenderFrameHost *main_frame = contents->GetPrimaryMainFrame();
    content::RenderFrameHost *pdf_frame = QtWebEngineCore::FindPdfChildFrame(main_frame);
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
