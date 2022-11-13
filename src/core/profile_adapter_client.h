// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef PROFILE_ADAPTER_CLIENT_H
#define PROFILE_ADAPTER_CLIENT_H

#include "api/qtwebenginecoreglobal_p.h"
#include <QSharedPointer>
#include <QString>
#include <QUrl>
#include <time.h>

namespace QtWebEngineCore {

class WebContentsAdapterClient;
class WebEngineSettings;
class UserNotificationController;

class Q_WEBENGINECORE_PRIVATE_EXPORT ProfileAdapterClient
{
public:
    // Keep in sync with content::DownloadItem::DownloadState
    enum DownloadState {
        // Download is actively progressing.
        DownloadInProgress = 0,
        // Download is completely finished.
        DownloadCompleted,
        // Download has been cancelled.
        DownloadCancelled,
        // This state indicates that the download has been interrupted.
        DownloadInterrupted
    };

    // Keep in sync with content::SavePageType
    enum SavePageFormat {
        UnknownSavePageFormat = -1,
        SingleHtmlSaveFormat,
        CompleteHtmlSaveFormat,
        MimeHtmlSaveFormat
    };

    // Keep in sync with content::DownloadInterruptReason
    enum DownloadInterruptReason {
        NoReason = 0,
        FileFailed = 1,
        FileAccessDenied = 2,
        FileNoSpace = 3,
        FileNameTooLong = 5,
        FileTooLarge = 6,
        FileVirusInfected = 7,
        FileTransientError = 10,
        FileBlocked = 11,
        FileSecurityCheckFailed = 12,
        FileTooShort = 13,
        FileHashMismatch = 14,
        NetworkFailed = 20,
        NetworkTimeout = 21,
        NetworkDisconnected = 22,
        NetworkServerDown = 23,
        NetworkInvalidRequest = 24,
        ServerFailed = 30,
        //ServerNoRange = 31,
        ServerBadContent = 33,
        ServerUnauthorized = 34,
        ServerCertProblem = 35,
        ServerForbidden = 36,
        ServerUnreachable = 37,
        UserCanceled = 40,
        //UserShutdown = 41,
        //Crash = 50
    };

    struct DownloadItemInfo {
        const quint32 id;
        const QUrl url;
        const int state;
        const qint64 totalBytes;
        const qint64 receivedBytes;
        const QString mimeType;

        QString path;
        int savePageFormat;
        bool accepted;
        bool paused;
        bool done;
        bool isSavePageDownload;
        int downloadInterruptReason;
        WebContentsAdapterClient *page;
        QString suggestedFileName;
        qint64 startTime;
    };

    virtual ~ProfileAdapterClient() { }

    virtual void downloadRequested(DownloadItemInfo &info) = 0;
    virtual void downloadUpdated(const DownloadItemInfo &info) = 0;
    virtual void showNotification(QSharedPointer<UserNotificationController> &) { }

    virtual void addWebContentsAdapterClient(WebContentsAdapterClient *adapter) = 0;
    virtual void removeWebContentsAdapterClient(WebContentsAdapterClient *adapter) = 0;
    virtual WebEngineSettings *coreSettings() const = 0;
    static QString downloadInterruptReasonToString(DownloadInterruptReason reason);
};

} // namespace

#endif // PROFILE_ADAPTER_CLIENT_H
