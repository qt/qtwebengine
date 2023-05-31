// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PLUGIN_SERVICE_FILTER_QT
#define PLUGIN_SERVICE_FILTER_QT

#include "content/public/browser/plugin_service_filter.h"

#include "base/memory/singleton.h"

namespace extensions {

class PluginServiceFilterQt : public content::PluginServiceFilter {
public:
    static PluginServiceFilterQt* GetInstance();

    bool IsPluginAvailable(int render_process_id, int render_frame_id,
                           content::BrowserContext *browser_context,
                           const content::WebPluginInfo &plugin) override;

    bool CanLoadPlugin(int render_process_id,
                       const base::FilePath &path) override;

private:
    friend struct base::DefaultSingletonTraits<PluginServiceFilterQt>;

    PluginServiceFilterQt();
    ~PluginServiceFilterQt();
};

} // namespace extensions

#endif // PLUGIN_SERVICE_FILTER_QT
