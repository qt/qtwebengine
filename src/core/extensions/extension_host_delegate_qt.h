// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef EXTENSION_HOST_DELEGATE_QT_H
#define EXTENSION_HOST_DELEGATE_QT_H

#include "extensions/browser/extension_host_delegate.h"

namespace extensions {

class ExtensionHostDelegateQt : public ExtensionHostDelegate
{
public:
    ExtensionHostDelegateQt();

    // EtensionHostDelegate implementation.
    void OnExtensionHostCreated(content::WebContents *web_contents) override;
    void OnMainFrameCreatedForBackgroundPage(ExtensionHost *host) override;
    content::JavaScriptDialogManager *GetJavaScriptDialogManager() override;
    void CreateTab(std::unique_ptr<content::WebContents> web_contents,
                   const std::string &extension_id,
                   WindowOpenDisposition disposition,
                   const blink::mojom::WindowFeatures &features,
                   bool user_gesture) override;
    void ProcessMediaAccessRequest(content::WebContents *web_contents,
                                   const content::MediaStreamRequest &request,
                                   content::MediaResponseCallback callback,
                                   const Extension *extension) override;
    bool CheckMediaAccessPermission(content::RenderFrameHost *render_frame_host,
                                    const GURL &security_origin,
                                    blink::mojom::MediaStreamType type,
                                    const Extension *extension) override;
    content::PictureInPictureResult EnterPictureInPicture(content::WebContents *web_contents) override;
    void ExitPictureInPicture() override;
};

} // namespace extensions

#endif // EXTENSION_HOST_DELEGATE_QT_H
