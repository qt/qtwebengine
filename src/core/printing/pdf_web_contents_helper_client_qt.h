// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PDF_WEB_CONTENTS_HELPER_CLIENT_QT_H
#define PDF_WEB_CONTENTS_HELPER_CLIENT_QT_H

#include "components/pdf/browser/pdf_web_contents_helper_client.h"

// based on chrome/browser/ui/pdf/chrome_pdf_web_contents_helper_client.h:
class PDFWebContentsHelperClientQt : public pdf::PDFWebContentsHelperClient
{
public:
    PDFWebContentsHelperClientQt();
    PDFWebContentsHelperClientQt(const PDFWebContentsHelperClientQt&) = delete;
    PDFWebContentsHelperClientQt& operator=(const PDFWebContentsHelperClientQt&) = delete;
    ~PDFWebContentsHelperClientQt() override;

private:
    // pdf::PDFWebContentsHelperClient:
    content::RenderFrameHost* FindPdfFrame(content::WebContents *contents) override;
    void OnPDFHasUnsupportedFeature(content::WebContents *contents) override {}
    void OnSaveURL(content::WebContents *contents) override {}
    void SetPluginCanSave(content::RenderFrameHost *render_frame_host, bool can_save) override;
    void UpdateContentRestrictions(content::RenderFrameHost *, int) override;
};

#endif // PDF_WEB_CONTENTS_HELPER_CLIENT_QT_H
