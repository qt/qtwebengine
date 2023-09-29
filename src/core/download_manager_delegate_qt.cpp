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

#include "profile_adapter_client.h"
#include "profile_adapter.h"
#include "profile_qt.h"
#include "qtwebenginecoreglobal.h"
#include "type_conversion.h"
#include "web_contents_delegate_qt.h"

namespace QtWebEngineCore {

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

void DownloadManagerDelegateQt::cancelDownload(content::DownloadTargetCallback callback)
{
    std::move(callback).Run(base::FilePath(),
                            download::DownloadItem::TARGET_DISPOSITION_PROMPT,
                            download::DownloadDangerType::DOWNLOAD_DANGER_TYPE_MAYBE_DANGEROUS_CONTENT,
                            download::DownloadItem::UNKNOWN,
                            base::FilePath(),
                            base::FilePath(),
                            std::string(),
                            download::DownloadInterruptReason::DOWNLOAD_INTERRUPT_REASON_USER_CANCELED);
}

bool DownloadManagerDelegateQt::cancelDownload(quint32 downloadId)
{
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
}

bool DownloadManagerDelegateQt::DetermineDownloadTarget(download::DownloadItem *item,
                                                        content::DownloadTargetCallback *callback)
{
    m_currentId = item->GetId();

    // Keep the forced file path if set, also as the temporary file, so the check for existence
    // will already return that the file exists. Forced file paths seem to be only used for
    // store downloads and other special downloads, so they might never end up here anyway.
    if (!item->GetForcedFilePath().empty()) {
        std::move(*callback).Run(item->GetForcedFilePath(), download::DownloadItem::TARGET_DISPOSITION_PROMPT,
                                 download::DownloadDangerType::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
                                 download::DownloadItem::VALIDATED,
                                 item->GetForcedFilePath(),
                                 item->GetFileNameToReportUser(),
                                 item->GetMimeType(),
                                 download::DownloadInterruptReason::DOWNLOAD_INTERRUPT_REASON_NONE);
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
        suggestedFilename = QStringLiteral("qwe_download");
        QMimeType mimeType = QMimeDatabase().mimeTypeForName(mimeTypeString);
        if (mimeType.isValid() && !mimeType.preferredSuffix().isEmpty())
            suggestedFilename += QStringLiteral(".") + mimeType.preferredSuffix();
    }

    QDir defaultDownloadDirectory(m_profileAdapter->downloadPath());

    if (suggestedFilePath.isEmpty())
        suggestedFilePath = m_profileAdapter->determineDownloadPath(defaultDownloadDirectory.absolutePath(), suggestedFilename, item->GetStartTime().ToTimeT());

    item->AddObserver(this);
    QList<ProfileAdapterClient*> clients = m_profileAdapter->clients();
    if (!clients.isEmpty()) {
        Q_ASSERT(m_currentId == item->GetId());
        ProfileAdapterClient::DownloadItemInfo info = {
            item->GetId(),
            toQt(item->GetURL()),
            item->GetState(),
            item->GetTotalBytes(),
            item->GetReceivedBytes(),
            mimeTypeString,
            suggestedFilePath,
            ProfileAdapterClient::UnknownSavePageFormat,
            acceptedByDefault,
            false /* paused */,
            false /* done */,
            isSavePageDownload,
            item->GetLastReason(),
            adapterClient,
            suggestedFilename,
            item->GetStartTime().ToTimeT()
        };

        for (ProfileAdapterClient *client : std::as_const(clients)) {
            client->downloadRequested(info);
            if (info.accepted)
                break;
        }

        QFileInfo suggestedFile(info.path);

        if (info.accepted && !suggestedFile.absoluteDir().mkpath(suggestedFile.absolutePath())) {
#if defined(Q_OS_WIN)
            // TODO: Remove this when https://bugreports.qt.io/browse/QTBUG-85997 is fixed.
            QDir suggestedDir = QDir(suggestedFile.absolutePath());
            if (!suggestedDir.isRoot() || !suggestedDir.exists()) {
#endif
            qWarning("Creating download path failed, download cancelled: %s", suggestedFile.absolutePath().toUtf8().data());
            info.accepted = false;
#if defined(Q_OS_WIN)
            }
#endif
        }

        if (!info.accepted) {
            cancelDownload(std::move(*callback));
            return true;
        }

        base::FilePath filePathForCallback(toFilePathString(suggestedFile.absoluteFilePath()));
        std::move(*callback).Run(filePathForCallback,
                                 download::DownloadItem::TARGET_DISPOSITION_OVERWRITE,
                                 download::DownloadDangerType::DOWNLOAD_DANGER_TYPE_MAYBE_DANGEROUS_CONTENT,
                                 download::DownloadItem::VALIDATED,
                                 filePathForCallback.AddExtension(toFilePathString("download")),
                                 base::FilePath(),
                                 item->GetMimeType(),
                                 download::DownloadInterruptReason::DOWNLOAD_INTERRUPT_REASON_NONE);
    } else
        cancelDownload(std::move(*callback));

    return true;
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
        suggestedFilePath = QFileInfo(toQt(suggested_path.AsUTF8Unsafe())).completeBaseName()
                + QStringLiteral(".mhtml");
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

    // Chromium doesn't increase download ID when saving page.
    ProfileAdapterClient::DownloadItemInfo info = {
        ++m_currentId,
        toQt(web_contents->GetURL()),
        download::DownloadItem::IN_PROGRESS,
        -1, /* totalBytes */
        0, /* receivedBytes */
        QStringLiteral("application/x-mimearchive"),
        suggestedFilePath,
        suggestedSaveFormat,
        acceptedByDefault,
        false, /* paused */
        false, /* done */
        true, /* isSavePageDownload */
        ProfileAdapterClient::NoReason,
        adapterClient,
        QFileInfo(suggestedFilePath).fileName(),
        QDateTime::currentMSecsSinceEpoch()
    };

    for (ProfileAdapterClient *client : std::as_const(clients)) {
        client->downloadRequested(info);
        if (info.accepted)
            break;
    }

    if (!info.accepted)
        return;

    std::move(callback).Run(toFilePath(info.path), static_cast<content::SavePageType>(info.savePageFormat),
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

        ProfileAdapterClient::DownloadItemInfo info = {
            download->GetId(),
            toQt(download->GetURL()),
            download->GetState(),
            download->GetTotalBytes(),
            download->GetReceivedBytes(),
            toQt(download->GetMimeType()),
            QString(),
            ProfileAdapterClient::UnknownSavePageFormat,
            true /* accepted */,
            download->IsPaused(),
            download->IsDone(),
            0 /* downloadType (unused) */,
            download->GetLastReason(),
            adapterClient,
            toQt(download->GetSuggestedFilename()),
            download->GetStartTime().ToTimeT()
        };

        for (ProfileAdapterClient *client : std::as_const(clients)) {
            client->downloadUpdated(info);
        }
    }
}

void DownloadManagerDelegateQt::OnDownloadDestroyed(download::DownloadItem *download)
{
    download->RemoveObserver(this);
    download->Cancel(/* user_cancel */ false);
}

} // namespace QtWebEngineCore
