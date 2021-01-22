/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include "extension_host_delegate_qt.h"
#include "extension_web_contents_observer_qt.h"
#include "media_capture_devices_dispatcher.h"
#include "extension_system_qt.h"

#include "extensions/browser/extension_host.h"

namespace extensions {

ExtensionHostDelegateQt::ExtensionHostDelegateQt()
{
}

void ExtensionHostDelegateQt::OnExtensionHostCreated(content::WebContents *web_contents)
{
    extensions::ExtensionWebContentsObserverQt::CreateForWebContents(web_contents);
}

void ExtensionHostDelegateQt::OnRenderViewCreatedForBackgroundPage(ExtensionHost *host)
{
    Q_UNUSED(host);
}

content::JavaScriptDialogManager *ExtensionHostDelegateQt::GetJavaScriptDialogManager()
{
    Q_UNREACHABLE();
    return nullptr;
}

void ExtensionHostDelegateQt::CreateTab(std::unique_ptr<content::WebContents> web_contents,
                                        const std::string &extension_id,
                                        WindowOpenDisposition disposition,
                                        const gfx::Rect &initial_rect,
                                        bool user_gesture)
{
    Q_UNUSED(web_contents);
    Q_UNUSED(extension_id);
    Q_UNUSED(disposition);
    Q_UNUSED(initial_rect);
    Q_UNUSED(user_gesture);

    Q_UNREACHABLE();
}

void ExtensionHostDelegateQt::ProcessMediaAccessRequest(content::WebContents *web_contents,
                                                        const content::MediaStreamRequest &request,
                                                        content::MediaResponseCallback callback,
                                                        const Extension *extension)
{
    Q_UNUSED(extension);

    QtWebEngineCore::MediaCaptureDevicesDispatcher::GetInstance()->processMediaAccessRequest(web_contents, request, std::move(callback));
}

bool ExtensionHostDelegateQt::CheckMediaAccessPermission(content::RenderFrameHost *render_frame_host,
                                                         const GURL &security_origin,
                                                         blink::mojom::MediaStreamType type,
                                                         const Extension *extension)
{
    Q_UNUSED(render_frame_host);
    Q_UNUSED(security_origin);
    Q_UNUSED(type);
    Q_UNUSED(extension);

    Q_UNREACHABLE();
    return false;
}

content::PictureInPictureResult ExtensionHostDelegateQt::EnterPictureInPicture(content::WebContents *web_contents,
                                                                               const viz::SurfaceId &surface_id,
                                                                               const gfx::Size &natural_size)
{
    Q_UNUSED(web_contents);
    Q_UNUSED(surface_id);
    Q_UNUSED(natural_size);

    Q_UNREACHABLE();
    return content::PictureInPictureResult::kNotSupported;
}

void ExtensionHostDelegateQt::ExitPictureInPicture()
{
    Q_UNREACHABLE();
}

} // namespace extensions
