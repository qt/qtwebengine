/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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
                                                         const base::string16& title)
    : plugins::LoadablePluginPlaceholder(render_frame, params, html_data)
    , context_menu_request_id_(0)
{}

LoadablePluginPlaceholderQt::~LoadablePluginPlaceholderQt()
{
    if (context_menu_request_id_ && render_frame())
        render_frame()->CancelContextMenu(context_menu_request_id_);
}

// TODO(bauerb): Move this method to NonLoadablePluginPlaceholder?
// static
LoadablePluginPlaceholderQt* LoadablePluginPlaceholderQt::CreateLoadableMissingPlugin(content::RenderFrame* render_frame,
                                                                                      const blink::WebPluginParams& params)
{
    const base::StringPiece template_html(ui::ResourceBundle::GetSharedInstance().GetRawDataResource(IDR_BLOCKED_PLUGIN_HTML));

    base::DictionaryValue values;
    values.SetString("name", "");
    values.SetString("message", l10n_util::GetStringUTF8(IDS_PLUGIN_NOT_SUPPORTED));

    const std::string html_data = webui::GetI18nTemplateHtml(template_html, &values);

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
