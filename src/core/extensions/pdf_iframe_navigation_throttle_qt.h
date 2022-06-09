// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on //chrome/browser/plugins/pdf_iframe_navigation_throttle.h
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_IFRAME_NAVIGATION_THROTTLE_QT
#define PDF_IFRAME_NAVIGATION_THROTTLE_QT

#include "content/public/browser/navigation_throttle.h"

#include "base/memory/weak_ptr.h"

namespace content {
class NavigationHandle;
struct WebPluginInfo;
}

namespace extensions {

// This class prevents automatical download of PDFs when they are embedded
// in subframes and plugins are disabled in API.
class PDFIFrameNavigationThrottleQt : public content::NavigationThrottle
{
public:
    static std::unique_ptr<content::NavigationThrottle> MaybeCreateThrottleFor(content::NavigationHandle *handle);

    explicit PDFIFrameNavigationThrottleQt(content::NavigationHandle *handle);
    ~PDFIFrameNavigationThrottleQt() override;

    // content::NavigationThrottle
    content::NavigationThrottle::ThrottleCheckResult WillProcessResponse() override;
    const char *GetNameForLogging() override;

private:
    void OnPluginsLoaded(const std::vector<content::WebPluginInfo> &plugins);
    void LoadPlaceholderHTML();

    base::WeakPtrFactory<PDFIFrameNavigationThrottleQt> weak_factory_{this};
};

} // namespace extensions

#endif // PDF_IFRAME_NAVIGATION_THROTTLE_QT
