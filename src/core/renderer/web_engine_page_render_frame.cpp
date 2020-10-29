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

#include "renderer/web_engine_page_render_frame.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_frame.h"
#include "third_party/blink/public/web/web_frame_content_dumper.h"
#include "third_party/blink/public/web/web_frame_widget.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_view.h"

namespace QtWebEngineCore {

WebEnginePageRenderFrame::WebEnginePageRenderFrame(content::RenderFrame *render_frame)
    : content::RenderFrameObserver(render_frame), m_binding(this)
{
    render_frame->GetAssociatedInterfaceRegistry()->AddInterface(
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
    blink::WebString markup =
            blink::WebFrameContentDumper::DumpAsMarkup(render_frame()->GetWebFrame());
    std::move(callback).Run(requestId, markup.Utf8());
}

void WebEnginePageRenderFrame::FetchDocumentInnerText(uint64_t requestId,
                                                      FetchDocumentInnerTextCallback callback)
{
    blink::WebString text = blink::WebFrameContentDumper::DumpWebViewAsText(
            render_frame()->GetWebFrame()->View(), std::numeric_limits<std::size_t>::max());
    std::move(callback).Run(requestId, text.Utf8());
}

void WebEnginePageRenderFrame::SetBackgroundColor(uint32_t color)
{
    render_frame()->GetWebFrame()->View()->SetBaseBackgroundColorOverride(color);
}

void WebEnginePageRenderFrame::OnDestruct()
{
    delete this;
}

}
