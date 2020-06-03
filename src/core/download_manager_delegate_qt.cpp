/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "download_manager_delegate_qt.h"

#include "base/files/file_util.h"
#include "base/time/time_to_iso8601.h"
#include "content/public/browser/download_item_utils.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/save_page_type.h"
#include "content/public/browser/web_contents.h"
#include "net/base/net_string_util.h"
#include "net/http/http_content_disposition.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
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
    , m_nextDownloadIsUserRequested(false)
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
    content::DownloadManager* dlm = content::BrowserContext::GetDownloadManager(m_profileAdapter->profile());
    return dlm->GetDownload(downloadId);
}

void DownloadManagerDelegateQt::cancelDownload(content::DownloadTargetCallback callback)
{
    std::move(callback).Run(base::FilePath(),
                            download::DownloadItem::TARGET_DISPOSITION_PROMPT,
                            download::DownloadDangerType::DOWNLOAD_DANGER_TYPE_MAYBE_DANGEROUS_CONTENT,
                            download::DownloadItem::UNKNOWN,
                            base::FilePath(),
                            download::DownloadInterruptReason::DOWNLOAD_INTERRUPT_REASON_USER_CANCELED);
}

void DownloadManagerDelegateQt::cancelDownload(quint32 downloadId)
{
    if (download::DownloadItem *download = findDownloadById(downloadId))
        download->Cancel(/* user_cancel */ true);
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
                                 download::DownloadInterruptReason::DOWNLOAD_INTERRUPT_REASON_NONE);
        return true;
    }

    QString suggestedFilename = toQt(item->GetSuggestedFilename());
    QString mimeTypeString = toQt(item->GetMimeType());

    int downloadType = 0;
    if (m_nextDownloadIsUserRequested) {
        downloadType = ProfileAdapterClient::UserRequested;
        m_nextDownloadIsUserRequested = false;
    } else {
        bool isAttachment = net::HttpContentDisposition(item->GetContentDisposition(), std::string()).is_attachment();
        if (isAttachment)
            downloadType = ProfileAdapterClient::Attachment;
        else
            downloadType = ProfileAdapterClient::DownloadAttribute;
    }

    if (suggestedFilename.isEmpty())
        suggestedFilename = toQt(net::HttpContentDisposition(item->GetContentDisposition(), net::kCharsetLatin1).filename());

    if (suggestedFilename.isEmpty())
        suggestedFilename = toQt(item->GetTargetFilePath().AsUTF8Unsafe());

    if (suggestedFilename.isEmpty())
        suggestedFilename = QUrl::fromPercentEncoding(toQByteArray(item->GetURL().ExtractFileName()));

    if (suggestedFilename.isEmpty()) {
        suggestedFilename = QStringLiteral("qwe_download");
        QMimeType mimeType = QMimeDatabase().mimeTypeForName(mimeTypeString);
        if (mimeType.isValid() && !mimeType.preferredSuffix().isEmpty())
            suggestedFilename += QStringLiteral(".") + mimeType.preferredSuffix();
    }

    QDir defaultDownloadDirectory(m_profileAdapter->downloadPath());

    QString suggestedFilePath = m_profileAdapter->determineDownloadPath(defaultDownloadDirectory.absolutePath(), suggestedFilename, item->GetStartTime().ToTimeT());

    item->AddObserver(this);
    QList<ProfileAdapterClient*> clients = m_profileAdapter->clients();
    if (!clients.isEmpty()) {
        content::WebContents *webContents = content::DownloadItemUtils::GetWebContents(item);
        WebContentsAdapterClient *adapterClient = nullptr;
        if (webContents)
            adapterClient = static_cast<WebContentsDelegateQt *>(webContents->GetDelegate())->adapterClient();

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
            false /* accepted */,
            false /* paused */,
            false /* done */,
            downloadType,
            item->GetLastReason(),
            adapterClient,
            suggestedFilename,
            item->GetStartTime().ToTimeT()
        };

        for (ProfileAdapterClient *client : qAsConst(clients)) {
            client->downloadRequested(info);
            if (info.accepted)
                break;
        }

        QFileInfo suggestedFile(info.path);

        if (info.accepted && !suggestedFile.absoluteDir().mkpath(suggestedFile.absolutePath())) {
            qWarning("Creating download path failed, download cancelled: %s", suggestedFile.absolutePath().toUtf8().data());
            info.accepted = false;
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

    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(
            web_contents->GetDelegate());
    const SavePageInfo &spi = contentsDelegate->savePageInfo();

    bool acceptedByDefault = false;
    QString suggestedFilePath = spi.requestedFilePath;
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

    ProfileAdapterClient::SavePageFormat suggestedSaveFormat
            = static_cast<ProfileAdapterClient::SavePageFormat>(spi.requestedFormat);
    if (suggestedSaveFormat == ProfileAdapterClient::UnknownSavePageFormat)
        suggestedSaveFormat = ProfileAdapterClient::MimeHtmlSaveFormat;

    // Clear the delegate's SavePageInfo. It's only valid for the page currently being saved.
    contentsDelegate->setSavePageInfo(SavePageInfo());

    WebContentsAdapterClient *adapterClient = nullptr;
    if (web_contents)
        adapterClient = static_cast<WebContentsDelegateQt *>(web_contents->GetDelegate())->adapterClient();

    // Chromium doesn't increase download ID when saving page.
    ProfileAdapterClient::DownloadItemInfo info = {
        ++m_currentId,
        toQt(web_contents->GetURL()),
        download::DownloadItem::IN_PROGRESS,
        0, /* totalBytes */
        0, /* receivedBytes */
        QStringLiteral("application/x-mimearchive"),
        suggestedFilePath,
        suggestedSaveFormat,
        acceptedByDefault,
        false, /* paused */
        false, /* done */
        ProfileAdapterClient::SavePage,
        ProfileAdapterClient::NoReason,
        adapterClient,
        QFileInfo(suggestedFilePath).fileName(),
        QDateTime::currentMSecsSinceEpoch()
    };

    for (ProfileAdapterClient *client : qAsConst(clients)) {
        client->downloadRequested(info);
        if (info.accepted)
            break;
    }

    if (!info.accepted)
        return;

    std::move(callback).Run(toFilePath(info.path), static_cast<content::SavePageType>(info.savePageFormat),
                            base::Bind(&DownloadManagerDelegateQt::savePackageDownloadCreated,
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

        for (ProfileAdapterClient *client : qAsConst(clients)) {
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
