// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#ifndef PRINT_WEB_VIEW_HELPER_DELEGATE_QT_H
#define PRINT_WEB_VIEW_HELPER_DELEGATE_QT_H

#include "components/printing/renderer/print_render_frame_helper.h"

namespace blink {
class WebLocalFrame;
}

namespace QtWebEngineCore {

class PrintWebViewHelperDelegateQt : public printing::PrintRenderFrameHelper::Delegate
{
public:
    ~PrintWebViewHelperDelegateQt() override;

    blink::WebElement GetPdfElement(blink::WebLocalFrame *frame) override;

    bool IsPrintPreviewEnabled() override;

    bool OverridePrint(blink::WebLocalFrame *frame) override;
};

} // namespace QtWebEngineCore

#endif // PRINT_WEB_VIEW_HELPER_DELEGATE_QT_H
