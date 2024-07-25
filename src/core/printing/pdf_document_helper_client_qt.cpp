// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/ui/pdf/chrome_pdf_document_helper_client.cc:

#include "pdf_document_helper_client_qt.h"

#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest.h"

PDFDocumentHelperClientQt::PDFDocumentHelperClientQt() = default;
PDFDocumentHelperClientQt::~PDFDocumentHelperClientQt() = default;

void PDFDocumentHelperClientQt::SetPluginCanSave(content::RenderFrameHost *render_frame_host, bool can_save)
{
    auto *guest_view = extensions::MimeHandlerViewGuest::FromRenderFrameHost(render_frame_host);
    if (guest_view)
        guest_view->SetPluginCanSave(can_save);
}

void PDFDocumentHelperClientQt::UpdateContentRestrictions(content::RenderFrameHost *, int)
{
}
