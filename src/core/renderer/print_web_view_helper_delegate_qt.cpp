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
#include "extensions/renderer/guest_view/mime_handler_view/post_message_support.h"
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)

#include "pdf_util_qt.h"
#include "print_web_view_helper_delegate_qt.h"
#include "web_engine_library_info.h"

namespace QtWebEngineCore {

PrintWebViewHelperDelegateQt::~PrintWebViewHelperDelegateQt() {}

blink::WebElement PrintWebViewHelperDelegateQt::GetPdfElement(blink::WebLocalFrame *frame)
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    if (frame->Parent() && IsPdfInternalPluginAllowedOrigin(frame->Parent()->GetSecurityOrigin())) {
        auto plugin_element = frame->GetDocument().QuerySelector("embed");
        DCHECK(!plugin_element.IsNull());
        return plugin_element;
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
