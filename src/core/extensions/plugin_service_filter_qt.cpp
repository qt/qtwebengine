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

#include "content/public/browser/render_process_host.h"

#include "profile_adapter.h"
#include "profile_adapter_client.h"
#include "profile_qt.h"
#include "web_engine_settings.h"

using namespace QtWebEngineCore;

namespace extensions {

// static
PluginServiceFilterQt *PluginServiceFilterQt::GetInstance()
{
    return base::Singleton<PluginServiceFilterQt>::get();
}

bool PluginServiceFilterQt::IsPluginAvailable(int render_process_id,
                                              const content::WebPluginInfo &plugin)
{
    Q_UNUSED(plugin);
    content::RenderProcessHost *rph = content::RenderProcessHost::FromID(render_process_id);
    if (!rph)
      return false;

    ProfileQt *profile = static_cast<ProfileQt *>(rph->GetBrowserContext());
    for (auto *client : profile->profileAdapter()->clients()) {
        const WebEngineSettings *settings = client->coreSettings();
        if (!settings)
            return false;
        if (!settings->testAttribute(QWebEngineSettings::PdfViewerEnabled)
            || !settings->testAttribute(QWebEngineSettings::PluginsEnabled))
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
