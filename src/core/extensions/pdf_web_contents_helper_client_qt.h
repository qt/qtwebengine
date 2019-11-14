// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_WEB_CONTENTS_HELPER_CLIENT_QT_H_
#define PDF_WEB_CONTENTS_HELPER_CLIENT_QT_H_

#include "base/macros.h"
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

    DISALLOW_COPY_AND_ASSIGN(PDFWebContentsHelperClientQt);
};

} // namespace extensions

#endif // PDF_WEB_CONTENTS_HELPER_CLIENT_QT_H_
