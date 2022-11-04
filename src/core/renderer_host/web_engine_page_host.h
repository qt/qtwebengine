// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WEB_ENGINE_PAGE_HOST_H
#define WEB_ENGINE_PAGE_HOST_H

#include "content/public/browser/web_contents_observer.h"

#include <QtGlobal>

namespace content {
class WebContents;
}

namespace mojo {
template<typename Type>
class AssociatedRemote;
}

namespace qtwebenginepage {
namespace mojom {
class WebEnginePageRenderFrame;
}
}

namespace QtWebEngineCore {

using WebEnginePageRenderFrameRemote = mojo::AssociatedRemote<qtwebenginepage::mojom::WebEnginePageRenderFrame>;

class WebContentsAdapterClient;

class WebEnginePageHost : public content::WebContentsObserver
{
public:
    WebEnginePageHost(content::WebContents *, WebContentsAdapterClient *adapterClient);
    void FetchDocumentMarkup(uint64_t requestId);
    void FetchDocumentInnerText(uint64_t requestId);
    void RenderFrameDeleted(content::RenderFrameHost *render_frame) override;
    void SetBackgroundColor(uint32_t color);

private:
    void OnDidFetchDocumentMarkup(uint64_t requestId, const std::string &markup);
    void OnDidFetchDocumentInnerText(uint64_t requestId, const std::string &innerText);
    const WebEnginePageRenderFrameRemote &
    GetWebEnginePageRenderFrame(content::RenderFrameHost *rfh);

private:
    WebContentsAdapterClient *m_adapterClient;
    std::map<content::RenderFrameHost *, WebEnginePageRenderFrameRemote> m_renderFrames;
};

} // namespace QtWebEngineCore

#endif // WEB_ENGINE_PAGE_HOST_H
