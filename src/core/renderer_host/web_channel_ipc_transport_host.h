// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WEB_CHANNEL_IPC_TRANSPORT_H
#define WEB_CHANNEL_IPC_TRANSPORT_H

#include "qtwebenginecoreglobal.h"

#include "content/public/browser/render_frame_host_receiver_set.h"
#include "content/public/browser/web_contents_observer.h"
#include "qtwebengine/browser/qtwebchannel.mojom.h"

#include <QWebChannelAbstractTransport>
#include <map>

QT_FORWARD_DECLARE_CLASS(QString)

namespace QtWebEngineCore {

class WebChannelIPCTransportHost
        : public QWebChannelAbstractTransport
        , private content::WebContentsObserver
        , qtwebchannel::mojom::WebChannelTransportHost
{
public:
    WebChannelIPCTransportHost(content::WebContents *webContents, uint32_t worldId = 0, QObject *parent = nullptr);
    ~WebChannelIPCTransportHost() override;

    void setWorldId(uint32_t worldId);
    uint32_t worldId() const;

    // QWebChannelAbstractTransport
    void sendMessage(const QJsonObject &message) override;

    void BindReceiver(
        mojo::PendingAssociatedReceiver<qtwebchannel::mojom::WebChannelTransportHost> receiver,
        content::RenderFrameHost *rfh);

private:
    void setWorldId(content::RenderFrameHost *frame, uint32_t worldId);
    void resetWorldId();

    const mojo::AssociatedRemote<qtwebchannel::mojom::WebChannelTransportRender> &
    GetWebChannelIPCTransportRemote(content::RenderFrameHost *rfh);

    // WebContentsObserver
    void RenderFrameCreated(content::RenderFrameHost *frame) override;
    void RenderFrameDeleted(content::RenderFrameHost *render_frame_host) override;

    // qtwebchannel::mojom::WebChannelTransportHost
    void DispatchWebChannelMessage(const std::vector<uint8_t> &json) override;

    // Empty only during construction/destruction. Synchronized to all the
    // WebChannelIPCTransports/RenderFrames in the observed WebContents.
    uint32_t m_worldId;
    content::RenderFrameHostReceiverSet<qtwebchannel::mojom::WebChannelTransportHost> m_receiver;
    std::map<content::RenderFrameHost *,
             mojo::AssociatedRemote<qtwebchannel::mojom::WebChannelTransportRender>>
            m_renderFrames;
};

} // namespace

#endif // WEB_CHANNEL_IPC_TRANSPORT_H
