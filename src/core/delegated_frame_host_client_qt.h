// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DELEGATED_FRAME_HOST_CLIENT_QT_H
#define DELEGATED_FRAME_HOST_CLIENT_QT_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

#include "content/browser/renderer_host/delegated_frame_host.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"

namespace QtWebEngineCore {

class RenderWidgetHostViewQt;
class DelegatedFrameHostClientQt : public content::DelegatedFrameHostClient
{
public:
    explicit DelegatedFrameHostClientQt(RenderWidgetHostViewQt *p) : p(p) {}

    // Overridden from content::DelegatedFrameHostClient
    ui::Layer *DelegatedFrameHostGetLayer() const override;
    bool DelegatedFrameHostIsVisible() const override;
    SkColor DelegatedFrameHostGetGutterColor() const override;
    void OnFrameTokenChanged(uint32_t frame_token,
                             base::TimeTicks activation_time) override;
    float GetDeviceScaleFactor() const override;
    void InvalidateLocalSurfaceIdOnEviction() override;
    std::vector<viz::SurfaceId> CollectSurfaceIdsForEviction() override;
    bool ShouldShowStaleContentOnEviction() override;

private:
    RenderWidgetHostViewQt *p;
};

} // namespace QtWebEngineCore

#endif // !DELEGATED_FRAME_HOST_CLIENT_QT_H
