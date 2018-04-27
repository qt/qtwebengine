/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "web_channel_ipc_transport_host.h"

#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"

#include "common/qt_messages.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

#include <QtCore/private/qjson_p.h>

namespace QtWebEngineCore {

Q_LOGGING_CATEGORY(log, "qt.webengine.webchanneltransport");

inline QDebug operator<<(QDebug stream, content::RenderFrameHost *frame)
{
    return stream << "frame " << frame->GetRoutingID() << " in process " << frame->GetProcess()->GetID();
}

template <class T>
inline QDebug operator<<(QDebug stream, const base::Optional<T> &opt)
{
    if (opt)
        return stream << *opt;
    else
        return stream << "nullopt";
}

WebChannelIPCTransportHost::WebChannelIPCTransportHost(content::WebContents *contents, uint worldId, QObject *parent)
    : QWebChannelAbstractTransport(parent)
    , content::WebContentsObserver(contents)
{
    setWorldId(worldId);
}

WebChannelIPCTransportHost::~WebChannelIPCTransportHost()
{
    setWorldId(base::nullopt);
}

void WebChannelIPCTransportHost::sendMessage(const QJsonObject &message)
{
    QJsonDocument doc(message);
    int size = 0;
    const char *rawData = doc.rawData(&size);
    content::RenderFrameHost *frame = web_contents()->GetMainFrame();
    qCDebug(log).nospace() << "sending webchannel message to " << frame << ": " << doc;
    frame->Send(new WebChannelIPCTransport_Message(frame->GetRoutingID(), std::vector<char>(rawData, rawData + size), *m_worldId));
}

void WebChannelIPCTransportHost::setWorldId(base::Optional<uint> worldId)
{
    if (m_worldId == worldId)
        return;
    for (content::RenderFrameHost *frame : web_contents()->GetAllFrames())
        setWorldId(frame, worldId);
    m_worldId = worldId;
}

void WebChannelIPCTransportHost::setWorldId(content::RenderFrameHost *frame, base::Optional<uint> worldId)
{
    if (!frame->IsRenderFrameLive())
        return;
    qCDebug(log).nospace() << "sending setWorldId(" << worldId << ") message to " << frame;
    frame->Send(new WebChannelIPCTransport_SetWorldId(frame->GetRoutingID(), worldId));
}

void WebChannelIPCTransportHost::onWebChannelMessage(const std::vector<char> &message)
{
    content::RenderFrameHost *frame = web_contents()->GetMainFrame();

    QJsonDocument doc;
    // QJsonDocument::fromRawData does not check the length before it starts
    // parsing the QJsonPrivate::Header and QJsonPrivate::Base structures.
    if (message.size() >= sizeof(QJsonPrivate::Header) + sizeof(QJsonPrivate::Base))
        doc = QJsonDocument::fromRawData(message.data(), message.size());

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

bool WebChannelIPCTransportHost::OnMessageReceived(const IPC::Message& message, content::RenderFrameHost *receiver)
{
    if (receiver != web_contents()->GetMainFrame())
        return false;

    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(WebChannelIPCTransportHost, message)
        IPC_MESSAGE_HANDLER(WebChannelIPCTransportHost_SendMessage, onWebChannelMessage)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled;
}

} // namespace QtWebEngineCore
