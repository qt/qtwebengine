/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "download_manager_delegate_qt.h"

#include "content/public/browser/download_item.h"
#include "content/public/browser/save_page_type.h"
#include "content/public/browser/web_contents.h"
#include "shared/shared_globals.h"

#include <QStandardPaths>

DownloadManagerDelegateQt::DownloadManagerDelegateQt() : m_currentId(0)
{
}

void DownloadManagerDelegateQt::Shutdown()
{
    QT_NOT_YET_IMPLEMENTED
}

void DownloadManagerDelegateQt::GetNextId(const content::DownloadIdCallback& callback)
{
    QT_NOT_YET_IMPLEMENTED
    callback.Run(++m_currentId);
}

bool DownloadManagerDelegateQt::ShouldOpenFileBasedOnExtension(const base::FilePath& path)
{
    QT_NOT_YET_IMPLEMENTED
    return false;
}

bool DownloadManagerDelegateQt::ShouldCompleteDownload(content::DownloadItem* item,
                                                       const base::Closure& complete_callback)
{
    QT_NOT_YET_IMPLEMENTED
    return true;
}

bool DownloadManagerDelegateQt::ShouldOpenDownload(content::DownloadItem* item,
                                                   const content::DownloadOpenDelayedCallback& callback)
{
    QT_NOT_YET_IMPLEMENTED
    return false;
}

bool DownloadManagerDelegateQt::DetermineDownloadTarget(content::DownloadItem* item,
                                                        const content::DownloadTargetCallback& callback)
{
    QT_NOT_YET_IMPLEMENTED
    base::FilePath downloadFilePath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation).toStdString());
    std::string fileName = item->GetURL().ExtractFileName();
    std::string tmpFileName = fileName + ".tmp";

    callback.Run(downloadFilePath.Append(fileName),
                 content::DownloadItem::TARGET_DISPOSITION_OVERWRITE, content::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
                 downloadFilePath.Append(tmpFileName));

    return true;
}

bool DownloadManagerDelegateQt::GenerateFileHash()
{
    QT_NOT_YET_IMPLEMENTED
    return false;
}

void DownloadManagerDelegateQt::ChooseSavePath(
        content::WebContents* web_contents,
        const base::FilePath& suggested_path,
        const base::FilePath::StringType& default_extension,
        bool can_save_as_complete,
        const content::SavePackagePathPickedCallback& callback)
{
    QT_NOT_YET_IMPLEMENTED
}

void DownloadManagerDelegateQt::OpenDownload(content::DownloadItem* download)
{
    QT_NOT_YET_IMPLEMENTED
}

void DownloadManagerDelegateQt::ShowDownloadInShell(content::DownloadItem* download)
{
    QT_NOT_YET_IMPLEMENTED
}

void DownloadManagerDelegateQt::CheckForFileExistence(
        content::DownloadItem* download,
        const content::CheckForFileExistenceCallback& callback)
{
    QT_NOT_YET_IMPLEMENTED
}

void DownloadManagerDelegateQt::GetSaveDir(content::BrowserContext* browser_context,
                        base::FilePath* website_save_dir,
                        base::FilePath* download_save_dir,
                        bool* skip_dir_check)
{
    QT_NOT_YET_IMPLEMENTED
    *website_save_dir = base::FilePath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation).toStdString());
    *download_save_dir = base::FilePath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation).toStdString());
    *skip_dir_check = true;
}

