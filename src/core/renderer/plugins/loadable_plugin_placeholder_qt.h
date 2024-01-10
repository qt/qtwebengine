// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LOADALBLE_PLUGIN_PLACEHOLDER_QT_H
#define LOADALBLE_PLUGIN_PLACEHOLDER_QT_H

#include "components/plugins/renderer/loadable_plugin_placeholder.h"

namespace QtWebEngineCore {

class LoadablePluginPlaceholderQt final : public plugins::LoadablePluginPlaceholder
                                        , public gin::Wrappable<LoadablePluginPlaceholderQt>
{
public:
    static gin::WrapperInfo kWrapperInfo;

    // Creates a new WebViewPlugin with a MissingPlugin as a delegate.
    static LoadablePluginPlaceholderQt* CreateLoadableMissingPlugin(content::RenderFrame* render_frame,
                                                                    const blink::WebPluginParams& params);

private:
    LoadablePluginPlaceholderQt(content::RenderFrame* render_frame,
                                const blink::WebPluginParams& params,
                                const std::string& html_data,
                                const std::u16string& title);
    ~LoadablePluginPlaceholderQt() override;

    // content::LoadablePluginPlaceholder overrides.
    blink::WebPlugin* CreatePlugin() override;

    // WebViewPlugin::Delegate (via PluginPlaceholder) methods:
    v8::Local<v8::Value> GetV8Handle(v8::Isolate* isolate) override;
};

}  // namespace QtWebEngineCore

#endif  // LOADALBLE_PLUGIN_PLACEHOLDER_QT_H
