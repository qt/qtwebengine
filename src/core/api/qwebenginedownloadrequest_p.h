// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEDOWNLOADREQUEST_P_H
#define QWEBENGINEDOWNLOADREQUEST_P_H

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

#include "qtwebenginecoreglobal.h"
#include "qwebenginedownloadrequest.h"
#include "profile_adapter_client.h"
#include <QString>
#include <QPointer>

namespace QtWebEngineCore {
class ProfileAdapter;
class WebContentsAdapterClient;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_PRIVATE_EXPORT QWebEngineDownloadRequestPrivate
{
public:
    QWebEngineDownloadRequestPrivate(QtWebEngineCore::ProfileAdapter *adapter);
    ~QWebEngineDownloadRequestPrivate();

    void update(const QtWebEngineCore::ProfileAdapterClient::DownloadItemInfo &info);
    void setFinished();

    bool downloadFinished = false;
    quint32 downloadId = -1;
    qint64 startTime;
    QWebEngineDownloadRequest::DownloadState downloadState =
            QWebEngineDownloadRequest::DownloadCancelled;
    QWebEngineDownloadRequest::SavePageFormat savePageFormat =
            QWebEngineDownloadRequest::MimeHtmlSaveFormat;
    QWebEngineDownloadRequest::DownloadInterruptReason interruptReason =
            QWebEngineDownloadRequest::NoReason;
    QString downloadPath;
    QUrl downloadUrl;
    QString mimeType;
    bool downloadPaused = false;
    QString suggestedFileName;
    QString downloadDirectory;
    QString downloadFileName;
    bool isCustomFileName = false;
    qint64 totalBytes = -1;
    qint64 receivedBytes = 0;
    bool isSavePageDownload = false;
    QWebEngineDownloadRequest *q_ptr;
    QPointer<QtWebEngineCore::ProfileAdapter> profileAdapter;
    QtWebEngineCore::WebContentsAdapterClient *adapterClient = nullptr;
    Q_DECLARE_PUBLIC(QWebEngineDownloadRequest)
};

QT_END_NAMESPACE

#endif // QWEBENGINEDOWNLOADREQUEST_P_H
