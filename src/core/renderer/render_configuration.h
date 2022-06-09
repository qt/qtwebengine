// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef RENDER_CONFIGURATION_H
#define RENDER_CONFIGURATION_H

#include "content/public/renderer/render_thread_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver_set.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "qtwebengine/common/renderer_configuration.mojom.h"

namespace QtWebEngineCore {

class RenderConfiguration : public content::RenderThreadObserver,
                            public qtwebengine::mojom::RendererConfiguration
{
public:
    RenderConfiguration() = default;
    ~RenderConfiguration() override = default;

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

    mojo::AssociatedReceiverSet<qtwebengine::mojom::RendererConfiguration>
            m_rendererConfigurationReceivers;
};

} // namespace QtWebEngineCore

#endif // RENDER_CONFIGURATION_H
