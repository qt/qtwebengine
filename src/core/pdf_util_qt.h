// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#ifndef PDF_UTIL_QT_H
#define PDF_UTIL_QT_H

namespace content {
class RenderFrameHost;
class WebContents;
}  // namespace content

namespace url {
class Origin;
}  // namespace url

namespace QtWebEngineCore {

// from chrome/common/pdf_util.cc:
constexpr char kPDFMimeType[] = "application/pdf";

bool IsPdfExtensionOrigin(const url::Origin &origin);
bool IsPdfInternalPluginAllowedOrigin(const url::Origin &origin);

// from chrome/browser/pdf/pdf_frame_util.cc:
content::RenderFrameHost *GetFullPagePlugin(content::WebContents *contents);
content::RenderFrameHost *FindPdfChildFrame(content::RenderFrameHost *rfh);

} // namespace QtWebEngineCore

#endif // PDF_UTIL_QT_H
