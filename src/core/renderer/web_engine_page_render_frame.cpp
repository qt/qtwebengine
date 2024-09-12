// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "renderer/web_engine_page_render_frame.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

#include "third_party/blink/public/common/metrics/document_update_reason.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/web/web_frame_content_dumper.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_view.h"
#include "third_party/blink/renderer/core/frame/web_local_frame_impl.h"
#include "third_party/blink/renderer/platform/heap/thread_state.h"

namespace {
// Forces layouting of document and it's subtree.
void updateStyleAndLayoutForTree(blink::Document *document)
{
    // Document::UpdateStyleAndLayout must run on the main thread of the render processs
    CHECK(WTF::IsMainThread());

    if (!document)
        return;

    document->UpdateStyleAndLayout(blink::DocumentUpdateReason::kUnknown);

    blink::Frame *frame = document->GetFrame();
    for (blink::Frame *child = frame->Tree().FirstChild(); child;
         child = child->Tree().TraverseNext(frame)) {
        if (child->IsLocalFrame())
            if (auto *localFrame = DynamicTo<blink::LocalFrame>(child))
                if (auto *doc = localFrame->GetDocument())
                    updateStyleAndLayoutForTree(doc);
    }
}
} // namespace

namespace QtWebEngineCore {

WebEnginePageRenderFrame::WebEnginePageRenderFrame(content::RenderFrame *render_frame)
    : content::RenderFrameObserver(render_frame), m_binding(this), m_ready(false)
{
    render_frame->GetAssociatedInterfaceRegistry()->AddInterface<qtwebenginepage::mojom::WebEnginePageRenderFrame>(
            base::BindRepeating(&WebEnginePageRenderFrame::BindReceiver, base::Unretained(this)));
}

void WebEnginePageRenderFrame::BindReceiver(
        mojo::PendingAssociatedReceiver<qtwebenginepage::mojom::WebEnginePageRenderFrame> receiver)
{
    m_binding.Bind(std::move(receiver));
}

void WebEnginePageRenderFrame::FetchDocumentMarkup(uint64_t requestId,
                                                   FetchDocumentMarkupCallback callback)
{
    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    blink::WebString markup;
    if (m_ready)
        markup = blink::WebFrameContentDumper::DumpAsMarkup(frame);
    else
        markup = blink::WebString::FromUTF8("<html><head></head><body></body></html>");
    std::move(callback).Run(requestId, markup.Utf8());
}

void WebEnginePageRenderFrame::FetchDocumentInnerText(uint64_t requestId,
                                                      FetchDocumentInnerTextCallback callback)
{
    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    blink::WebString text;
    if (m_ready) {
        auto *document = To<blink::WebLocalFrameImpl>(frame)->GetFrame()->GetDocument();
        updateStyleAndLayoutForTree(document);
        text = blink::WebFrameContentDumper::DumpFrameTreeAsText(
                frame, std::numeric_limits<int32_t>::max());
    }
    std::move(callback).Run(requestId, text.Utf8());
}

void WebEnginePageRenderFrame::SetBackgroundColor(uint32_t color)
{
    render_frame()->GetWebFrame()->View()->SetBaseBackgroundColorOverrideForInspector(color);
}

void WebEnginePageRenderFrame::OnDestruct()
{
    delete this;
}

void WebEnginePageRenderFrame::DidFinishLoad()
{
    m_ready = true;
}
}
