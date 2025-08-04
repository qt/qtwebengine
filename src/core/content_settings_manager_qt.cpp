// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "content_settings_manager_qt.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"

#include "profile_io_data_qt.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

ContentSettingsManagerQt::ContentSettingsManagerQt(base::WeakPtr<ProfileIODataQt> profile_io_data)
    : m_profile_io_data(profile_io_data)
{
}

void ContentSettingsManagerQt::Create(
        ProfileIODataQt *profile_io_data,
        mojo::PendingReceiver<content_settings::mojom::ContentSettingsManager> receiver)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    content::GetIOThreadTaskRunner({})->PostTask(
            FROM_HERE,
            base::BindOnce(&ContentSettingsManagerQt::CreateAndBindOnIoThread, profile_io_data,
                           std::move(receiver)));
}

void ContentSettingsManagerQt::CreateAndBindOnIoThread(
        ProfileIODataQt *profile_io_data,
        mojo::PendingReceiver<content_settings::mojom::ContentSettingsManager> receiver)
{
    auto wrapper =
            base::WrapUnique(new ContentSettingsManagerQt(profile_io_data->getWeakPtrOnIOThread()));
    mojo::MakeSelfOwnedReceiver(std::move(wrapper), std::move(receiver));
}

void ContentSettingsManagerQt::Clone(
        mojo::PendingReceiver<content_settings::mojom::ContentSettingsManager> receiver)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    mojo::MakeSelfOwnedReceiver(base::WrapUnique(new ContentSettingsManagerQt(
                                        this->m_profile_io_data->getWeakPtrOnIOThread())),
                                std::move(receiver));
}

void ContentSettingsManagerQt::AllowStorageAccess(const blink::LocalFrameToken &frame_token,
                                                  StorageType storage_type,
                                                  const url::Origin &origin,
                                                  const net::SiteForCookies &site_for_cookies,
                                                  const url::Origin &top_frame_origin,
                                                  base::OnceCallback<void(bool)> callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    Q_UNUSED(frame_token);
    Q_UNUSED(storage_type);
    Q_UNUSED(site_for_cookies);

    bool allowed = m_profile_io_data->canGetCookies(toQt(top_frame_origin), toQt(origin));
    std::move(callback).Run(allowed);
}

void ContentSettingsManagerQt::OnContentBlocked(const blink::LocalFrameToken &frame_token,
                                                ContentSettingsType type) {}
}
