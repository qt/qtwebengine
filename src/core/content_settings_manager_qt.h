// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef CONTENT_SETTINGS_MANAGER_QT_H
#define CONTENT_SETTINGS_MANAGER_QT_H

#include "components/content_settings/core/common/content_settings_types.h"
#include "components/content_settings/common/content_settings_manager.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"

namespace QtWebEngineCore {

class ProfileIODataQt;

class ContentSettingsManagerQt : public content_settings::mojom::ContentSettingsManager
{
public:
    ContentSettingsManagerQt(base::WeakPtr<ProfileIODataQt> profile_io_data);
    static void
    Create(ProfileIODataQt *profile_io_data,
           mojo::PendingReceiver<content_settings::mojom::ContentSettingsManager> receiver);
    static void CreateAndBindOnIoThread(
            ProfileIODataQt *profile_io_data,
            mojo::PendingReceiver<content_settings::mojom::ContentSettingsManager> receiver);

    // mojom::ContentSettingsManager methods:
    void Clone(mojo::PendingReceiver<content_settings::mojom::ContentSettingsManager> receiver) override;
    void AllowStorageAccess(const blink::LocalFrameToken &frame_token, StorageType storage_type,
                            const url::Origin &origin, const net::SiteForCookies &site_for_cookies,
                            const url::Origin &top_frame_origin,
                            base::OnceCallback<void(bool)> callback) override;
    void OnContentBlocked(const blink::LocalFrameToken &frame_token,
                          ContentSettingsType type) override;

private:
    base::WeakPtr<ProfileIODataQt> m_profile_io_data;
};
}

#endif // CONTENT_SETTINGS_MANAGER_QT_H
