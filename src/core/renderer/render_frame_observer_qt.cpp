/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "render_frame_observer_qt.h"

#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"

#include "renderer/pepper/pepper_renderer_host_factory_qt.h"
#include "renderer/pepper/pepper_flash_renderer_host_qt.h"

namespace QtWebEngineCore {

RenderFrameObserverQt::RenderFrameObserverQt(content::RenderFrame* render_frame)
    : RenderFrameObserver(render_frame)
{
}

RenderFrameObserverQt::~RenderFrameObserverQt()
{
}

#if defined(ENABLE_PLUGINS)
void RenderFrameObserverQt::DidCreatePepperPlugin(content::RendererPpapiHost* host)
{
    host->GetPpapiHost()->AddHostFactoryFilter(
        scoped_ptr<ppapi::host::HostFactory>(
            new PepperRendererHostFactoryQt(host)));
}
#endif

} // namespace QtWebEngineCore
