// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DOWNLOAD_MANAGER_DELEGATE_QT_H
#define DOWNLOAD_MANAGER_DELEGATE_QT_H

#include "content/public/browser/download_manager_delegate.h"
#include <base/memory/weak_ptr.h>

#include <QtGlobal>

namespace base {
class FilePath;
}

namespace content {
class BrowserContext;
class WebContents;
}

namespace download {
class DownloadItem;
}

namespace QtWebEngineCore {
class ProfileAdapter;

class DownloadManagerDelegateQt
        : public content::DownloadManagerDelegate
        , public download::DownloadItem::Observer
{
public:
    DownloadManagerDelegateQt(ProfileAdapter *profileAdapter);
    ~DownloadManagerDelegateQt();
    void GetNextId(content::DownloadIdCallback callback) override;

    bool DetermineDownloadTarget(download::DownloadItem *item,
                                 content::DownloadTargetCallback *callback) override;

    void GetSaveDir(content::BrowserContext* browser_context,
                    base::FilePath* website_save_dir,
                    base::FilePath* download_save_dir) override;
    void ChooseSavePath(content::WebContents *web_contents,
                        const base::FilePath &suggested_path,
                        const base::FilePath::StringType &default_extension,
                        bool can_save_as_complete,
                        content::SavePackagePathPickedCallback callback) override;

    bool cancelDownload(quint32 downloadId);
    void pauseDownload(quint32 downloadId);
    void resumeDownload(quint32 downloadId);
    void removeDownload(quint32 downloadId);

    // Inherited from content::DownloadItem::Observer
    void OnDownloadUpdated(download::DownloadItem *download) override;
    void OnDownloadDestroyed(download::DownloadItem *download) override;

private:
    void cancelDownload(content::DownloadTargetCallback callback);
    download::DownloadItem *findDownloadById(quint32 downloadId);
    void savePackageDownloadCreated(download::DownloadItem *download);
    ProfileAdapter *m_profileAdapter;

    uint32_t m_currentId;
    base::WeakPtrFactory<DownloadManagerDelegateQt> m_weakPtrFactory;
};

} // namespace QtWebEngineCore

#endif // DOWNLOAD_MANAGER_DELEGATE_QT_H
