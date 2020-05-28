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

#ifndef PROFILE_ADAPTER_H
#define PROFILE_ADAPTER_H

#include "qtwebenginecoreglobal_p.h"

#include <QEnableSharedFromThis>
#include <QList>
#include <QPointer>
#include <QScopedPointer>
#include <QString>
#include <QVector>

#include "api/qwebengineclientcertificatestore.h"
#include "api/qwebenginecookiestore.h"
#include "api/qwebengineurlrequestinterceptor.h"
#include "api/qwebengineurlschemehandler.h"
#include "net/qrc_url_scheme_handler.h"

QT_FORWARD_DECLARE_CLASS(QObject)

namespace QtWebEngineCore {

class UserNotificationController;
class DownloadManagerDelegateQt;
class ProfileAdapterClient;
class ProfileQt;
class UserResourceControllerHost;
class VisitedLinksManagerQt;
class WebContentsAdapterClient;

class Q_WEBENGINECORE_PRIVATE_EXPORT ProfileAdapter : public QObject
{
public:
    explicit ProfileAdapter(const QString &storagePrefix = QString());
    virtual ~ProfileAdapter();

    static ProfileAdapter* createDefaultProfileAdapter();
    static ProfileAdapter* defaultProfileAdapter();
    static QObject* globalQObjectRoot();

    VisitedLinksManagerQt *visitedLinksManager();
    DownloadManagerDelegateQt *downloadManagerDelegate();

    QWebEngineCookieStore *cookieStore();

    QWebEngineUrlRequestInterceptor* requestInterceptor();
    void setRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor);

    QList<ProfileAdapterClient*> clients() { return m_clients; }
    void addClient(ProfileAdapterClient *adapterClient);
    void removeClient(ProfileAdapterClient *adapterClient);

    void cancelDownload(quint32 downloadId);
    void pauseDownload(quint32 downloadId);
    void resumeDownload(quint32 downloadId);
    void removeDownload(quint32 downloadId);

    ProfileQt *profile();

    QString storageName() const { return m_name; }
    void setStorageName(const QString &storageName);

    bool isOffTheRecord() const { return m_offTheRecord; }
    void setOffTheRecord(bool offTheRecord);

    QString dataPath() const;
    void setDataPath(const QString &path);

    QString downloadPath() const { return m_downloadPath; }
    void setDownloadPath(const QString &path);

    QString cachePath() const;
    void setCachePath(const QString &path);

    QString httpCachePath() const;

    QString httpUserAgent() const;
    void setHttpUserAgent(const QString &userAgent);

    void setSpellCheckLanguages(const QStringList &language);
    QStringList spellCheckLanguages() const;
    void setSpellCheckEnabled(bool enabled);
    bool isSpellCheckEnabled() const;

    void addWebContentsAdapterClient(WebContentsAdapterClient *client);
    void removeWebContentsAdapterClient(WebContentsAdapterClient *client);

    // KEEP IN SYNC with API or add mapping layer
    enum HttpCacheType {
        MemoryHttpCache = 0,
        DiskHttpCache,
        NoCache
    };

    enum PersistentCookiesPolicy {
        NoPersistentCookies = 0,
        AllowPersistentCookies,
        ForcePersistentCookies
    };

    enum VisitedLinksPolicy {
        DoNotTrackVisitedLinks = 0,
        TrackVisitedLinksInMemory,
        TrackVisitedLinksOnDisk,
    };

    enum PermissionType {
        UnsupportedPermission = 0,
        GeolocationPermission = 1,
        NotificationPermission = 2,
        AudioCapturePermission = 3,
        VideoCapturePermission = 4,
        ClipboardRead = 5,
        ClipboardWrite = 6,
    };

    enum PermissionState {
        AskPermission = 0,
        AllowedPermission = 1,
        DeniedPermission = 2
    };

    HttpCacheType httpCacheType() const;
    void setHttpCacheType(ProfileAdapter::HttpCacheType);

    PersistentCookiesPolicy persistentCookiesPolicy() const;
    void setPersistentCookiesPolicy(ProfileAdapter::PersistentCookiesPolicy);

    VisitedLinksPolicy visitedLinksPolicy() const;
    void setVisitedLinksPolicy(ProfileAdapter::VisitedLinksPolicy);

    int httpCacheMaxSize() const;
    void setHttpCacheMaxSize(int maxSize);

    bool trackVisitedLinks() const;

    QWebEngineUrlSchemeHandler *urlSchemeHandler(const QByteArray &scheme);
    void installUrlSchemeHandler(const QByteArray &scheme, QWebEngineUrlSchemeHandler *handler);
    void removeUrlScheme(const QByteArray &scheme);
    void removeUrlSchemeHandler(QWebEngineUrlSchemeHandler *handler);
    void removeAllUrlSchemeHandlers();

    const QList<QByteArray> customUrlSchemes() const;
    UserResourceControllerHost *userResourceController();

    void permissionRequestReply(const QUrl &origin, PermissionType type, PermissionState reply);
    bool checkPermission(const QUrl &origin, PermissionType type);

    QString httpAcceptLanguageWithoutQualities() const;
    QString httpAcceptLanguage() const;
    void setHttpAcceptLanguage(const QString &httpAcceptLanguage);

    void clearHttpCache();

    void setUseForGlobalCertificateVerification(bool enable = true);
    bool isUsedForGlobalCertificateVerification() const;

#if QT_CONFIG(ssl)
    QWebEngineClientCertificateStore *clientCertificateStore();
#endif

    QHash<QByteArray, QWeakPointer<UserNotificationController>> &ephemeralNotifications()
    {   return m_ephemeralNotifications; }
    QHash<QByteArray, QSharedPointer<UserNotificationController>> &persistentNotifications()
    {   return m_persistentNotifications; }

    QString determineDownloadPath(const QString &downloadDirectory, const QString &suggestedFilename, const time_t &startTime);

private:
    void updateCustomUrlSchemeHandlers();
    void resetVisitedLinksManager();
    bool persistVisitedLinks() const;

    QString m_name;
    bool m_offTheRecord;
    bool m_usedForGlobalCertificateVerification = false;
    QScopedPointer<ProfileQt> m_profile;
    QScopedPointer<VisitedLinksManagerQt> m_visitedLinksManager;
    QScopedPointer<DownloadManagerDelegateQt> m_downloadManagerDelegate;
    QScopedPointer<UserResourceControllerHost> m_userResourceController;
    QScopedPointer<QWebEngineCookieStore> m_cookieStore;
#if QT_CONFIG(ssl)
    QWebEngineClientCertificateStore *m_clientCertificateStore = nullptr;
#endif
    QPointer<QWebEngineUrlRequestInterceptor> m_requestInterceptor;

    QString m_dataPath;
    QString m_downloadPath;
    QString m_cachePath;
    QString m_httpUserAgent;
    HttpCacheType m_httpCacheType;
    QString m_httpAcceptLanguage;
    PersistentCookiesPolicy m_persistentCookiesPolicy;
    VisitedLinksPolicy m_visitedLinksPolicy;
    QHash<QByteArray, QPointer<QWebEngineUrlSchemeHandler>> m_customUrlSchemeHandlers;
    QHash<QByteArray, QWeakPointer<UserNotificationController>> m_ephemeralNotifications;
    QHash<QByteArray, QSharedPointer<UserNotificationController>> m_persistentNotifications;

    QList<ProfileAdapterClient*> m_clients;
    QVector<WebContentsAdapterClient *> m_webContentsAdapterClients;
    int m_httpCacheMaxSize;
    QrcUrlSchemeHandler m_qrcHandler;

    Q_DISABLE_COPY(ProfileAdapter)
};

} // namespace QtWebEngineCore

#endif // PROFILE_ADAPTER_H
