// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "extension_host_delegate_qt.h"

#include "desktop_media_controller.h"
#include "desktop_media_controller_p.h"
#include "extension_web_contents_observer_qt.h"
#include "media_capture_devices_dispatcher.h"
#include "extension_system_qt.h"
#include "web_contents_view_qt.h"

#include "base/functional/callback.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "extensions/browser/extension_host.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom-shared.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom.h"

using namespace QtWebEngineCore;

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

static void processMediaAccessRequest(content::WebContents *webContents,
                                      const content::MediaStreamRequest &request,
                                      content::MediaResponseCallback callback,
                                      content::DesktopMediaID id)
{
    MediaCaptureDevicesDispatcher::GetInstance()->processMediaAccessRequest(
            webContents, request, std::move(callback), id);
}

void ExtensionHostDelegateQt::ProcessMediaAccessRequest(content::WebContents *web_contents,
                                                        const content::MediaStreamRequest &request,
                                                        content::MediaResponseCallback callback,
                                                        const Extension *extension)
{
    Q_UNUSED(extension);
    base::OnceCallback<void(content::DesktopMediaID)> cb = base::BindOnce(
            &processMediaAccessRequest, web_contents, std::move(request), std::move(callback));

    // ownership is taken by the request
    auto *controller = new DesktopMediaController(new DesktopMediaControllerPrivate(std::move(cb)));
    base::WeakPtr<content::WebContents> webContents = web_contents->GetWeakPtr();
    QObject::connect(controller, &DesktopMediaController::mediaListsInitialized, [controller, webContents]() {
        if (webContents) {
            auto *client = WebContentsViewQt::from(static_cast<content::WebContentsImpl *>(webContents.get())->GetView())->client();
            client->desktopMediaRequested(controller);
        } else {
            controller->deleteLater();
        }
    });
}

bool ExtensionHostDelegateQt::CheckMediaAccessPermission(
        content::RenderFrameHost *render_frame_host, const url::Origin &security_origin,
        blink::mojom::MediaStreamType type, const Extension *extension)
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
