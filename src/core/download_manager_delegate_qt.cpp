/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "download_manager_delegate_qt.h"

#include "content/public/browser/download_manager.h"
#include "content/public/browser/download_item.h"
#include "content/public/browser/save_page_type.h"
#include "content/public/browser/web_contents.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QStandardPaths>

#include "browser_context_adapter.h"
#include "browser_context_adapter_client.h"
#include "browser_context_qt.h"
#include "type_conversion.h"
#include "qtwebenginecoreglobal.h"

namespace QtWebEngineCore {

ASSERT_ENUMS_MATCH(content::DownloadItem::IN_PROGRESS, BrowserContextAdapterClient::DownloadInProgress)
ASSERT_ENUMS_MATCH(content::DownloadItem::COMPLETE, BrowserContextAdapterClient::DownloadCompleted)
ASSERT_ENUMS_MATCH(content::DownloadItem::CANCELLED, BrowserContextAdapterClient::DownloadCancelled)
ASSERT_ENUMS_MATCH(content::DownloadItem::INTERRUPTED, BrowserContextAdapterClient::DownloadInterrupted)

DownloadManagerDelegateQt::DownloadManagerDelegateQt(BrowserContextAdapter *contextAdapter)
    : m_contextAdapter(contextAdapter)
    , m_currentId(0)
{
    Q_ASSERT(m_contextAdapter);
}

DownloadManagerDelegateQt::~DownloadManagerDelegateQt()
{
}

void DownloadManagerDelegateQt::GetNextId(const content::DownloadIdCallback& callback)
{
    callback.Run(++m_currentId);
}

void DownloadManagerDelegateQt::cancelDownload(const content::DownloadTargetCallback& callback)
{
    callback.Run(base::FilePath(), content::DownloadItem::TARGET_DISPOSITION_PROMPT, content::DOWNLOAD_DANGER_TYPE_MAYBE_DANGEROUS_CONTENT, base::FilePath());
}

void DownloadManagerDelegateQt::cancelDownload(quint32 downloadId)
{
    content::DownloadManager* dlm = content::BrowserContext::GetDownloadManager(m_contextAdapter->browserContext());
    content::DownloadItem *download = dlm->GetDownload(downloadId);
    if (download)
        download->Cancel(/* user_cancel */ true);
}

bool DownloadManagerDelegateQt::DetermineDownloadTarget(content::DownloadItem* item,
                                                        const content::DownloadTargetCallback& callback)
{
    // Keep the forced file path if set, also as the temporary file, so the check for existence
    // will already return that the file exists. Forced file paths seem to be only used for
    // store downloads and other special downloads, so they might never end up here anyway.
    if (!item->GetForcedFilePath().empty()) {
        callback.Run(item->GetForcedFilePath(), content::DownloadItem::TARGET_DISPOSITION_PROMPT,
                     content::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS, item->GetForcedFilePath());
        return true;
    }

    std::string suggestedFilename = item->GetSuggestedFilename();

    if (suggestedFilename.empty())
        suggestedFilename = item->GetTargetFilePath().AsUTF8Unsafe();

    if (suggestedFilename.empty())
        suggestedFilename = item->GetURL().ExtractFileName();

    if (suggestedFilename.empty())
        suggestedFilename = "qwe_download";

    QDir defaultDownloadDirectory = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    QFileInfo suggestedFile(defaultDownloadDirectory.absoluteFilePath(QString::fromStdString(suggestedFilename)));
    QString suggestedFilePath = suggestedFile.absoluteFilePath();
    QString tmpFileBase = QString("%1%2%3").arg(suggestedFile.absolutePath()).arg(QDir::separator()).arg(suggestedFile.baseName());

    for (int i = 1; QFileInfo::exists(suggestedFilePath); ++i) {
        suggestedFilePath = QString("%1(%2).%3").arg(tmpFileBase).arg(i).arg(suggestedFile.completeSuffix());
        if (i >= 99) {
            suggestedFilePath = suggestedFile.absoluteFilePath();
            break;
        }
    }

    item->AddObserver(this);
    QList<BrowserContextAdapterClient*> clients = m_contextAdapter->clients();
    if (!clients.isEmpty()) {
        BrowserContextAdapterClient::DownloadItemInfo info = {
            item->GetId(),
            toQt(item->GetURL()),
            item->GetState(),
            item->GetTotalBytes(),
            item->GetReceivedBytes(),
            suggestedFilePath,
            false /* accepted */
        };

        Q_FOREACH (BrowserContextAdapterClient *client, clients) {
            client->downloadRequested(info);
            if (info.accepted)
                break;
        }

        suggestedFile.setFile(info.path);

        if (info.accepted && !suggestedFile.absoluteDir().mkpath(suggestedFile.absolutePath())) {
            qWarning("Creating download path failed, download cancelled: %s", suggestedFile.absolutePath().toUtf8().data());
            info.accepted = false;
        }

        if (!info.accepted) {
            cancelDownload(callback);
            return true;
        }

        base::FilePath filePathForCallback(toFilePathString(suggestedFile.absoluteFilePath()));
        callback.Run(filePathForCallback, content::DownloadItem::TARGET_DISPOSITION_OVERWRITE,
                     content::DOWNLOAD_DANGER_TYPE_MAYBE_DANGEROUS_CONTENT, filePathForCallback.AddExtension(toFilePathString("download")));
    } else
        cancelDownload(callback);

    return true;
}

void DownloadManagerDelegateQt::GetSaveDir(content::BrowserContext* browser_context,
                        base::FilePath* website_save_dir,
                        base::FilePath* download_save_dir,
                        bool* skip_dir_check)
{
    static base::FilePath::StringType save_dir = toFilePathString(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    *website_save_dir = base::FilePath(save_dir);
    *download_save_dir = base::FilePath(save_dir);
    *skip_dir_check = true;
}

void DownloadManagerDelegateQt::OnDownloadUpdated(content::DownloadItem *download)
{
    QList<BrowserContextAdapterClient*> clients = m_contextAdapter->clients();
    if (!clients.isEmpty()) {
        BrowserContextAdapterClient::DownloadItemInfo info = {
            download->GetId(),
            toQt(download->GetURL()),
            download->GetState(),
            download->GetTotalBytes(),
            download->GetReceivedBytes(),
            QString(),
            true /* accepted */
        };

        Q_FOREACH (BrowserContextAdapterClient *client, clients) {
            client->downloadUpdated(info);
        }
    }
}

void DownloadManagerDelegateQt::OnDownloadDestroyed(content::DownloadItem *download)
{
    download->RemoveObserver(this);
    download->Cancel(/* user_cancel */ false);
}

} // namespace QtWebEngineCore
