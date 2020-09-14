/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef DELEGATED_FRAME_HOST_CLIENT_QT_H
#define DELEGATED_FRAME_HOST_CLIENT_QT_H

#include "qtwebenginecoreglobal_p.h"

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
    void OnFrameTokenChanged(uint32_t frame_token) override;
    float GetDeviceScaleFactor() const override;
    void InvalidateLocalSurfaceIdOnEviction() override;
    std::vector<viz::SurfaceId> CollectSurfaceIdsForEviction() override;
    bool ShouldShowStaleContentOnEviction() override;

private:
    RenderWidgetHostViewQt *p;
};

} // namespace QtWebEngineCore

#endif // !DELEGATED_FRAME_HOST_CLIENT_QT_H
