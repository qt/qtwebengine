// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef PDF_DOCUMENT_HELPER_CLIENT_QT_H
#define PDF_DOCUMENT_HELPER_CLIENT_QT_H

#include "components/pdf/browser/pdf_document_helper_client.h"

// based on chrome/browser/ui/pdf/chrome_pdf_document_helper_client.h:
class PDFDocumentHelperClientQt : public pdf::PDFDocumentHelperClient // FIXME: rename
{
public:
    PDFDocumentHelperClientQt();
    PDFDocumentHelperClientQt(const PDFDocumentHelperClientQt&) = delete;
    PDFDocumentHelperClientQt& operator=(const PDFDocumentHelperClientQt&) = delete;
    ~PDFDocumentHelperClientQt() override;

private:
    // pdf::PDFDocumentHelperClient:
    void OnSaveURL(content::WebContents *contents) override {}
    void SetPluginCanSave(content::RenderFrameHost *render_frame_host, bool can_save) override;
    void UpdateContentRestrictions(content::RenderFrameHost *, int) override;
    void OnSearchifyStarted(content::WebContents *) override;
};

#endif // PDF_DOCUMENT_HELPER_CLIENT_QT_H
