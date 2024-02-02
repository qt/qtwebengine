// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "profile_adapter.h"

#include "base/files/file_util.h"
#include "base/task/cancelable_task_tracker.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/time_to_iso8601.h"
#include "components/favicon/core/favicon_service.h"
#include "components/history/content/browser/history_database_helper.h"
#include "components/history/core/browser/history_database_params.h"
#include "components/history/core/browser/history_service.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browsing_data_remover.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/storage_partition.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "url/url_util.h"

#include "api/qwebengineurlscheme.h"
#include "content_browser_client_qt.h"
#include "download_manager_delegate_qt.h"
#include "favicon_service_factory_qt.h"
#include "permission_manager_qt.h"
#include "profile_adapter_client.h"
#include "profile_io_data_qt.h"
#include "profile_qt.h"
#include "renderer_host/user_resource_controller_host.h"
#include "type_conversion.h"
#include "visited_links_manager_qt.h"
#include "web_contents_adapter_client.h"
#include "web_engine_context.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/browser/extension_system.h"
#endif

#include <QCoreApplication>
#include <QDir>
#include <QSet>
#include <QString>
#include <QStandardPaths>

namespace {
inline QString buildLocationFromStandardPath(const QString &standardPath, const QString &name) {
    QString location = standardPath;
    if (location.isEmpty())
        location = QDir::homePath() % QLatin1String("/.") % QCoreApplication::applicationName();

    location.append(QLatin1String("/QtWebEngine/") % name);
    return location;
}
}

namespace QtWebEngineCore {

ProfileAdapter::ProfileAdapter(const QString &storageName):
      m_name(storageName)
    , m_offTheRecord(storageName.isEmpty())
    , m_downloadPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation))
    , m_httpCacheType(DiskHttpCache)
    , m_persistentCookiesPolicy(AllowPersistentCookies)
    , m_visitedLinksPolicy(TrackVisitedLinksOnDisk)
    , m_pushServiceEnabled(false)
    , m_httpCacheMaxSize(0)
{
    WebEngineContext::current()->addProfileAdapter(this);
    // creation of profile requires webengine context
    m_profile.reset(new ProfileQt(this));
    // fixme: this should not be here
    m_profile->m_profileIOData->initializeOnUIThread();
    m_customUrlSchemeHandlers.insert(QByteArrayLiteral("qrc"), &m_qrcHandler);
#if BUILDFLAG(ENABLE_EXTENSIONS)
    if (!storageName.isEmpty())
        extensions::ExtensionSystem::Get(m_profile.data())->InitForRegularProfile(true);
#endif
    m_cancelableTaskTracker.reset(new base::CancelableTaskTracker());

    m_profile->DoFinalInit();
}

ProfileAdapter::~ProfileAdapter()
{
    m_cancelableTaskTracker->TryCancelAll();
    m_profile->NotifyWillBeDestroyed();
    releaseAllWebContentsAdapterClients();

    WebEngineContext::current()->removeProfileAdapter(this);
    if (m_downloadManagerDelegate) {
        m_profile->GetDownloadManager()->Shutdown();
        m_downloadManagerDelegate.reset();
    }
#if QT_CONFIG(ssl)
    delete m_clientCertificateStore;
    m_clientCertificateStore = nullptr;
#endif
    WebEngineContext::flushMessages();
}

void ProfileAdapter::setStorageName(const QString &storageName)
{
    if (storageName == m_name)
        return;
    m_name = storageName;
    if (!m_offTheRecord) {
        m_profile->setupPrefService();
        if (!m_profile->m_profileIOData->isClearHttpCacheInProgress())
            m_profile->m_profileIOData->resetNetworkContext();
        if (m_visitedLinksManager)
            resetVisitedLinksManager();

        reinitializeHistoryService();
    }
}

void ProfileAdapter::setOffTheRecord(bool offTheRecord)
{
    if (offTheRecord == m_offTheRecord)
        return;
    m_offTheRecord = offTheRecord;
    m_profile->setupPrefService();
    if (!m_profile->m_profileIOData->isClearHttpCacheInProgress())
        m_profile->m_profileIOData->resetNetworkContext();
    if (m_visitedLinksManager)
        resetVisitedLinksManager();

    if (offTheRecord) {
        favicon::FaviconService *faviconService =
                FaviconServiceFactoryQt::GetForBrowserContext(m_profile.data());
        faviconService->SetHistoryService(nullptr);
    } else if (!m_name.isEmpty()) {
        reinitializeHistoryService();
    }
}

ProfileQt *ProfileAdapter::profile()
{
    return m_profile.data();
}

bool ProfileAdapter::ensureDataPathExists()
{
    Q_ASSERT(!m_offTheRecord);
    base::ScopedAllowBlocking allowBlock;
    const base::FilePath &path = toFilePath(dataPath());
    if (path.empty())
        return false;
    if (base::DirectoryExists(path))
        return true;

    base::File::Error error;
    if (base::CreateDirectoryAndGetError(path, &error))
        return true;

    std::string errorstr = base::File::ErrorToString(error);
    qWarning("Cannot create directory %s. Error: %s.", path.AsUTF8Unsafe().c_str(),
             errorstr.c_str());
    return false;
}

VisitedLinksManagerQt *ProfileAdapter::visitedLinksManager()
{
    if (!m_visitedLinksManager)
        resetVisitedLinksManager();
    return m_visitedLinksManager.data();
}

DownloadManagerDelegateQt *ProfileAdapter::downloadManagerDelegate()
{
    if (!m_downloadManagerDelegate)
        m_downloadManagerDelegate.reset(new DownloadManagerDelegateQt(this));
    return m_downloadManagerDelegate.data();
}

QWebEngineCookieStore *ProfileAdapter::cookieStore()
{
    if (!m_cookieStore)
        m_cookieStore.reset(new QWebEngineCookieStore);
    return m_cookieStore.data();
}

QWebEngineUrlRequestInterceptor *ProfileAdapter::requestInterceptor()
{
    return m_requestInterceptor.data();
}

void ProfileAdapter::setRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor)
{
    m_requestInterceptor = interceptor;
}

void ProfileAdapter::addClient(ProfileAdapterClient *adapterClient)
{
    m_clients.append(adapterClient);
}

void ProfileAdapter::removeClient(ProfileAdapterClient *adapterClient)
{
    m_clients.removeOne(adapterClient);
}

bool ProfileAdapter::cancelDownload(quint32 downloadId)
{
    return downloadManagerDelegate()->cancelDownload(downloadId);
}

void ProfileAdapter::pauseDownload(quint32 downloadId)
{
    downloadManagerDelegate()->pauseDownload(downloadId);
}

void ProfileAdapter::resumeDownload(quint32 downloadId)
{
    downloadManagerDelegate()->resumeDownload(downloadId);
}

void ProfileAdapter::removeDownload(quint32 downloadId)
{
    downloadManagerDelegate()->removeDownload(downloadId);
}

ProfileAdapter *ProfileAdapter::createDefaultProfileAdapter()
{
    return WebEngineContext::current()->createDefaultProfileAdapter();
}

ProfileAdapter *ProfileAdapter::defaultProfileAdapter()
{
    WebEngineContext *context = WebEngineContext::current();
    return context ? context->defaultProfileAdapter() : nullptr;
}

QObject* ProfileAdapter::globalQObjectRoot()
{
    return WebEngineContext::current()->globalQObject();
}

QString ProfileAdapter::dataPath() const
{
    if (!m_dataPath.isEmpty())
        return m_dataPath;
    // And off-the-record or memory-only profile should not write to disk
    // but Chromium often creates temporary directories anyway, so given them
    // a location to do so.
    QString name = m_name;
    if (m_offTheRecord)
        name = QStringLiteral("OffTheRecord");
    else if (m_name.isEmpty())
        name = QStringLiteral("UnknownProfile");
    return buildLocationFromStandardPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation), name);
}

void ProfileAdapter::setDataPath(const QString &path)
{
    if (m_dataPath == path)
        return;
    m_dataPath = path;
    m_profile->setupPrefService();
    if (!m_profile->m_profileIOData->isClearHttpCacheInProgress())
        m_profile->m_profileIOData->resetNetworkContext();
    if (!m_offTheRecord && m_visitedLinksManager)
        resetVisitedLinksManager();

    if (!m_offTheRecord)
        reinitializeHistoryService();
}

void ProfileAdapter::setDownloadPath(const QString &path)
{
    m_downloadPath = path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) : path;
}

QString ProfileAdapter::cachePath() const
{
    if (m_offTheRecord)
        return QString();
    if (!m_cachePath.isEmpty())
        return m_cachePath;
    if (!m_name.isNull())
        return buildLocationFromStandardPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation), m_name);
    return QString();
}

void ProfileAdapter::setCachePath(const QString &path)
{
    if (m_cachePath == path)
        return;
    m_cachePath = path;
    if (!m_offTheRecord && !m_profile->m_profileIOData->isClearHttpCacheInProgress())
        m_profile->m_profileIOData->resetNetworkContext();
}

QString ProfileAdapter::httpCachePath() const
{
    if (m_offTheRecord)
        return QString();
    QString basePath = cachePath();
    if (!basePath.isEmpty())
        return basePath % QLatin1String("/Cache");
    return QString();
}

QString ProfileAdapter::httpUserAgent() const
{
    if (m_httpUserAgent.isNull())
        return QString::fromStdString(ContentBrowserClientQt::getUserAgent());
    return m_httpUserAgent;
}

void ProfileAdapter::setHttpUserAgent(const QString &userAgent)
{
    const QString httpUserAgent = userAgent.simplified();
    if (m_httpUserAgent == httpUserAgent)
        return;
    m_httpUserAgent = httpUserAgent;
    const std::string stdUserAgent = httpUserAgent.toStdString();

    std::vector<content::WebContentsImpl *> list = content::WebContentsImpl::GetAllWebContents();
    for (content::WebContentsImpl *web_contents : list)
        if (web_contents->GetBrowserContext() == m_profile.data())
            web_contents->SetUserAgentOverride(blink::UserAgentOverride::UserAgentOnly(stdUserAgent), true);

    m_profile->ForEachLoadedStoragePartition(
                base::BindRepeating([](const std::string &user_agent, content::StoragePartition *storage_partition) {
                    storage_partition->GetNetworkContext()->SetUserAgent(user_agent);
                }, stdUserAgent));
}

ProfileAdapter::HttpCacheType ProfileAdapter::httpCacheType() const
{
    if (m_httpCacheType == NoCache)
        return NoCache;
    if (isOffTheRecord() || httpCachePath().isEmpty())
        return MemoryHttpCache;
    return m_httpCacheType;
}

void ProfileAdapter::setHttpCacheType(ProfileAdapter::HttpCacheType newhttpCacheType)
{
    ProfileAdapter::HttpCacheType oldCacheType = httpCacheType();
    m_httpCacheType = newhttpCacheType;
    if (oldCacheType == httpCacheType())
        return;
    if (!m_offTheRecord && !m_profile->m_profileIOData->isClearHttpCacheInProgress()) {
        m_profile->m_profileIOData->resetNetworkContext();
    }
}

ProfileAdapter::PersistentCookiesPolicy ProfileAdapter::persistentCookiesPolicy() const
{
    if (isOffTheRecord() || m_name.isEmpty())
        return NoPersistentCookies;
    return m_persistentCookiesPolicy;
}

void ProfileAdapter::setPersistentCookiesPolicy(ProfileAdapter::PersistentCookiesPolicy newPersistentCookiesPolicy)
{
    ProfileAdapter::PersistentCookiesPolicy oldPolicy = persistentCookiesPolicy();
    m_persistentCookiesPolicy = newPersistentCookiesPolicy;
    if (oldPolicy == persistentCookiesPolicy())
        return;
    if (!m_offTheRecord && !m_profile->m_profileIOData->isClearHttpCacheInProgress())
        m_profile->m_profileIOData->resetNetworkContext();
}

ProfileAdapter::VisitedLinksPolicy ProfileAdapter::visitedLinksPolicy() const
{
    if (isOffTheRecord() || m_visitedLinksPolicy == DoNotTrackVisitedLinks)
        return DoNotTrackVisitedLinks;
    if (m_name.isEmpty())
        return TrackVisitedLinksInMemory;
    return m_visitedLinksPolicy;
}

bool ProfileAdapter::trackVisitedLinks() const
{
    switch (visitedLinksPolicy()) {
    case DoNotTrackVisitedLinks:
        return false;
    default:
        break;
    }
    return true;
}

bool ProfileAdapter::persistVisitedLinks() const
{
    switch (visitedLinksPolicy()) {
    case DoNotTrackVisitedLinks:
    case TrackVisitedLinksInMemory:
        return false;
    default:
        break;
    }
    return true;
}

void ProfileAdapter::setVisitedLinksPolicy(ProfileAdapter::VisitedLinksPolicy visitedLinksPolicy)
{
    if (m_visitedLinksPolicy == visitedLinksPolicy)
        return;
    m_visitedLinksPolicy = visitedLinksPolicy;
    if (m_visitedLinksManager)
        resetVisitedLinksManager();
}

int ProfileAdapter::httpCacheMaxSize() const
{
    return m_httpCacheMaxSize;
}

void ProfileAdapter::setHttpCacheMaxSize(int maxSize)
{
    if (m_httpCacheMaxSize == maxSize)
        return;
    m_httpCacheMaxSize = maxSize;
    if (!m_offTheRecord && !m_profile->m_profileIOData->isClearHttpCacheInProgress())
        m_profile->m_profileIOData->resetNetworkContext();
}

enum class SchemeType { Protected, Overridable, Custom, Unknown };
static SchemeType schemeType(const QByteArray &canonicalScheme)
{
    static const QSet<QByteArray> blacklist{
        QByteArrayLiteral("about"),
        QByteArrayLiteral("blob"),
        QByteArrayLiteral("data"),
        QByteArrayLiteral("javascript"),
        QByteArrayLiteral("qrc"),
        // See also kStandardURLSchemes in url/url_util.cc (through url::IsStandard below)
    };

    static const QSet<QByteArray> whitelist{
        QByteArrayLiteral("gopher"),
    };

    bool standardSyntax = url::IsStandard(canonicalScheme.data(), url::Component(0, canonicalScheme.size()));
    bool customScheme = QWebEngineUrlScheme::schemeByName(canonicalScheme) != QWebEngineUrlScheme();
    bool blacklisted = blacklist.contains(canonicalScheme);
    bool whitelisted = whitelist.contains(canonicalScheme);

    if (whitelisted)
        return SchemeType::Overridable;
    if (blacklisted || (standardSyntax && !customScheme))
        return SchemeType::Protected;
    if (customScheme)
        return SchemeType::Custom;
    return SchemeType::Unknown;
}

QWebEngineUrlSchemeHandler *ProfileAdapter::urlSchemeHandler(const QByteArray &scheme)
{
    return m_customUrlSchemeHandlers.value(scheme.toLower()).data();
}

const QList<QByteArray> ProfileAdapter::customUrlSchemes() const
{
    return m_customUrlSchemeHandlers.keys();
}

void ProfileAdapter::updateCustomUrlSchemeHandlers()
{
    m_profile->ForEachLoadedStoragePartition(
        base::BindRepeating([](content::StoragePartition *storage_partition) {
            storage_partition->ResetURLLoaderFactories();
        }));
}

void ProfileAdapter::removeUrlSchemeHandler(QWebEngineUrlSchemeHandler *handler)
{
    Q_ASSERT(handler);
    bool removedOneOrMore = false;
    auto it = m_customUrlSchemeHandlers.begin();
    while (it != m_customUrlSchemeHandlers.end()) {
        if (it.value() == handler) {
            if (schemeType(it.key()) == SchemeType::Protected) {
                qWarning("Cannot remove the URL scheme handler for an internal scheme: %s", it.key().constData());
                continue;
            }
            it = m_customUrlSchemeHandlers.erase(it);
            removedOneOrMore = true;
            continue;
        }
        ++it;
    }
    if (removedOneOrMore)
        updateCustomUrlSchemeHandlers();
}

void ProfileAdapter::removeUrlScheme(const QByteArray &scheme)
{
    QByteArray canonicalScheme = scheme.toLower();
    if (schemeType(canonicalScheme) == SchemeType::Protected) {
        qWarning("Cannot remove the URL scheme handler for an internal scheme: %s", scheme.constData());
        return;
    }
    if (m_customUrlSchemeHandlers.remove(canonicalScheme))
        updateCustomUrlSchemeHandlers();
}

void ProfileAdapter::installUrlSchemeHandler(const QByteArray &scheme, QWebEngineUrlSchemeHandler *handler)
{
    Q_ASSERT(handler);
    QByteArray canonicalScheme = scheme.toLower();
    SchemeType type = schemeType(canonicalScheme);

    if (type == SchemeType::Protected) {
        qWarning("Cannot install a URL scheme handler overriding internal scheme: %s", scheme.constData());
        return;
    }

    if (m_customUrlSchemeHandlers.value(canonicalScheme, handler) != handler) {
        qWarning("URL scheme handler already installed for the scheme: %s", scheme.constData());
        return;
    }

    if (type == SchemeType::Unknown)
        qWarning("Please register the custom scheme '%s' via QWebEngineUrlScheme::registerScheme() "
                 "before installing the custom scheme handler.", scheme.constData());

    m_customUrlSchemeHandlers.insert(canonicalScheme, handler);
    updateCustomUrlSchemeHandlers();
}

void ProfileAdapter::removeAllUrlSchemeHandlers()
{
    if (m_customUrlSchemeHandlers.size() > 1) {
        m_customUrlSchemeHandlers.clear();
        m_customUrlSchemeHandlers.insert(QByteArrayLiteral("qrc"), &m_qrcHandler);
        updateCustomUrlSchemeHandlers();
    }
}

UserResourceControllerHost *ProfileAdapter::userResourceController()
{
    if (!m_userResourceController)
        m_userResourceController.reset(new UserResourceControllerHost);
    return m_userResourceController.data();
}

void ProfileAdapter::permissionRequestReply(const QUrl &origin, PermissionType type, PermissionState reply)
{
    static_cast<PermissionManagerQt*>(profile()->GetPermissionControllerDelegate())->permissionRequestReply(origin, type, reply);
}

bool ProfileAdapter::checkPermission(const QUrl &origin, PermissionType type)
{
    return static_cast<PermissionManagerQt*>(profile()->GetPermissionControllerDelegate())->checkPermission(origin, type);
}

QString ProfileAdapter::httpAcceptLanguageWithoutQualities() const
{
    const QStringList list = m_httpAcceptLanguage.split(QLatin1Char(','));
    QString out;
    for (const QString &str : list) {
        if (!out.isEmpty())
            out.append(QLatin1Char(','));
        out.append(str.split(QLatin1Char(';')).first());
    }
    return out;
}

QString ProfileAdapter::httpAcceptLanguage() const
{
    return m_httpAcceptLanguage;
}

void ProfileAdapter::setHttpAcceptLanguage(const QString &httpAcceptLanguage)
{
    if (m_httpAcceptLanguage == httpAcceptLanguage)
        return;
    m_httpAcceptLanguage = httpAcceptLanguage;

    std::string http_accept_language = httpAcceptLanguageWithoutQualities().toStdString();

    std::vector<content::WebContentsImpl *> list = content::WebContentsImpl::GetAllWebContents();
    for (content::WebContentsImpl *web_contents : list) {
        if (web_contents->GetBrowserContext() == m_profile.data()) {
            blink::RendererPreferences *rendererPrefs = web_contents->GetMutableRendererPrefs();
            rendererPrefs->accept_languages = http_accept_language;
            web_contents->SyncRendererPrefs();
        }
    }

    m_profile->ForEachLoadedStoragePartition(
        base::BindRepeating([](std::string accept_language, content::StoragePartition *storage_partition) {
            storage_partition->GetNetworkContext()->SetAcceptLanguage(accept_language);
        }, http_accept_language));
}

void ProfileAdapter::clearHttpCache()
{
    m_profile->m_profileIOData->clearHttpCache();
}

void ProfileAdapter::setSpellCheckLanguages(const QStringList &languages)
{
#if QT_CONFIG(webengine_spellchecker)
    m_profile->prefServiceAdapter().setSpellCheckLanguages(languages);
#endif
}

QStringList ProfileAdapter::spellCheckLanguages() const
{
#if QT_CONFIG(webengine_spellchecker)
    return m_profile->prefServiceAdapter().spellCheckLanguages();
#else
    return QStringList();
#endif
}

void ProfileAdapter::setSpellCheckEnabled(bool enabled)
{
#if QT_CONFIG(webengine_spellchecker)
    m_profile->prefServiceAdapter().setSpellCheckEnabled(enabled);
#endif
}

bool ProfileAdapter::isSpellCheckEnabled() const
{
#if QT_CONFIG(webengine_spellchecker)
    return m_profile->prefServiceAdapter().isSpellCheckEnabled();
#else
    return false;
#endif
}

bool ProfileAdapter::pushServiceEnabled() const
{
  return m_pushServiceEnabled;
}

void ProfileAdapter::setPushServiceEnabled(bool enabled)
{
    m_pushServiceEnabled = enabled;
}

void ProfileAdapter::addWebContentsAdapterClient(WebContentsAdapterClient *client)
{
    m_webContentsAdapterClients.append(client);
}

void ProfileAdapter::removeWebContentsAdapterClient(WebContentsAdapterClient *client)
{
    m_webContentsAdapterClients.removeAll(client);
}

void ProfileAdapter::releaseAllWebContentsAdapterClients()
{
    while (!m_webContentsAdapterClients.isEmpty())
        m_webContentsAdapterClients.first()->releaseProfile();
}

void ProfileAdapter::resetVisitedLinksManager()
{
    m_visitedLinksManager.reset(new VisitedLinksManagerQt(m_profile.data(), persistVisitedLinks()));
}

void ProfileAdapter::reinitializeHistoryService()
{
    Q_ASSERT(!m_offTheRecord);
    if (ensureDataPathExists()) {
        favicon::FaviconService *faviconService =
                FaviconServiceFactoryQt::GetForBrowserContext(m_profile.data());
        history::HistoryService *historyService = static_cast<history::HistoryService *>(
                HistoryServiceFactoryQt::GetInstance()->GetForBrowserContext(m_profile.data()));
        Q_ASSERT(historyService);
        faviconService->SetHistoryService(historyService);
    } else {
        qWarning("Favicon database is not available for this profile.");
    }
}

QString ProfileAdapter::determineDownloadPath(const QString &downloadDirectory, const QString &suggestedFilename, const time_t &startTime)
{
    QFileInfo suggestedFile(QDir(downloadDirectory).absoluteFilePath(suggestedFilename));
    QString suggestedFilePath = suggestedFile.absoluteFilePath();
    base::FilePath tmpFilePath(toFilePath(suggestedFilePath).NormalizePathSeparatorsTo('/'));

    int uniquifier = 0;
    {
        base::ScopedAllowBlocking allowBlock;
        uniquifier = base::GetUniquePathNumber(tmpFilePath);
    }
    if (uniquifier > 0)
        suggestedFilePath = toQt(tmpFilePath.InsertBeforeExtensionASCII(base::StringPrintf(" (%d)", uniquifier)).AsUTF8Unsafe());
    else if (uniquifier == -1) {
        base::Time::Exploded exploded;
        base::Time::FromTimeT(startTime).LocalExplode(&exploded);
        std::string suffix = base::StringPrintf(
                    " - %04d-%02d-%02dT%02d%02d%02d.%03d", exploded.year, exploded.month,
                    exploded.day_of_month, exploded.hour, exploded.minute,
                    exploded.second, exploded.millisecond);
        suggestedFilePath = toQt(tmpFilePath.InsertBeforeExtensionASCII(suffix).AsUTF8Unsafe());
    }
    return suggestedFilePath;
}

#if QT_CONFIG(ssl)
QWebEngineClientCertificateStore *ProfileAdapter::clientCertificateStore()
{
    if (!m_clientCertificateStore)
        m_clientCertificateStore = new QWebEngineClientCertificateStore(m_profile->m_profileIOData->clientCertificateStoreData());
    return m_clientCertificateStore;
}
#endif

static void callbackOnIconAvailableForPageURL(std::function<void (const QIcon &, const QUrl &, const QUrl &)> iconAvailableCallback,
                                              const QUrl &pageUrl,
                                              const favicon_base::FaviconRawBitmapResult &result)
{
    if (!result.is_valid()) {
        iconAvailableCallback(QIcon(), toQt(result.icon_url), pageUrl);
        return;
    }
    QPixmap pixmap(toQt(result.pixel_size));
    pixmap.loadFromData(result.bitmap_data->data(), result.bitmap_data->size());
    iconAvailableCallback(QIcon(pixmap), toQt(result.icon_url), pageUrl);
}

void ProfileAdapter::requestIconForPageURL(const QUrl &pageUrl,
                                           int desiredSizeInPixel,
                                           bool touchIconsEnabled,
                                           std::function<void (const QIcon &, const QUrl &, const QUrl &)> iconAvailableCallback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    favicon::FaviconService *service = FaviconServiceFactoryQt::GetForBrowserContext(m_profile.data());

    if (!service->HistoryService()) {
        callbackOnIconAvailableForPageURL(iconAvailableCallback, pageUrl,
                                          favicon_base::FaviconRawBitmapResult());
        return;
    }

    favicon_base::IconTypeSet types = { favicon_base::IconType::kFavicon };
    if (touchIconsEnabled) {
        types.insert(favicon_base::IconType::kTouchIcon);
        types.insert(favicon_base::IconType::kTouchPrecomposedIcon);
        types.insert(favicon_base::IconType::kWebManifestIcon);
    }
    service->GetRawFaviconForPageURL(
            toGurl(pageUrl), types, desiredSizeInPixel, true /* fallback_to_host */,
            base::BindOnce(&callbackOnIconAvailableForPageURL, iconAvailableCallback, pageUrl),
            m_cancelableTaskTracker.get());
}

static void callbackOnIconAvailableForIconURL(std::function<void (const QIcon &, const QUrl &)> iconAvailableCallback,
                                              ProfileAdapter *profileAdapter,
                                              const QUrl &iconUrl, int iconType,
                                              int desiredSizeInPixel,
                                              bool touchIconsEnabled,
                                              const favicon_base::FaviconRawBitmapResult &result)
{
    if (!result.is_valid()) {
        // If touch icons are disabled there is no need to try further icon types.
        if (!touchIconsEnabled) {
            iconAvailableCallback(QIcon(), iconUrl);
            return;
        }
        if (static_cast<favicon_base::IconType>(iconType) != favicon_base::IconType::kMax) {
            //Q_ASSERT(profileAdapter->profile());
            favicon::FaviconService *service = FaviconServiceFactoryQt::GetForBrowserContext(profileAdapter->profile());
            service->GetRawFavicon(toGurl(iconUrl),
                                   static_cast<favicon_base::IconType>(iconType + 1),
                                   desiredSizeInPixel,
                                   base::BindOnce(&callbackOnIconAvailableForIconURL, iconAvailableCallback,
                                                  profileAdapter, iconUrl, iconType + 1, desiredSizeInPixel,
                                                  touchIconsEnabled),
                                   profileAdapter->cancelableTaskTracker());
            return;
        }
        iconAvailableCallback(QIcon(), iconUrl);
        return;
    }
    QPixmap pixmap(toQt(result.pixel_size));
    pixmap.loadFromData(result.bitmap_data->data(), result.bitmap_data->size());
    iconAvailableCallback(QIcon(pixmap), toQt(result.icon_url));
}

void ProfileAdapter::requestIconForIconURL(const QUrl &iconUrl,
                                           int desiredSizeInPixel,
                                           bool touchIconsEnabled,
                                           std::function<void (const QIcon &, const QUrl &)> iconAvailableCallback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    favicon::FaviconService *service = FaviconServiceFactoryQt::GetForBrowserContext(m_profile.data());

    if (!service->HistoryService()) {
        callbackOnIconAvailableForIconURL(iconAvailableCallback,
                                          this,
                                          iconUrl,
                                          static_cast<int>(favicon_base::IconType::kMax), 0,
                                          touchIconsEnabled,
                                          favicon_base::FaviconRawBitmapResult());
        return;
    }
    service->GetRawFavicon(
            toGurl(iconUrl), favicon_base::IconType::kFavicon, desiredSizeInPixel,
            base::BindOnce(&callbackOnIconAvailableForIconURL, iconAvailableCallback, this, iconUrl,
                           static_cast<int>(favicon_base::IconType::kFavicon), desiredSizeInPixel,
                           touchIconsEnabled),
            m_cancelableTaskTracker.get());
}

} // namespace QtWebEngineCore
