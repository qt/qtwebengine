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

#include "profile_adapter.h"

#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browsing_data_remover.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/shared_cors_origin_access_list.h"
#include "content/public/browser/storage_partition.h"
#include "services/network/public/cpp/cors/origin_access_list.h"
#include "url/url_util.h"

#include "api/qwebengineurlscheme.h"
#include "content_browser_client_qt.h"
#include "download_manager_delegate_qt.h"
#include "permission_manager_qt.h"
#include "profile_adapter_client.h"
#include "profile_io_data_qt.h"
#include "profile_qt.h"
#include "renderer_host/user_resource_controller_host.h"
#include "type_conversion.h"
#include "visited_links_manager_qt.h"
#include "web_engine_context.h"
#include "web_contents_adapter_client.h"

#include "base/files/file_util.h"
#include "base/time/time_to_iso8601.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/browser/extension_system.h"
#endif

#include <QCoreApplication>
#include <QDir>
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
    , m_httpCacheMaxSize(0)
{
    WebEngineContext::current()->addProfileAdapter(this);
    // creation of profile requires webengine context
    m_profile.reset(new ProfileQt(this));
    content::BrowserContext::Initialize(m_profile.data(), toFilePath(dataPath()));
    // fixme: this should not be here
    m_profile->m_profileIOData->initializeOnUIThread();
    m_customUrlSchemeHandlers.insert(QByteArrayLiteral("qrc"), &m_qrcHandler);
#if BUILDFLAG(ENABLE_EXTENSIONS)
    if (!storageName.isEmpty())
        extensions::ExtensionSystem::Get(m_profile.data())->InitForRegularProfile(true);
#endif

    // Allow XMLHttpRequests from qrc to file.
    // ### consider removing for Qt6
    url::Origin qrc = url::Origin::Create(GURL("qrc://"));
    auto pattern = network::mojom::CorsOriginPattern::New("file", "", 0,
                                                          network::mojom::CorsDomainMatchMode::kAllowSubdomains,
                                                          network::mojom::CorsPortMatchMode::kAllowAnyPort,
                                                          network::mojom::CorsOriginAccessMatchPriority::kDefaultPriority);
    std::vector<network::mojom::CorsOriginPatternPtr> list;
    list.push_back(std::move(pattern));
    m_profile->GetSharedCorsOriginAccessList()->SetForOrigin(qrc, std::move(list), {}, base::BindOnce([]{}));
}

ProfileAdapter::~ProfileAdapter()
{
    while (!m_webContentsAdapterClients.isEmpty()) {
       m_webContentsAdapterClients.first()->releaseProfile();
    }
    WebEngineContext::current()->removeProfileAdapter(this);
    if (m_downloadManagerDelegate) {
        m_profile->GetDownloadManager(m_profile.data())->Shutdown();
        m_downloadManagerDelegate.reset();
    }
#if QT_CONFIG(ssl)
    delete m_clientCertificateStore;
#endif
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
}

ProfileQt *ProfileAdapter::profile()
{
    return m_profile.data();
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

void ProfileAdapter::cancelDownload(quint32 downloadId)
{
    downloadManagerDelegate()->cancelDownload(downloadId);
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
    return buildLocationFromStandardPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation), name);
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

    content::BrowserContext::ForEachStoragePartition(
        m_profile.get(), base::BindRepeating([](const std::string &user_agent, content::StoragePartition *storage_partition) {
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
        if (m_httpCacheType == NoCache)
            clearHttpCache();
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
    content::BrowserContext::ForEachStoragePartition(
        m_profile.get(), base::BindRepeating([](content::StoragePartition *storage_partition) {
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
            blink::mojom::RendererPreferences *rendererPrefs = web_contents->GetMutableRendererPrefs();
            rendererPrefs->accept_languages = http_accept_language;
            web_contents->SyncRendererPrefs();
        }
    }

    content::BrowserContext::ForEachStoragePartition(
        m_profile.get(), base::BindRepeating([](std::string accept_language, content::StoragePartition *storage_partition) {
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

void ProfileAdapter::addWebContentsAdapterClient(WebContentsAdapterClient *client)
{
    m_webContentsAdapterClients.append(client);
}

void ProfileAdapter::removeWebContentsAdapterClient(WebContentsAdapterClient *client)
{
    m_webContentsAdapterClients.removeAll(client);
}

void ProfileAdapter::resetVisitedLinksManager()
{
    m_visitedLinksManager.reset(new VisitedLinksManagerQt(m_profile.data(), persistVisitedLinks()));
}

void ProfileAdapter::setUseForGlobalCertificateVerification(bool enable)
{
    if (m_usedForGlobalCertificateVerification == enable)
        return;

    static QPointer<ProfileAdapter> profileForglobalCertificateVerification;

    m_usedForGlobalCertificateVerification = enable;
    if (enable) {
        if (profileForglobalCertificateVerification) {
            profileForglobalCertificateVerification->m_usedForGlobalCertificateVerification = false;
            if (!m_profile->m_profileIOData->isClearHttpCacheInProgress())
                profileForglobalCertificateVerification->m_profile->m_profileIOData->resetNetworkContext();
            for (auto *client : qAsConst(profileForglobalCertificateVerification->m_clients))
                client->useForGlobalCertificateVerificationChanged();
        }
        profileForglobalCertificateVerification = this;
    } else {
        Q_ASSERT(profileForglobalCertificateVerification);
        Q_ASSERT(profileForglobalCertificateVerification == this);
        profileForglobalCertificateVerification = nullptr;
    }

    if (!m_profile->m_profileIOData->isClearHttpCacheInProgress())
        m_profile->m_profileIOData->resetNetworkContext();
}

bool ProfileAdapter::isUsedForGlobalCertificateVerification() const
{
    return m_usedForGlobalCertificateVerification;
}

QString ProfileAdapter::determineDownloadPath(const QString &downloadDirectory, const QString &suggestedFilename, const time_t &startTime)
{
    QFileInfo suggestedFile(QDir(downloadDirectory).absoluteFilePath(suggestedFilename));
    QString suggestedFilePath = suggestedFile.absoluteFilePath();
    base::FilePath tmpFilePath(toFilePath(suggestedFilePath).NormalizePathSeparatorsTo('/'));

    int uniquifier = base::GetUniquePathNumber(tmpFilePath);
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

} // namespace QtWebEngineCore
