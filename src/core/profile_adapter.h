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

#ifndef PROFILE_ADAPTER_H
#define PROFILE_ADAPTER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

#include <QHash>
#include <QList>
#include <QPointer>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>

#include <QtWebEngineCore/qwebengineclientcertificatestore.h>
#include <QtWebEngineCore/qwebenginecookiestore.h>
#include <QtWebEngineCore/qwebengineurlrequestinterceptor.h>
#include <QtWebEngineCore/qwebengineurlschemehandler.h>
#include "net/qrc_url_scheme_handler.h"

QT_FORWARD_DECLARE_CLASS(QObject)

namespace base {
class CancelableTaskTracker;
}

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

    bool cancelDownload(quint32 downloadId);
    void pauseDownload(quint32 downloadId);
    void resumeDownload(quint32 downloadId);
    void removeDownload(quint32 downloadId);

    ProfileQt *profile();
    bool ensureDataPathExists();

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

    bool pushServiceEnabled() const;
    void setPushServiceEnabled(bool enabled);

    void addWebContentsAdapterClient(WebContentsAdapterClient *client);
    void removeWebContentsAdapterClient(WebContentsAdapterClient *client);
    void releaseAllWebContentsAdapterClients();

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

#if QT_CONFIG(ssl)
    QWebEngineClientCertificateStore *clientCertificateStore();
#endif

    QHash<QByteArray, QWeakPointer<UserNotificationController>> &ephemeralNotifications()
    {   return m_ephemeralNotifications; }
    QHash<QByteArray, QSharedPointer<UserNotificationController>> &persistentNotifications()
    {   return m_persistentNotifications; }

    QString determineDownloadPath(const QString &downloadDirectory, const QString &suggestedFilename, const time_t &startTime);

    void requestIconForPageURL(const QUrl &pageUrl, int desiredSizeInPixel, bool touchIconsEnabled,
                               std::function<void (const QIcon &, const QUrl &, const QUrl &)> iconAvailableCallback);
    void requestIconForIconURL(const QUrl &iconUrl, int desiredSizeInPixel, bool touchIconsEnabled,
                               std::function<void (const QIcon &, const QUrl &)> iconAvailableCallback);
    base::CancelableTaskTracker *cancelableTaskTracker() { return m_cancelableTaskTracker.get(); }

private:
    void updateCustomUrlSchemeHandlers();
    void resetVisitedLinksManager();
    bool persistVisitedLinks() const;
    void reinitializeHistoryService();

    QString m_name;
    bool m_offTheRecord;
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
    QList<WebContentsAdapterClient *> m_webContentsAdapterClients;
    bool m_pushServiceEnabled;
    int m_httpCacheMaxSize;
    QrcUrlSchemeHandler m_qrcHandler;
    std::unique_ptr<base::CancelableTaskTracker> m_cancelableTaskTracker;

    Q_DISABLE_COPY(ProfileAdapter)
};

} // namespace QtWebEngineCore

#endif // PROFILE_ADAPTER_H
