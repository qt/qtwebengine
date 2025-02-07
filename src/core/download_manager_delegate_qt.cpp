// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "download_manager_delegate_qt.h"

#include "content/public/browser/download_item_utils.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/save_page_type.h"
#include "content/public/browser/web_contents.h"
#include "net/base/net_string_util.h"
#include "net/http/http_content_disposition.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QTimer>

#include "profile_adapter_client.h"
#include "profile_adapter.h"
#include "profile_qt.h"
#include "qtwebenginecoreglobal.h"
#include "type_conversion.h"
#include "web_contents_delegate_qt.h"

using namespace Qt::StringLiterals;

namespace QtWebEngineCore {

void provideDownloadTarget(download::DownloadItem *item, download::DownloadTargetCallback *callback,
                           const base::FilePath &target)
{
    download::DownloadTargetInfo target_info;
    target_info.target_disposition = download::DownloadItem::TARGET_DISPOSITION_OVERWRITE;
    target_info.danger_type = download::DOWNLOAD_DANGER_TYPE_MAYBE_DANGEROUS_CONTENT;
    target_info.insecure_download_status = download::DownloadItem::VALIDATED;
    target_info.mime_type = item->GetMimeType();
    target_info.display_name = item->GetFileNameToReportUser();
    target_info.target_path = target;
    target_info.intermediate_path = target.AddExtensionASCII("download");
    std::move(*callback).Run(std::move(target_info));
}

DownloadManagerDelegateQt::DownloadManagerDelegateQt(ProfileAdapter *profileAdapter)
    : m_profileAdapter(profileAdapter)
    , m_currentId(0)
    , m_weakPtrFactory(this)
{
    Q_ASSERT(m_profileAdapter);
}

DownloadManagerDelegateQt::~DownloadManagerDelegateQt()
{
}

void DownloadManagerDelegateQt::GetNextId(content::DownloadIdCallback callback)
{
    std::move(callback).Run(m_currentId);
}

download::DownloadItem *DownloadManagerDelegateQt::findDownloadById(quint32 downloadId)
{
    content::DownloadManager *dlm = m_profileAdapter->profile()->GetDownloadManager();
    return dlm->GetDownload(downloadId);
}

void DownloadManagerDelegateQt::cancelDownload(download::DownloadTargetCallback callback)
{
    download::DownloadTargetInfo target_info;
    target_info.target_disposition = download::DownloadItem::TARGET_DISPOSITION_PROMPT;
    target_info.danger_type = download::DOWNLOAD_DANGER_TYPE_MAYBE_DANGEROUS_CONTENT;
    target_info.interrupt_reason = download::DOWNLOAD_INTERRUPT_REASON_USER_CANCELED;
    std::move(callback).Run(std::move(target_info));
}

bool DownloadManagerDelegateQt::cancelDownload(quint32 downloadId)
{
    m_pendingDownloads.erase(downloadId);
    m_pendingSaves.erase(downloadId);
    if (download::DownloadItem *download = findDownloadById(downloadId)) {
        download->Cancel(/* user_cancel */ true);
        return true;
    }
    return false;
}

void DownloadManagerDelegateQt::pauseDownload(quint32 downloadId)
{
    if (download::DownloadItem *download = findDownloadById(downloadId))
        download->Pause();
}

void DownloadManagerDelegateQt::resumeDownload(quint32 downloadId)
{
    if (download::DownloadItem *download = findDownloadById(downloadId))
        download->Resume(/* user_resume */ true);
}

void DownloadManagerDelegateQt::removeDownload(quint32 downloadId)
{
    if (download::DownloadItem *download = findDownloadById(downloadId))
        download->Remove();
    m_pendingDownloads.erase(downloadId);
    m_pendingSaves.erase(downloadId);
}

bool DownloadManagerDelegateQt::DetermineDownloadTarget(download::DownloadItem *item,
                                                        download::DownloadTargetCallback *callback)
{
    // The item came back for another round of target determination; this happens for example when
    // network error occurs. We already gave it a target path, let it use that, then it can report
    // the reason of its failure in OnDownloadUpdated().
    if (m_currentId >= item->GetId() && !item->GetTargetFilePath().empty()) {
        provideDownloadTarget(item, callback, item->GetTargetFilePath());
        return true;
    }

    m_currentId = item->GetId();

    // Keep the forced file path if set, also as the temporary file, so the check for existence
    // will already return that the file exists. Forced file paths seem to be only used for
    // store downloads and other special downloads, so they might never end up here anyway.
    if (!item->GetForcedFilePath().empty()) {
        download::DownloadTargetInfo target_info;
        target_info.target_disposition = download::DownloadItem::TARGET_DISPOSITION_PROMPT;
        target_info.danger_type = download::DOWNLOAD_DANGER_TYPE_MAYBE_DANGEROUS_CONTENT;
        target_info.insecure_download_status = download::DownloadItem::VALIDATED;
        target_info.mime_type = item->GetMimeType();
        target_info.display_name = item->GetFileNameToReportUser();
        target_info.target_path = item->GetForcedFilePath();
        target_info.intermediate_path = item->GetForcedFilePath();
        std::move(*callback).Run(std::move(target_info));
        return true;
    }

    bool acceptedByDefault = false;
    QString suggestedFilePath;
    QString suggestedFilename;
    bool isSavePageDownload = false;
    WebContentsAdapterClient *adapterClient = nullptr;
    if (content::WebContents *webContents = content::DownloadItemUtils::GetWebContents(item)) {
        WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(webContents->GetDelegate());
        adapterClient = contentsDelegate->adapterClient();
        if (SavePageInfo *spi = contentsDelegate->savePageInfo()) {
            // We end up here when saving non text-based files (MHTML, PDF or images)
            suggestedFilePath = spi->requestedFilePath;
            const QFileInfo fileInfo(suggestedFilePath);
            if (fileInfo.isRelative()) {
                const QDir downloadDir(m_profileAdapter->downloadPath());
                suggestedFilePath = downloadDir.absoluteFilePath(suggestedFilePath);
            }
            suggestedFilename = fileInfo.fileName();

            if (!suggestedFilePath.isEmpty() && !suggestedFilename.isEmpty())
                acceptedByDefault = true;
            isSavePageDownload = true;

            // Clear the delegate's SavePageInfo. It's only valid for the page currently being saved.
            contentsDelegate->setSavePageInfo(nullptr);
        }
    }

    QString mimeTypeString = toQt(item->GetMimeType());

    if (suggestedFilename.isEmpty())
        suggestedFilename = toQt(item->GetSuggestedFilename());

    if (suggestedFilename.isEmpty())
        suggestedFilename = toQt(net::HttpContentDisposition(item->GetContentDisposition(), net::kCharsetLatin1).filename());

    if (suggestedFilename.isEmpty())
        suggestedFilename = toQt(item->GetTargetFilePath().AsUTF8Unsafe());

    if (suggestedFilename.isEmpty()) {
        GURL itemUrl = item->GetURL();
        if (!itemUrl.SchemeIs("about") && !itemUrl.SchemeIs("data"))
            suggestedFilename = QUrl::fromPercentEncoding(toQByteArray(itemUrl.ExtractFileName()));
    }

    if (suggestedFilename.isEmpty()) {
        suggestedFilename += "qwe_download"_L1;
        QMimeType mimeType = QMimeDatabase().mimeTypeForName(mimeTypeString);
        if (mimeType.isValid() && !mimeType.preferredSuffix().isEmpty())
            suggestedFilename += u'.' + mimeType.preferredSuffix();
    }

    QDir defaultDownloadDirectory(m_profileAdapter->downloadPath());

    if (suggestedFilePath.isEmpty())
        suggestedFilePath = m_profileAdapter->determineDownloadPath(defaultDownloadDirectory.absolutePath(), suggestedFilename, item->GetStartTime().ToTimeT());

    item->AddObserver(this);
    QList<ProfileAdapterClient*> clients = m_profileAdapter->clients();
    if (!clients.isEmpty()) {
        ProfileAdapterClient::DownloadItemInfo info = {};
        info.id = item->GetId();
        info.url = toQt(item->GetURL());
        info.state = item->GetState();
        info.totalBytes = item->GetTotalBytes();
        info.receivedBytes = item->GetReceivedBytes();
        info.mimeType = std::move(mimeTypeString);
        info.path = std::move(suggestedFilePath);
        info.savePageFormat = ProfileAdapterClient::UnknownSavePageFormat;
        info.accepted = acceptedByDefault;
        info.paused = false;
        info.done = false;
        info.isSavePageDownload = isSavePageDownload;
        info.useDownloadTargetCallback = true;
        info.downloadInterruptReason = item->GetLastReason();
        info.page = adapterClient;
        info.suggestedFileName = std::move(suggestedFilename);
        info.startTime = item->GetStartTime().ToTimeT();

        m_pendingDownloads.emplace(m_currentId, std::move(*callback));
        QTimer::singleShot(0, m_profileAdapter,
                           [client = clients[0], info]() { client->downloadRequested(info); });
    } else
        cancelDownload(std::move(*callback));

    return true;
}

void DownloadManagerDelegateQt::downloadTargetDetermined(quint32 downloadId, bool accepted,
                                                         const QString &path)
{
    if (!m_pendingDownloads.contains(downloadId))
        return;
    auto callback = std::move(m_pendingDownloads.find(downloadId)->second);
    m_pendingDownloads.erase(downloadId);

    download::DownloadItem *item = findDownloadById(downloadId);
    if (!accepted || !item) {
        cancelDownload(std::move(callback));
        return;
    }

    QFileInfo suggestedFile(path);
    if (!suggestedFile.absoluteDir().mkpath(suggestedFile.absolutePath())) {
        qWarning("Creating download path failed, download cancelled: %ls",
                 qUtf16Printable(suggestedFile.absolutePath()));
        cancelDownload(std::move(callback));
        return;
    }
    base::FilePath targetPath(toFilePathString(suggestedFile.absoluteFilePath()));
    provideDownloadTarget(item, &callback, targetPath);
}

void DownloadManagerDelegateQt::GetSaveDir(content::BrowserContext* browser_context,
                                           base::FilePath* website_save_dir,
                                           base::FilePath* download_save_dir)
{
    static base::FilePath::StringType save_dir = toFilePathString(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    *website_save_dir = base::FilePath(save_dir);
    *download_save_dir = base::FilePath(save_dir);
}

void DownloadManagerDelegateQt::ChooseSavePath(content::WebContents *web_contents,
                        const base::FilePath &suggested_path,
                        const base::FilePath::StringType &default_extension,
                        bool can_save_as_complete,
                        content::SavePackagePathPickedCallback callback)
{
    Q_UNUSED(default_extension);
    Q_UNUSED(can_save_as_complete);

    QList<ProfileAdapterClient*> clients = m_profileAdapter->clients();
    if (clients.isEmpty())
        return;

    bool acceptedByDefault = false;
    QString suggestedFilePath;
    ProfileAdapterClient::SavePageFormat suggestedSaveFormat = ProfileAdapterClient::UnknownSavePageFormat;
    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(
            web_contents->GetDelegate());
    if (SavePageInfo *spi = contentsDelegate->savePageInfo()) {
        suggestedFilePath = spi->requestedFilePath;
        suggestedSaveFormat = static_cast<ProfileAdapterClient::SavePageFormat>(spi->requestedFormat);
        // Clear the delegate's SavePageInfo. It's only valid for the page currently being saved.
        contentsDelegate->setSavePageInfo(nullptr);
    }

    if (suggestedFilePath.isEmpty()) {
        suggestedFilePath +=
                QFileInfo(toQt(suggested_path.AsUTF8Unsafe())).completeBaseName() + ".mhtml"_L1;
    } else {
        acceptedByDefault = true;
    }
    if (QFileInfo(suggestedFilePath).isRelative()) {
        const QDir downloadDir(m_profileAdapter->downloadPath());
        suggestedFilePath = downloadDir.absoluteFilePath(suggestedFilePath);
    }

    if (suggestedSaveFormat == ProfileAdapterClient::UnknownSavePageFormat)
        suggestedSaveFormat = ProfileAdapterClient::MimeHtmlSaveFormat;

    WebContentsAdapterClient *adapterClient = nullptr;
    if (web_contents)
        adapterClient = static_cast<WebContentsDelegateQt *>(web_contents->GetDelegate())->adapterClient();

    ProfileAdapterClient::DownloadItemInfo info = {};
    // Chromium doesn't increase download ID when saving page.
    info.id = ++m_currentId;
    info.url = toQt(web_contents->GetURL());
    info.state = download::DownloadItem::IN_PROGRESS;
    info.totalBytes = -1;
    info.receivedBytes = 0;
    info.mimeType = u"application/x-mimearchive"_s;
    info.path = suggestedFilePath;
    info.savePageFormat = suggestedSaveFormat;
    info.accepted = acceptedByDefault;
    info.paused = false;
    info.done = false;
    info.isSavePageDownload = true;
    info.useDownloadTargetCallback = false;
    info.downloadInterruptReason = ProfileAdapterClient::NoReason;
    info.page = adapterClient;
    info.suggestedFileName = QFileInfo(suggestedFilePath).fileName();
    info.startTime = QDateTime::currentMSecsSinceEpoch();

    m_pendingSaves.emplace(m_currentId, std::move(callback));
    QTimer::singleShot(0, m_profileAdapter,
                       [client = clients[0], info]() { client->downloadRequested(info); });
}

void DownloadManagerDelegateQt::savePathDetermined(quint32 downloadId, bool accepted,
                                                   const QString &path, int format)
{
    if (!accepted) {
        m_pendingSaves.erase(downloadId);
        return;
    }

    if (!m_pendingSaves.contains(downloadId))
        return;
    auto callback = std::move(m_pendingSaves.find(downloadId)->second);
    m_pendingSaves.erase(downloadId);

    content::SavePackagePathPickedParams params;
    params.file_path = toFilePath(path);
    params.save_type = static_cast<content::SavePageType>(format);
    std::move(callback).Run(std::move(params),
                            base::BindOnce(&DownloadManagerDelegateQt::savePackageDownloadCreated,
                                           m_weakPtrFactory.GetWeakPtr()));
}

void DownloadManagerDelegateQt::savePackageDownloadCreated(download::DownloadItem *item)
{
    OnDownloadUpdated(item);
    item->AddObserver(this);
}

void DownloadManagerDelegateQt::OnDownloadUpdated(download::DownloadItem *download)
{
    QList<ProfileAdapterClient*> clients = m_profileAdapter->clients();
    if (!clients.isEmpty()) {
        WebContentsAdapterClient *adapterClient = nullptr;
        content::WebContents *webContents = content::DownloadItemUtils::GetWebContents(download);
        if (webContents)
            adapterClient = static_cast<WebContentsDelegateQt *>(webContents->GetDelegate())->adapterClient();

        ProfileAdapterClient::DownloadItemInfo info = {};
        // Chromium doesn't increase download ID when saving page.
        info.id = download->GetId();
        info.url = toQt(download->GetURL());
        info.state = download->GetState();
        info.totalBytes = download->GetTotalBytes();
        info.receivedBytes = download->GetReceivedBytes();
        info.mimeType = toQt(download->GetMimeType());
        info.path = QString();
        info.savePageFormat = ProfileAdapterClient::UnknownSavePageFormat;
        info.accepted = true;
        info.paused = download->IsPaused();
        info.done = download->IsDone();
        info.isSavePageDownload = false; // unused
        info.useDownloadTargetCallback = false; // unused
        info.downloadInterruptReason = download->GetLastReason();
        info.page = adapterClient;
        info.suggestedFileName = toQt(download->GetSuggestedFilename());
        info.startTime = download->GetStartTime().ToTimeT();

        QTimer::singleShot(0, m_profileAdapter,
                           [client = clients[0], info]() { client->downloadUpdated(info); });
    }
}

void DownloadManagerDelegateQt::OnDownloadDestroyed(download::DownloadItem *download)
{
    download->RemoveObserver(this);
    download->Cancel(/* user_cancel */ false);
}

} // namespace QtWebEngineCore
