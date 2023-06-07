// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WEB_CHANNEL_IPC_TRANSPORT_H
#define WEB_CHANNEL_IPC_TRANSPORT_H

#include "content/public/renderer/render_frame_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "qtwebengine/browser/qtwebchannel.mojom.h"

#include <QtCore/qglobal.h>

namespace QtWebEngineCore {

class WebChannelIPCTransport
    : private content::RenderFrameObserver
    , public qtwebchannel::mojom::WebChannelTransportRender
{
public:
    WebChannelIPCTransport(content::RenderFrame *);

private:
    // qtwebchannel::mojom::WebChannelTransportRender
    void SetWorldId(uint32_t worldId) override;
    void ResetWorldId() override;
    void DispatchWebChannelMessage(const std::vector<uint8_t> &json, uint32_t worldId) override;

    // RenderFrameObserver
    void DidCreateScriptContext(v8::Local<v8::Context> context, int32_t worldId) override;
    void WillReleaseScriptContext(v8::Local<v8::Context> context, int worldId) override;
    void OnDestruct() override;
    void BindReceiver(mojo::PendingAssociatedReceiver<qtwebchannel::mojom::WebChannelTransportRender> receiver);

private:
    // The worldId from our WebChannelIPCTransportHost or empty when there is no
    // WebChannelIPCTransportHost.
    uint32_t m_worldId;
    bool m_worldInitialized;
    // True means it's currently OK to manipulate the frame's script context.
    bool m_canUseContext = false;
    mojo::AssociatedReceiver<qtwebchannel::mojom::WebChannelTransportRender> m_binding;
};

} // namespace

#endif // WEB_CHANNEL_IPC_TRANSPORT
