// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
    auto &remote = GetWebEnginePageRenderFrame(web_contents()->GetPrimaryMainFrame());
    remote->FetchDocumentMarkup(
            requestId,
            base::BindOnce(&WebEnginePageHost::OnDidFetchDocumentMarkup, base::Unretained(this)));
}

void WebEnginePageHost::FetchDocumentInnerText(uint64_t requestId)
{
    auto &remote = GetWebEnginePageRenderFrame(web_contents()->GetPrimaryMainFrame());
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
    auto &remote = GetWebEnginePageRenderFrame(web_contents()->GetPrimaryMainFrame());
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
