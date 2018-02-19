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

#ifndef WEB_CHANNEL_IPC_TRANSPORT_H
#define WEB_CHANNEL_IPC_TRANSPORT_H

#include "qtwebenginecoreglobal.h"

#include "content/public/browser/web_contents_observer.h"

#include <QWebChannelAbstractTransport>

QT_FORWARD_DECLARE_CLASS(QString)

namespace QtWebEngineCore {

class WebChannelIPCTransportHost : public QWebChannelAbstractTransport
                                 , private content::WebContentsObserver {
public:
    WebChannelIPCTransportHost(content::WebContents *webContents, uint worldId = 0, QObject *parent = nullptr);
    virtual ~WebChannelIPCTransportHost();

    void setWorldId(uint worldId) { setWorldId(base::Optional<uint>(worldId)); }
    uint worldId() const { return *m_worldId; }

    // QWebChannelAbstractTransport
    void sendMessage(const QJsonObject &message) override;

private:
    void setWorldId(base::Optional<uint> worldId);
    void setWorldId(content::RenderFrameHost *frame, base::Optional<uint> worldId);
    void onWebChannelMessage(const std::vector<char> &message);

    // WebContentsObserver
    void RenderFrameCreated(content::RenderFrameHost *frame) override;
    bool OnMessageReceived(const IPC::Message& message, content::RenderFrameHost *receiver) override;

    // Empty only during construction/destruction. Synchronized to all the
    // WebChannelIPCTransports/RenderFrames in the observed WebContents.
    base::Optional<uint> m_worldId;
};

} // namespace

#endif // WEB_CHANNEL_IPC_TRANSPORT_H
