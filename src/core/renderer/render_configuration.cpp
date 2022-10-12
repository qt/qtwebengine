// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/renderer/chrome_render_thread_observer.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/render_configuration.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

namespace QtWebEngineCore {

bool RenderConfiguration::m_isIncognitoProcess = false;

void RenderConfiguration::RegisterMojoInterfaces(
        blink::AssociatedInterfaceRegistry *associated_interfaces)
{
    associated_interfaces->AddInterface<qtwebengine::mojom::RendererConfiguration>(
            base::BindRepeating(&RenderConfiguration::OnRendererConfigurationAssociatedRequest,
                                base::Unretained(this)));
}

void RenderConfiguration::UnregisterMojoInterfaces(
        blink::AssociatedInterfaceRegistry *associated_interfaces)
{
    associated_interfaces->RemoveInterface(qtwebengine::mojom::RendererConfiguration::Name_);
}

void RenderConfiguration::SetInitialConfiguration(bool is_incognito_process)
{
    m_isIncognitoProcess = is_incognito_process;
}

void RenderConfiguration::OnRendererConfigurationAssociatedRequest(
        mojo::PendingAssociatedReceiver<qtwebengine::mojom::RendererConfiguration> receiver)
{
    m_rendererConfigurationReceivers.Add(this, std::move(receiver));
}

} // namespace
