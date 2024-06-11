// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef WEB_ENGINE_PAGE_RENDER_FRAME_H
#define WEB_ENGINE_PAGE_RENDER_FRAME_H

#include "content/public/renderer/render_frame_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "qtwebengine/browser/qtwebenginepage.mojom.h"

namespace content {
class RenderFrame;
}

namespace QtWebEngineCore {

class WebEnginePageRenderFrame : private content::RenderFrameObserver,
                                 public qtwebenginepage::mojom::WebEnginePageRenderFrame
{
public:
    WebEnginePageRenderFrame(content::RenderFrame *render_frame);
    WebEnginePageRenderFrame(const WebEnginePageRenderFrame &) = delete;
    WebEnginePageRenderFrame &operator=(const WebEnginePageRenderFrame &) = delete;

private:
    void FetchDocumentMarkup(uint64_t requestId, FetchDocumentMarkupCallback callback) override;
    void FetchDocumentInnerText(uint64_t requestId,
                                FetchDocumentInnerTextCallback callback) override;
    void SetBackgroundColor(uint32_t color) override;
    void OnDestruct() override;
    void DidFinishLoad() override;
    void
    BindReceiver(mojo::PendingAssociatedReceiver<qtwebenginepage::mojom::WebEnginePageRenderFrame>
                         receiver);

private:
    mojo::AssociatedReceiver<qtwebenginepage::mojom::WebEnginePageRenderFrame> m_binding;
    bool m_ready;
};
} // namespace QtWebEngineCore

#endif // WEB_ENGINE_PAGE_RENDER_FRAME_H
