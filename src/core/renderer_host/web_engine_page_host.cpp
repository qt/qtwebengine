/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "web_engine_page_host.h"

#include "qtwebengine/browser/qtwebenginepage.mojom.h"
#include "content/public/browser/web_contents.h"

#include "render_widget_host_view_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"

namespace QtWebEngineCore {

WebEnginePageHost::WebEnginePageHost(content::WebContents *webContents,
                                     WebContentsAdapterClient *adapterClient)
    : content::WebContentsObserver(webContents), m_adapterClient(adapterClient)
{
}

void WebEnginePageHost::FetchDocumentMarkup(uint64_t requestId)
{
    auto &remote = GetWebEnginePageRenderFrame(web_contents()->GetMainFrame());
    remote->FetchDocumentMarkup(
            requestId,
            base::BindOnce(&WebEnginePageHost::OnDidFetchDocumentMarkup, base::Unretained(this)));
}

void WebEnginePageHost::FetchDocumentInnerText(uint64_t requestId)
{
    auto &remote = GetWebEnginePageRenderFrame(web_contents()->GetMainFrame());
    remote->FetchDocumentInnerText(requestId,
                                   base::BindOnce(&WebEnginePageHost::OnDidFetchDocumentInnerText,
                                                  base::Unretained(this)));
}

void WebEnginePageHost::OnDidFetchDocumentMarkup(uint64_t requestId, const std::string &markup)
{
    m_adapterClient->didFetchDocumentMarkup(requestId, toQt(markup));
}

void WebEnginePageHost::OnDidFetchDocumentInnerText(uint64_t requestId,
                                                    const std::string &innerText)
{
    m_adapterClient->didFetchDocumentInnerText(requestId, toQt(innerText));
}

void WebEnginePageHost::RenderFrameDeleted(content::RenderFrameHost *render_frame)
{
    m_renderFrames.erase(render_frame);
}

void WebEnginePageHost::SetBackgroundColor(uint32_t color)
{
    auto &remote = GetWebEnginePageRenderFrame(web_contents()->GetMainFrame());
    remote->SetBackgroundColor(color);
}

const WebEnginePageRenderFrameRemote &
WebEnginePageHost::GetWebEnginePageRenderFrame(content::RenderFrameHost *rfh)
{
    auto it = m_renderFrames.find(rfh);
    if (it == m_renderFrames.end()) {
        WebEnginePageRenderFrameRemote remote;
        rfh->GetRemoteAssociatedInterfaces()->GetInterface(remote.BindNewEndpointAndPassReceiver());
        it = m_renderFrames.insert(std::make_pair(rfh, std::move(remote))).first;
    } else if (it->second.is_bound() && !it->second.is_connected()) {
        it->second.reset();
        rfh->GetRemoteAssociatedInterfaces()->GetInterface(&it->second);
    }

    return it->second;
}

} // namespace QtWebEngineCore
