// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef RENDER_FRAME_OBSERVER_QT_H
#define RENDER_FRAME_OBSERVER_QT_H

#include "qtwebenginecoreglobal_p.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame_observer_tracker.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

namespace content {
class RenderFrame;
}
namespace web_cache {
class WebCacheImpl;
}

namespace QtWebEngineCore {

class RenderFrameObserverQt
    : public content::RenderFrameObserver
    , public content::RenderFrameObserverTracker<RenderFrameObserverQt>
{
public:
    explicit RenderFrameObserverQt(content::RenderFrame *render_frame, web_cache::WebCacheImpl *web_cache_impl);
    ~RenderFrameObserverQt();

#if QT_CONFIG(webengine_pepper_plugins)
    void DidCreatePepperPlugin(content::RendererPpapiHost *host) override;
#endif
    bool OnAssociatedInterfaceRequestForFrame(
        const std::string &interface_name,
        mojo::ScopedInterfaceEndpointHandle *handle) override;
    void OnDestruct() override;
    void WillDetach() override;

    bool isFrameDetached() const;

    service_manager::BinderRegistry *registry() { return &registry_; }
    blink::AssociatedInterfaceRegistry *associatedInterfaces() {
        return &m_associated_interfaces;
    }

private:
    void ReadyToCommitNavigation(blink::WebDocumentLoader *) override;

    bool m_isFrameDetached;
    service_manager::BinderRegistry registry_;
    blink::AssociatedInterfaceRegistry m_associated_interfaces;
    web_cache::WebCacheImpl *m_web_cache_impl;
};

} // namespace QtWebEngineCore

#endif // RENDER_FRAME_OBSERVER_QT_H
