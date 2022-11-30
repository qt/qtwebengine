// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "web_channel_ipc_transport_host.h"

#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "services/service_manager/public/cpp/interface_provider.h"
#include "qtwebengine/browser/qtwebchannel.mojom.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

namespace QtWebEngineCore {

Q_LOGGING_CATEGORY(log, "qt.webengine.webchanneltransport")

inline QDebug operator<<(QDebug stream, content::RenderFrameHost *frame)
{
    return stream << "frame " << frame->GetRoutingID() << " in process " << frame->GetProcess()->GetID();
}

WebChannelIPCTransportHost::WebChannelIPCTransportHost(content::WebContents *contents, uint worldId, QObject *parent)
    : QWebChannelAbstractTransport(parent)
    , content::WebContentsObserver(contents)
    , m_worldId(worldId)
    , m_receiver(contents, this)
{
    contents->ForEachRenderFrameHost([this, worldId](content::RenderFrameHost *frame) {
                                         setWorldId(frame, worldId);
                                     });
}

WebChannelIPCTransportHost::~WebChannelIPCTransportHost()
{
    resetWorldId();
}

uint WebChannelIPCTransportHost::worldId() const
{
    return m_worldId;
}

void WebChannelIPCTransportHost::sendMessage(const QJsonObject &message)
{
    QJsonDocument doc(message);
    QByteArray json = doc.toJson(QJsonDocument::Compact);
    content::RenderFrameHost *frame = web_contents()->GetPrimaryMainFrame();
    qCDebug(log).nospace() << "sending webchannel message to " << frame << ": " << doc;
    GetWebChannelIPCTransportRemote(frame)->DispatchWebChannelMessage(
            std::vector<uint8_t>(json.begin(), json.end()), m_worldId);
}

void WebChannelIPCTransportHost::setWorldId(uint32_t worldId)
{
    if (m_worldId == worldId)
        return;
    web_contents()->ForEachRenderFrameHost([this, worldId](content::RenderFrameHost *frame) {
                                               setWorldId(frame, worldId);
                                           });
    m_worldId = worldId;
}

void WebChannelIPCTransportHost::setWorldId(content::RenderFrameHost *frame, uint32_t worldId)
{
    if (!frame->IsRenderFrameLive())
        return;
    qCDebug(log).nospace() << "sending setWorldId(" << worldId << ") message to " << frame;
    GetWebChannelIPCTransportRemote(frame)->SetWorldId(worldId);
}

void WebChannelIPCTransportHost::resetWorldId()
{
    web_contents()->ForEachRenderFrameHost([this] (content::RenderFrameHost *frame) {
        if (!frame->IsRenderFrameLive())
            return;
        GetWebChannelIPCTransportRemote(frame)->ResetWorldId();
    });
}

void WebChannelIPCTransportHost::DispatchWebChannelMessage(const std::vector<uint8_t> &json)
{
    content::RenderFrameHost *frame = web_contents()->GetPrimaryMainFrame();

    if (m_receiver.GetCurrentTargetFrame() != frame) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(
            QByteArray(reinterpret_cast<const char *>(json.data()), json.size()));

    if (!doc.isObject()) {
        qCCritical(log).nospace() << "received invalid webchannel message from " << frame;
        return;
    }

    qCDebug(log).nospace() << "received webchannel message from " << frame << ": " << doc;
    Q_EMIT messageReceived(doc.object(), this);
}

void WebChannelIPCTransportHost::RenderFrameCreated(content::RenderFrameHost *frame)
{
    setWorldId(frame, m_worldId);
}

void WebChannelIPCTransportHost::RenderFrameDeleted(content::RenderFrameHost *rfh)
{
    m_renderFrames.erase(rfh);
}

void WebChannelIPCTransportHost::BindReceiver(
        mojo::PendingAssociatedReceiver<qtwebchannel::mojom::WebChannelTransportHost> receiver,
        content::RenderFrameHost *rfh)
{
     m_receiver.Bind(rfh, std::move(receiver));
}


const mojo::AssociatedRemote<qtwebchannel::mojom::WebChannelTransportRender> &
WebChannelIPCTransportHost::GetWebChannelIPCTransportRemote(content::RenderFrameHost *rfh)
{
    auto it = m_renderFrames.find(rfh);
    if (it == m_renderFrames.end()) {
        mojo::AssociatedRemote<qtwebchannel::mojom::WebChannelTransportRender> remote;
        rfh->GetRemoteAssociatedInterfaces()->GetInterface(remote.BindNewEndpointAndPassReceiver());
        it = m_renderFrames.insert(std::make_pair(rfh, std::move(remote))).first;
    } else if (it->second.is_bound() && !it->second.is_connected()) {
        it->second.reset();
        rfh->GetRemoteAssociatedInterfaces()->GetInterface(&it->second);
    }

    return it->second;
}

} // namespace QtWebEngineCore
