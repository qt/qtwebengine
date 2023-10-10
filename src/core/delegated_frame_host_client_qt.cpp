// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "delegated_frame_host_client_qt.h"

#include "render_widget_host_view_qt.h"

namespace QtWebEngineCore {

ui::Layer *DelegatedFrameHostClientQt::DelegatedFrameHostGetLayer() const
{
    return p->m_rootLayer.get();
}

bool DelegatedFrameHostClientQt::DelegatedFrameHostIsVisible() const
{
    return !p->host()->is_hidden();
}

SkColor DelegatedFrameHostClientQt::DelegatedFrameHostGetGutterColor() const
{
    return p->GetBackgroundColor().value_or(SK_ColorWHITE);
}

void DelegatedFrameHostClientQt::OnFrameTokenChanged(uint32_t frame_token, base::TimeTicks activation_time)
{
    p->OnFrameTokenChangedForView(frame_token, activation_time);
}

float DelegatedFrameHostClientQt::GetDeviceScaleFactor() const
{
    return p->GetScreenInfo().device_scale_factor;
}

void DelegatedFrameHostClientQt::InvalidateLocalSurfaceIdOnEviction()
{
    p->m_dfhLocalSurfaceIdAllocator.Invalidate();
}

std::vector<viz::SurfaceId> DelegatedFrameHostClientQt::CollectSurfaceIdsForEviction()
{
    return p->host()->CollectSurfaceIdsForEviction();
}

bool DelegatedFrameHostClientQt::ShouldShowStaleContentOnEviction()
{
    return p->host()->ShouldShowStaleContentOnEviction();
}

} // namespace QtWebEngineCore
