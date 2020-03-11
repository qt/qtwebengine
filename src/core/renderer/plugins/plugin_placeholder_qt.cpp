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

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plugin_placeholder_qt.h"

#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/v8_value_converter.h"
#include "gin/object_template_builder.h"

namespace QtWebEngineCore {

// static
gin::WrapperInfo PluginPlaceholderQt::kWrapperInfo = {gin::kEmbedderNativeGin};

PluginPlaceholderQt::PluginPlaceholderQt(content::RenderFrame* render_frame,
                                         const blink::WebPluginParams& params,
                                         const std::string& html_data)
    : PluginPlaceholderBase(render_frame, params, html_data)
{}

PluginPlaceholderQt::~PluginPlaceholderQt() {}

v8::Local<v8::Value> PluginPlaceholderQt::GetV8Handle(v8::Isolate* isolate)
{
    return gin::CreateHandle(isolate, this).ToV8();
}

gin::ObjectTemplateBuilder PluginPlaceholderQt::GetObjectTemplateBuilder(v8::Isolate* isolate)
{
    return gin::Wrappable<PluginPlaceholderQt>::GetObjectTemplateBuilder(isolate)
        .SetMethod<void (QtWebEngineCore::PluginPlaceholderQt::*)()>(
            "hide", &PluginPlaceholderQt::HideCallback);
}

}  // namespace QtWebEngineCore
