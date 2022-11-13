// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEPROFILE_P_H
#define QWEBENGINEPROFILE_P_H

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

#include <QMap>
#include <QPointer>
#include <QScopedPointer>
#include <QSharedPointer>

#include <functional>

namespace QtWebEngineCore {
class ProfileAdapter;
class WebEngineSettings;
}

QT_BEGIN_NAMESPACE

class QWebEngineNotification;
class QWebEngineProfile;
class QWebEngineScriptCollection;
class QWebEngineSettings;

class Q_WEBENGINECORE_PRIVATE_EXPORT QWebEngineProfilePrivate : public QtWebEngineCore::ProfileAdapterClient {
public:
    Q_DECLARE_PUBLIC(QWebEngineProfile)
    QWebEngineProfilePrivate(QtWebEngineCore::ProfileAdapter *profileAdapter);
    ~QWebEngineProfilePrivate();

    QtWebEngineCore::ProfileAdapter *profileAdapter() const;
    QWebEngineSettings *settings() const { return m_settings; }
    QtWebEngineCore::WebEngineSettings *coreSettings() const override;

    void downloadDestroyed(quint32 downloadId);

    void cleanDownloads();

    void downloadRequested(DownloadItemInfo &info) override;
    void downloadUpdated(const DownloadItemInfo &info) override;

    void showNotification(QSharedPointer<QtWebEngineCore::UserNotificationController> &) override;

    void addWebContentsAdapterClient(QtWebEngineCore::WebContentsAdapterClient *adapter) override;
    void removeWebContentsAdapterClient(QtWebEngineCore::WebContentsAdapterClient *adapter) override;

private:
    QWebEngineProfile *q_ptr;
    QWebEngineSettings *m_settings;
    QPointer<QtWebEngineCore::ProfileAdapter> m_profileAdapter;
    QScopedPointer<QWebEngineScriptCollection> m_scriptCollection;
    QMap<quint32, QPointer<QWebEngineDownloadRequest>> m_ongoingDownloads;
    std::function<void(std::unique_ptr<QWebEngineNotification>)> m_notificationPresenter;
};

QT_END_NAMESPACE

#endif // QWEBENGINEPROFILE_P_H
