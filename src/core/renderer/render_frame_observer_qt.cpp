// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// This is based on chrome/renderer/pepper/pepper_helper.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "render_frame_observer_qt.h"

#include "components/web_cache/renderer/web_cache_impl.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/web/web_document_loader.h"

#if QT_CONFIG(webengine_pepper_plugins)
#include "base/memory/ptr_util.h"
#include "chrome/renderer/pepper/pepper_shared_memory_message_filter.h"
#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"

#include "renderer/pepper/pepper_renderer_host_factory_qt.h"
#endif

namespace QtWebEngineCore {

RenderFrameObserverQt::RenderFrameObserverQt(content::RenderFrame *render_frame, web_cache::WebCacheImpl *web_cache_impl)
    : RenderFrameObserver(render_frame)
    , RenderFrameObserverTracker<RenderFrameObserverQt>(render_frame)
    , m_isFrameDetached(false)
    , m_web_cache_impl(web_cache_impl)
{}

RenderFrameObserverQt::~RenderFrameObserverQt() {}

void RenderFrameObserverQt::OnDestruct()
{
    delete this;
}

#if QT_CONFIG(webengine_pepper_plugins)
void RenderFrameObserverQt::DidCreatePepperPlugin(content::RendererPpapiHost *host)
{
    host->GetPpapiHost()->AddHostFactoryFilter(base::WrapUnique(new PepperRendererHostFactoryQt(host)));
    host->GetPpapiHost()->AddInstanceMessageFilter(base::WrapUnique(new PepperSharedMemoryMessageFilter(host)));
}
#endif

bool RenderFrameObserverQt::OnAssociatedInterfaceRequestForFrame(const std::string &interface_name,
                                                                 mojo::ScopedInterfaceEndpointHandle *handle)
{
    return m_associated_interfaces.TryBindInterface(interface_name, handle);
}

void RenderFrameObserverQt::WillDetach()
{
    m_isFrameDetached = true;
}

bool RenderFrameObserverQt::isFrameDetached() const
{
    return m_isFrameDetached;
}

void RenderFrameObserverQt::ReadyToCommitNavigation(blink::WebDocumentLoader *)
{
    if (render_frame()->IsMainFrame() && m_web_cache_impl)
        m_web_cache_impl->ExecutePendingClearCache();
}

} // namespace QtWebEngineCore
