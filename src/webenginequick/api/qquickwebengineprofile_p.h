// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINEPROFILE_P_H
#define QQUICKWEBENGINEPROFILE_P_H

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

#include "profile_adapter_client.h"
#include "qquickwebengineprofile.h"

#include <QtCore/qmap.h>
#include <QtCore/qpointer.h>
#include <QtCore/qsharedpointer.h>

namespace QtWebEngineCore {
class ProfileAdapter;
}

QT_BEGIN_NAMESPACE

class QQuickWebEngineDownloadRequest;
class QQuickWebEngineSettings;
class QQuickWebEngineScriptCollection;

class QQuickWebEngineProfilePrivate : public QtWebEngineCore::ProfileAdapterClient {
public:
    Q_DECLARE_PUBLIC(QQuickWebEngineProfile)
    QQuickWebEngineProfilePrivate(QtWebEngineCore::ProfileAdapter *profileAdapter);
    ~QQuickWebEngineProfilePrivate();
    void addWebContentsAdapterClient(QtWebEngineCore::WebContentsAdapterClient *adapter) override;
    void removeWebContentsAdapterClient(QtWebEngineCore::WebContentsAdapterClient *adapter) override;

    QtWebEngineCore::ProfileAdapter* profileAdapter() const;
    QQuickWebEngineSettings *settings() const;
    QtWebEngineCore::WebEngineSettings *coreSettings() const override;

    void cancelDownload(quint32 downloadId);
    void downloadDestroyed(quint32 downloadId);

    void cleanDownloads();

    void downloadRequested(DownloadItemInfo &info) override;
    void downloadUpdated(const DownloadItemInfo &info) override;

    void showNotification(QSharedPointer<QtWebEngineCore::UserNotificationController> &controller) override;

private:
    QQuickWebEngineProfile *q_ptr;
    QScopedPointer<QQuickWebEngineSettings> m_settings;
    QPointer<QtWebEngineCore::ProfileAdapter> m_profileAdapter;
    QMap<quint32, QPointer<QQuickWebEngineDownloadRequest> > m_ongoingDownloads;

    QScopedPointer<QQuickWebEngineScriptCollection> m_scriptCollection;
    QQuickWebEngineScriptCollection *getUserScripts();
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEPROFILE_P_H
