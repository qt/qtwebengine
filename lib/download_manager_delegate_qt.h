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

#ifndef DOWNLOAD_MANAGER_DELEGATE_QT
#define DOWNLOAD_MANAGER_DELEGATE_QT

#include "shared/shared_globals.h"

#include "content/public/browser/download_manager_delegate.h"
#include "content/public/browser/download_item.h"
#include "content/public/browser/save_page_type.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include <qglobal.h>
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QStringBuilder>

#include <QDebug>

class DownloadManagerDelegateQt : public content::DownloadManagerDelegate
{
public:
    DownloadManagerDelegateQt() : m_currentId(0) { }
    virtual ~DownloadManagerDelegateQt() { }
    virtual void Shutdown() { QT_NOT_YET_IMPLEMENTED}
    virtual void GetNextId(const content::DownloadIdCallback& callback) { callback.Run(++m_currentId); }

    // Tests if a file type should be opened automatically.
    virtual bool ShouldOpenFileBasedOnExtension(const base::FilePath& path) {
        QT_NOT_YET_IMPLEMENTED
        return false;
    }

    // Allows the delegate to delay completion of the download.  This function
    // will either return true (in which case the download may complete)
    // or will call the callback passed when the download is ready for
    // completion.  This routine may be called multiple times; once the callback
    // has been called or the function has returned true for a particular
    // download it should continue to return true for that download.
    virtual bool ShouldCompleteDownload(
            content::DownloadItem* item,
            const base::Closure& complete_callback) {
        QT_NOT_YET_IMPLEMENTED
        return true;
    }

    // Allows the delegate to override opening the download. If this function
    // returns false, the delegate needs to call callback when it's done
    // with the item, and is responsible for opening it.  This function is called
    // after the final rename, but before the download state is set to COMPLETED.
    virtual bool ShouldOpenDownload(content::DownloadItem* item,
                                    const content::DownloadOpenDelayedCallback& callback)
    {
        QT_NOT_YET_IMPLEMENTED
        return false;
    }

    virtual bool DetermineDownloadTarget(
            content::DownloadItem* item,
            const content::DownloadTargetCallback& callback) {
        QT_NOT_YET_IMPLEMENTED
        return true;
    }

    // Returns true if we need to generate a binary hash for downloads.
    virtual bool GenerateFileHash() {
        QT_NOT_YET_IMPLEMENTED
        return false;
    }

    // Asks the user for the path to save a page. The delegate calls the callback
    // to give the answer.
    virtual void ChooseSavePath(
            content::WebContents* web_contents,
            const base::FilePath& suggested_path,
            const base::FilePath::StringType& default_extension,
            bool can_save_as_complete,
            const content::SavePackagePathPickedCallback& callback) {
        QT_NOT_YET_IMPLEMENTED
    }

    // Opens the file associated with this download.
    virtual void OpenDownload(content::DownloadItem* download) {
        QT_NOT_YET_IMPLEMENTED
    }

    // Shows the download via the OS shell.
    virtual void ShowDownloadInShell(content::DownloadItem* download) {
        QT_NOT_YET_IMPLEMENTED
    }

    // Checks whether a downloaded file still exists.
    virtual void CheckForFileExistence(
            content::DownloadItem* download,
            const content::CheckForFileExistenceCallback& callback) {
        QT_NOT_YET_IMPLEMENTED
    }

    virtual void GetSaveDir(content::BrowserContext* browser_context,
                            base::FilePath* website_save_dir,
                            base::FilePath* download_save_dir,
                            bool* skip_dir_check) {
        QT_NOT_YET_IMPLEMENTED
    }

private:
    uint64 m_currentId;

    DISALLOW_COPY_AND_ASSIGN(DownloadManagerDelegateQt);
};

#endif //DOWNLOAD_MANAGER_DELEGATE_QT
