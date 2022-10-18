// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "extensions/buildflags/buildflags.h"
#include "extensions/common/constants.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "url/origin.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "chrome/common/webui_url_constants.h"
#include "extensions/common/constants.h"
#include "third_party/blink/public/web/web_document.h"
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)

#include "print_web_view_helper_delegate_qt.h"
#include "web_engine_library_info.h"

namespace QtWebEngineCore {

PrintWebViewHelperDelegateQt::~PrintWebViewHelperDelegateQt() {}

bool IsPdfExtensionOrigin(const url::Origin& origin)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    return origin.scheme() == extensions::kExtensionScheme
        && origin.host() == extension_misc::kPdfExtensionId;
#else
    Q_UNUSED(origin);
    return false;
#endif
}

blink::WebElement PrintWebViewHelperDelegateQt::GetPdfElement(blink::WebLocalFrame *frame)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    const url::Origin origin = frame->GetDocument().GetSecurityOrigin();
    bool inside_print_preview = origin == url::Origin::Create(GURL(chrome::kChromeUIPrintURL));
    bool inside_pdf_extension = IsPdfExtensionOrigin(origin);
    if (inside_print_preview || inside_pdf_extension) {
        // <object> with id="plugin" is created in
        // chrome/browser/resources/pdf/pdf_viewer_base.js.
        auto viewer_element = frame->GetDocument().GetElementById("viewer");
        if (!viewer_element.IsNull() && !viewer_element.ShadowRoot().IsNull()) {
            auto plugin_element = viewer_element.ShadowRoot().QuerySelector("#plugin");
            if (!plugin_element.IsNull())
                return plugin_element;
        }
        NOTREACHED();
    }
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)
    return blink::WebElement();
}

bool PrintWebViewHelperDelegateQt::IsPrintPreviewEnabled()
{
    return true;
}

bool PrintWebViewHelperDelegateQt::OverridePrint(blink::WebLocalFrame *frame)
{
    return false;
}

} // namespace QtWebEngineCore

namespace printing {
// std::string PrintingContextDelegate::GetAppLocale()
std::string getApplicationLocale()
{
    return WebEngineLibraryInfo::getApplicationLocale();
}
}
