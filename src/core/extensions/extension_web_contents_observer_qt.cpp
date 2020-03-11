/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extension_web_contents_observer_qt.h"

#include "content/public/browser/child_process_security_policy.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/common/url_constants.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/manifest.h"

namespace extensions {

ExtensionWebContentsObserverQt::ExtensionWebContentsObserverQt(content::WebContents *web_contents)
    : ExtensionWebContentsObserver(web_contents)
{
}

ExtensionWebContentsObserverQt::~ExtensionWebContentsObserverQt()
{
}

// static
void ExtensionWebContentsObserverQt::CreateForWebContents(content::WebContents *web_contents)
{
    content::WebContentsUserData<ExtensionWebContentsObserverQt>::CreateForWebContents(web_contents);

    // Initialize this instance if necessary.
    FromWebContents(web_contents)->Initialize();
}

void ExtensionWebContentsObserverQt::RenderFrameCreated(content::RenderFrameHost *render_frame_host)
{
    ExtensionWebContentsObserver::RenderFrameCreated(render_frame_host);

    const Extension *extension = GetExtensionFromFrame(render_frame_host, false);
    if (!extension)
        return;

    int process_id = render_frame_host->GetProcess()->GetID();
    auto *policy = content::ChildProcessSecurityPolicy::GetInstance();

    if (extension->is_extension() && Manifest::IsComponentLocation(extension->location()))
        policy->GrantRequestOrigin(process_id, url::Origin::Create(GURL(content::kChromeUIResourcesURL)));
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(ExtensionWebContentsObserverQt)

} // namespace extensions
