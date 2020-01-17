/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

// based on chrome/renderer/chrome_render_thread_observer.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/render_thread_observer_qt.h"

#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

namespace QtWebEngineCore {

bool RenderThreadObserverQt::m_isIncognitoProcess = false;

void RenderThreadObserverQt::RegisterMojoInterfaces(blink::AssociatedInterfaceRegistry *associated_interfaces)
{
    associated_interfaces->AddInterface(
            base::Bind(&RenderThreadObserverQt::OnRendererConfigurationAssociatedRequest, base::Unretained(this)));
}

void RenderThreadObserverQt::UnregisterMojoInterfaces(blink::AssociatedInterfaceRegistry *associated_interfaces)
{
    associated_interfaces->RemoveInterface(qtwebengine::mojom::RendererConfiguration::Name_);
}

void RenderThreadObserverQt::SetInitialConfiguration(bool is_incognito_process)
{
    m_isIncognitoProcess = is_incognito_process;
}

void RenderThreadObserverQt::OnRendererConfigurationAssociatedRequest(
        mojo::PendingAssociatedReceiver<qtwebengine::mojom::RendererConfiguration> receiver)
{
    m_rendererConfigurationReceivers.Add(this, std::move(receiver));
}

} // namespace
