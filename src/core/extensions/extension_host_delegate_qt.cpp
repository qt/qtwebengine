// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

void ExtensionHostDelegateQt::OnMainFrameCreatedForBackgroundPage(ExtensionHost *host)
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
                                        const blink::mojom::WindowFeatures &features,
                                        bool user_gesture)
{
    Q_UNUSED(web_contents);
    Q_UNUSED(extension_id);
    Q_UNUSED(disposition);
    Q_UNUSED(features);
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

content::PictureInPictureResult ExtensionHostDelegateQt::EnterPictureInPicture(content::WebContents *web_contents)
{
    Q_UNUSED(web_contents);

    Q_UNREACHABLE();
    return content::PictureInPictureResult::kNotSupported;
}

void ExtensionHostDelegateQt::ExitPictureInPicture()
{
    Q_UNREACHABLE();
}

} // namespace extensions
