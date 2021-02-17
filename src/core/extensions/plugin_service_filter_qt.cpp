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

#include "extensions/plugin_service_filter_qt.h"

#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"

#include "web_contents_delegate_qt.h"

using namespace QtWebEngineCore;

namespace extensions {

// static
PluginServiceFilterQt *PluginServiceFilterQt::GetInstance() {
    return base::Singleton<PluginServiceFilterQt>::get();
}

bool PluginServiceFilterQt::IsPluginAvailable(int render_process_id,
                                              int render_frame_id,
                                              const GURL &url,
                                              const url::Origin &main_frame_origin,
                                              content::WebPluginInfo *plugin)
{
    content::RenderFrameHost *frame_host = content::RenderFrameHost::FromID(render_process_id, render_frame_id);
    content::WebContents *web_contents = content::WebContents::FromRenderFrameHost(frame_host);
    if (!web_contents) {
        // Availability checked somewhere before/during WebContents initialization. Let it load and enable
        // all the plugins at this phase. This information will be queried again when receiving the response
        // for the requested content. Postponing our decision and enabling/blocking there makes WebEngineSettings
        // modifiable in runtime without reconstructing WebContents.
        return true;
    }

    if (web_contents->IsInnerWebContentsForGuest())
        web_contents = web_contents->GetOuterWebContents();

    if (auto *delegate = static_cast<WebContentsDelegateQt *>(web_contents->GetDelegate())) {
        const WebEngineSettings *settings = delegate->webEngineSettings();
        if (!settings->testAttribute(WebEngineSettings::PdfViewerEnabled)
            || !settings->testAttribute(WebEngineSettings::PluginsEnabled))
            return false;
    }

    return true;
}

bool PluginServiceFilterQt::CanLoadPlugin(int render_process_id,
                                          const base::FilePath &path)
{
    return true;
}

PluginServiceFilterQt::PluginServiceFilterQt()
{
}

PluginServiceFilterQt::~PluginServiceFilterQt()
{
}

} // namespace extensions
