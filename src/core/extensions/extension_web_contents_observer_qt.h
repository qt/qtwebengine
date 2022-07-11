// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSION_WEB_CONTENTS_OBSERVER_QT_H_
#define EXTENSION_WEB_CONTENTS_OBSERVER_QT_H_

#include "content/public/browser/web_contents_user_data.h"
#include "extensions/browser/extension_web_contents_observer.h"

namespace extensions {

class ExtensionWebContentsObserverQt
    : public ExtensionWebContentsObserver
    , public content::WebContentsUserData<ExtensionWebContentsObserverQt>
{
public:
    explicit ExtensionWebContentsObserverQt(content::WebContents *web_contents);
    ~ExtensionWebContentsObserverQt() override;

    static void CreateForWebContents(content::WebContents *web_contents);

    // content::WebContentsObserver overrides.
    void RenderFrameCreated(content::RenderFrameHost *render_frame_host) override;

private:
    friend class content::WebContentsUserData<ExtensionWebContentsObserverQt>;
    WEB_CONTENTS_USER_DATA_KEY_DECL();
};

} // namespace extensions

#endif // EXTENSION_WEB_CONTENTS_OBSERVER_QT_H_
