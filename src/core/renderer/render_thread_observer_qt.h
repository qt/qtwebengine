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

#ifndef RENDER_THREAD_OBSERVER_QT_H
#define RENDER_THREAD_OBSERVER_QT_H

#include "content/public/renderer/render_thread_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver_set.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "qtwebengine/common/renderer_configuration.mojom.h"

namespace QtWebEngineCore {

class RenderThreadObserverQt
    : public content::RenderThreadObserver
    , public qtwebengine::mojom::RendererConfiguration
{
public:
    RenderThreadObserverQt() = default;
    ~RenderThreadObserverQt() override = default;

    static bool is_incognito_process() { return m_isIncognitoProcess; }

private:
    // content::RenderThreadObserver:
    void RegisterMojoInterfaces(blink::AssociatedInterfaceRegistry *associated_interfaces) override;
    void UnregisterMojoInterfaces(blink::AssociatedInterfaceRegistry *associated_interfaces) override;

    // qtwebengine::mojom::RendererConfiguration:
    void SetInitialConfiguration(bool is_incognito_process) override;

    void OnRendererConfigurationAssociatedRequest(
            mojo::PendingAssociatedReceiver<qtwebengine::mojom::RendererConfiguration> receiver);

    static bool m_isIncognitoProcess;

    mojo::AssociatedReceiverSet<qtwebengine::mojom::RendererConfiguration> m_rendererConfigurationReceivers;

    DISALLOW_COPY_AND_ASSIGN(RenderThreadObserverQt);
};

} // namespace QtWebEngineCore

#endif // RENDER_THREAD_OBSERVER_QT_H
