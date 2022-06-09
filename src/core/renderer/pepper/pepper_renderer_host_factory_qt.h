// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PEPPER_RENDERER_HOST_FACTORY_QT_H
#define PEPPER_RENDERER_HOST_FACTORY_QT_H

#include "ppapi/host/host_factory.h"
#include "content/public/renderer/render_frame_observer.h"


namespace content {
class RenderFrame;
}

namespace QtWebEngineCore {

class PepperRendererHostFactoryQt : public ppapi::host::HostFactory {
public:
    explicit PepperRendererHostFactoryQt(content::RendererPpapiHost* host);
    ~PepperRendererHostFactoryQt();

    // HostFactory.
    std::unique_ptr<ppapi::host::ResourceHost> CreateResourceHost(
            ppapi::host::PpapiHost* host,
            PP_Resource resource,
            PP_Instance instance,
            const IPC::Message& message) override;

private:
    // Not owned by this object.
    content::RendererPpapiHost* host_;
};

} // namespace QtWebEngineCore

#endif // PEPPER_RENDERER_HOST_FACTORY_QT_H
