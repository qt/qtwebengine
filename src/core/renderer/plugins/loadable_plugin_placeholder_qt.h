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

#ifndef LOADALBLE_PLUGIN_PLACEHOLDER_QT_H
#define LOADALBLE_PLUGIN_PLACEHOLDER_QT_H

#include "base/macros.h"
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
                                const base::string16& title);
    ~LoadablePluginPlaceholderQt() override;

    // content::LoadablePluginPlaceholder overrides.
    blink::WebPlugin* CreatePlugin() override;
    void OnBlockedContent(content::RenderFrame::PeripheralContentStatus status,
                          bool is_same_origin) override {}

    // WebViewPlugin::Delegate (via PluginPlaceholder) methods:
    v8::Local<v8::Value> GetV8Handle(v8::Isolate* isolate) override;

    int context_menu_request_id_;  // Nonzero when request pending.

    DISALLOW_COPY_AND_ASSIGN(LoadablePluginPlaceholderQt);
};

}  // namespace QtWebEngineCore

#endif  // LOADALBLE_PLUGIN_PLACEHOLDER_QT_H
