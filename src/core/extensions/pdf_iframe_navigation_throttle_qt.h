/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
