// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "loadable_plugin_placeholder_qt.h"
#include "qtwebenginecoreglobal_p.h"

#include "content/public/renderer/render_frame.h"
#include "components/strings/grit/components_strings.h"
#include "chrome/grit/renderer_resources.h"
#include "gin/handle.h"
#include "gin/wrappable.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/webui/jstemplate_builder.h"

namespace QtWebEngineCore {

// static
gin::WrapperInfo LoadablePluginPlaceholderQt::kWrapperInfo = {gin::kEmbedderNativeGin};

LoadablePluginPlaceholderQt::LoadablePluginPlaceholderQt(content::RenderFrame* render_frame,
                                                         const blink::WebPluginParams& params,
                                                         const std::string& html_data,
                                                         const std::u16string& title)
    : plugins::LoadablePluginPlaceholder(render_frame, params, html_data)
{}

LoadablePluginPlaceholderQt::~LoadablePluginPlaceholderQt()
{
}

// TODO(bauerb): Move this method to NonLoadablePluginPlaceholder?
// static
LoadablePluginPlaceholderQt* LoadablePluginPlaceholderQt::CreateLoadableMissingPlugin(content::RenderFrame* render_frame,
                                                                                      const blink::WebPluginParams& params)
{
    std::string template_html(ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(IDR_BLOCKED_PLUGIN_HTML));

    base::Value::Dict values;
    values.Set("name", "");
    values.Set("message", l10n_util::GetStringUTF8(IDS_PLUGIN_NOT_SUPPORTED));

    const std::string html_data = webui::GetI18nTemplateHtml(template_html, std::move(values));

    // Will destroy itself when its WebViewPlugin is going away.
    return new LoadablePluginPlaceholderQt(render_frame, params, html_data, params.mime_type.Utf16());
}

blink::WebPlugin* LoadablePluginPlaceholderQt::CreatePlugin()
{
    QT_NOT_YET_IMPLEMENTED
    return nullptr;
}

v8::Local<v8::Value> LoadablePluginPlaceholderQt::GetV8Handle(v8::Isolate* isolate)
{
    return gin::CreateHandle(isolate, this).ToV8();
}

}  // namespace QtWebEngineCore
