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
