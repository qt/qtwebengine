// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PDF_WEB_CONTENTS_HELPER_CLIENT_QT_H_
#define PDF_WEB_CONTENTS_HELPER_CLIENT_QT_H_

#include "components/pdf/browser/pdf_web_contents_helper_client.h"

namespace extensions {

class PDFWebContentsHelperClientQt : public pdf::PDFWebContentsHelperClient
{
public:
    PDFWebContentsHelperClientQt();
    ~PDFWebContentsHelperClientQt() override;

private:
    // pdf::PDFWebContentsHelperClient:
    void UpdateContentRestrictions(content::WebContents *contents, int content_restrictions) override;
    void OnPDFHasUnsupportedFeature(content::WebContents *contents) override;
    void OnSaveURL(content::WebContents *contents) override;
};

} // namespace extensions

#endif // PDF_WEB_CONTENTS_HELPER_CLIENT_QT_H_
