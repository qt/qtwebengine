// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "permission_manager_qt.h"

#include "base/threading/thread_restrictions.h"
#include "content/browser/renderer_host/render_view_host_delegate.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/permission_controller.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "third_party/blink/public/common/permissions/permission_utils.h"
#include "chrome/browser/prefs/chrome_command_line_pref_store.h"
#include "components/prefs/pref_member.h"
#include "components/prefs/in_memory_pref_store.h"
#include "components/prefs/json_pref_store.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "components/prefs/pref_service.h"

#include <QtWebEngineCore/private/qwebenginepermission_p.h>
#include "type_conversion.h"
#include "web_contents_delegate_qt.h"
#include "web_engine_settings.h"

namespace QtWebEngineCore {

static QWebEnginePermission::Feature toQt(blink::PermissionType type)
{
    switch (type) {
    case blink::PermissionType::GEOLOCATION:
        return QWebEnginePermission::Geolocation;
    case blink::PermissionType::AUDIO_CAPTURE:
        return QWebEnginePermission::MediaAudioCapture;
    case blink::PermissionType::VIDEO_CAPTURE:
        return QWebEnginePermission::MediaVideoCapture;
    case blink::PermissionType::DISPLAY_CAPTURE:
        return QWebEnginePermission::DesktopAudioVideoCapture;
        // We treat these both as read/write since we do not currently have a
        // ClipboardSanitizedWrite feature.
    case blink::PermissionType::CLIPBOARD_READ_WRITE:
    case blink::PermissionType::CLIPBOARD_SANITIZED_WRITE:
        return QWebEnginePermission::ClipboardReadWrite;
    case blink::PermissionType::NOTIFICATIONS:
        return QWebEnginePermission::Notifications;
    case blink::PermissionType::LOCAL_FONTS:
        return QWebEnginePermission::LocalFontsAccess;
    case blink::PermissionType::ACCESSIBILITY_EVENTS:
    case blink::PermissionType::CAMERA_PAN_TILT_ZOOM:
    case blink::PermissionType::WINDOW_MANAGEMENT:
        return QWebEnginePermission::Unsupported;
    case blink::PermissionType::MIDI_SYSEX:
    case blink::PermissionType::PROTECTED_MEDIA_IDENTIFIER:
    case blink::PermissionType::MIDI:
    case blink::PermissionType::DURABLE_STORAGE:
    case blink::PermissionType::BACKGROUND_SYNC:
    case blink::PermissionType::SENSORS:
    case blink::PermissionType::PAYMENT_HANDLER:
    case blink::PermissionType::BACKGROUND_FETCH:
    case blink::PermissionType::IDLE_DETECTION:
    case blink::PermissionType::PERIODIC_BACKGROUND_SYNC:
    case blink::PermissionType::WAKE_LOCK_SCREEN:
    case blink::PermissionType::WAKE_LOCK_SYSTEM:
    case blink::PermissionType::NFC:
    case blink::PermissionType::AR:
    case blink::PermissionType::VR:
    case blink::PermissionType::STORAGE_ACCESS_GRANT:
    case blink::PermissionType::TOP_LEVEL_STORAGE_ACCESS:
    case blink::PermissionType::CAPTURED_SURFACE_CONTROL:
    case blink::PermissionType::SMART_CARD:
    case blink::PermissionType::WEB_PRINTING:
    case blink::PermissionType::NUM:
        LOG(INFO) << "Unexpected unsupported Blink permission type: " << static_cast<int>(type);
        break;
    }
    return QWebEnginePermission::Unsupported;
}

static blink::PermissionType toBlink(QWebEnginePermission::Feature feature)
{
    switch (feature) {
    case QWebEnginePermission::Notifications:
        return blink::PermissionType::NOTIFICATIONS;
    case QWebEnginePermission::Geolocation:
        return blink::PermissionType::GEOLOCATION;
    case QWebEnginePermission::MediaAudioCapture:
        return blink::PermissionType::AUDIO_CAPTURE;
    case QWebEnginePermission::MediaVideoCapture:
        return blink::PermissionType::VIDEO_CAPTURE;
    case QWebEnginePermission::DesktopVideoCapture:
    case QWebEnginePermission::DesktopAudioVideoCapture:
        return blink::PermissionType::DISPLAY_CAPTURE;
    case QWebEnginePermission::ClipboardReadWrite:
        return blink::PermissionType::CLIPBOARD_READ_WRITE;
    case QWebEnginePermission::LocalFontsAccess:
        return blink::PermissionType::LOCAL_FONTS;
    case QWebEnginePermission::MediaAudioVideoCapture:
    case QWebEnginePermission::Unsupported:
        LOG(INFO) << "Unexpected unsupported WebEngine permission type: " << static_cast<int>(feature);
        return blink::PermissionType::NUM;
    }

    Q_UNREACHABLE();
}

static QWebEnginePermission::State toQt(blink::mojom::PermissionStatus state)
{
    switch (state) {
    case blink::mojom::PermissionStatus::ASK:
        return QWebEnginePermission::Ask;
    case blink::mojom::PermissionStatus::GRANTED:
        return QWebEnginePermission::Granted;
    case blink::mojom::PermissionStatus::DENIED:
        return QWebEnginePermission::Denied;
    }
}

static blink::mojom::PermissionStatus toBlink(QWebEnginePermission::State state)
{
    switch (state) {
    case QWebEnginePermission::Invalid:
    case QWebEnginePermission::Ask:
        return blink::mojom::PermissionStatus::ASK;
    case QWebEnginePermission::Granted:
        return blink::mojom::PermissionStatus::GRANTED;
    case QWebEnginePermission::Denied:
        return blink::mojom::PermissionStatus::DENIED;
    }
}

std::string featureString(QWebEnginePermission::Feature feature)
{
    // This is separate from blink::featureString() for the sake of future-proofing;
    // e.g. in case we add extra Features that do not correspond to a PermissionType, and
    // we need to store them.
    switch (feature) {
    case QWebEnginePermission::Notifications:
        return "Notifications";
    case QWebEnginePermission::Geolocation:
        return "Geolocation";
    case QWebEnginePermission::ClipboardReadWrite:
        return "ClipboardReadWrite";
    case QWebEnginePermission::LocalFontsAccess:
        return "LocalFontsAccess";
    case QWebEnginePermission::MediaAudioCapture:
        return "MediaAudioCapture";
    case QWebEnginePermission::MediaVideoCapture:
        return "MediaVideoCapture";
    case QWebEnginePermission::DesktopAudioVideoCapture:
        return "DesktopAudioVideoCapture";
    default:
        Q_UNREACHABLE();
        return nullptr;
    }
}

static blink::mojom::PermissionStatus getStatusFromSettings(blink::PermissionType type, WebEngineSettings *settings)
{
    switch (type) {
    case blink::PermissionType::CLIPBOARD_READ_WRITE:
    case blink::PermissionType::CLIPBOARD_SANITIZED_WRITE:
        if (settings->testAttribute(QWebEngineSettings::JavascriptCanPaste)
            && settings->testAttribute(QWebEngineSettings::JavascriptCanAccessClipboard))
            return blink::mojom::PermissionStatus::GRANTED;
        return blink::mojom::PermissionStatus::ASK;
    default:
        return blink::mojom::PermissionStatus::ASK;
    }
}

PermissionManagerQt::PermissionManagerQt(ProfileAdapter *profileAdapter)
    : m_requestIdCount(0)
    , m_persistence(true)
    , m_profileAdapter(profileAdapter)
{
    PrefServiceFactory factory;
    factory.set_async(false);
    factory.set_command_line_prefs(base::MakeRefCounted<ChromeCommandLinePrefStore>(
            base::CommandLine::ForCurrentProcess()));

    QString userPrefStorePath = profileAdapter->dataPath();
    auto prefRegistry = base::MakeRefCounted<PrefRegistrySimple>();

    auto policy = profileAdapter->persistentPermissionsPolicy();
    if (!profileAdapter->isOffTheRecord() && policy == ProfileAdapter::PersistentPermissionsOnDisk &&
            !userPrefStorePath.isEmpty() && profileAdapter->ensureDataPathExists()) {
        userPrefStorePath += QDir::separator();
        userPrefStorePath += QStringLiteral("permissions.json");
        factory.set_user_prefs(base::MakeRefCounted<JsonPrefStore>(toFilePath(userPrefStorePath)));
    } else {
        factory.set_user_prefs(new InMemoryPrefStore);
    }

    m_featureTypes.push_back(QWebEnginePermission::Notifications);
    m_featureTypes.push_back(QWebEnginePermission::Geolocation);
    m_featureTypes.push_back(QWebEnginePermission::ClipboardReadWrite);
    m_featureTypes.push_back(QWebEnginePermission::LocalFontsAccess);

    // Transient, but the implementation relies on them being written to storage
    m_featureTypes.push_back(QWebEnginePermission::MediaAudioCapture);
    m_featureTypes.push_back(QWebEnginePermission::MediaVideoCapture);

    // Register all preference types as keys prior to doing anything else
    for (auto &type : m_featureTypes) {
        prefRegistry->RegisterDictionaryPref(featureString(type));
    }
    PrefProxyConfigTrackerImpl::RegisterPrefs(prefRegistry.get());

    if (policy == ProfileAdapter::NoPersistentPermissions)
        m_persistence = false;

    {
        base::ScopedAllowBlocking allowBlock;
        m_prefService = factory.Create(prefRegistry);
    }
}

PermissionManagerQt::~PermissionManagerQt()
{
    commit();
}

void PermissionManagerQt::setPermission(const QUrl &url, QWebEnginePermission::Feature feature, QWebEnginePermission::State state)
{
    // Normalize the QUrl to Chromium origin form.
    const GURL gorigin = toGurl(url).DeprecatedGetOriginAsURL();
    const QUrl origin = gorigin.is_empty() ? url : toQt(gorigin);
    if (origin.isEmpty())
        return;
    if (state == QWebEnginePermission::Ask)
        ResetPermission(toBlink(feature), gorigin, gorigin);
    else
        setPermission(toBlink(feature), gorigin, state == QWebEnginePermission::Granted);
    blink::mojom::PermissionStatus status = toBlink(state);
    if (state != QWebEnginePermission::Ask) {
        auto it = m_requests.begin();
        while (it != m_requests.end()) {
            if (it->origin == origin && it->type == feature) {
                std::move(it->callback).Run(status);
                it = m_requests.erase(it);
            } else
                ++it;
        }
    }

    for (const auto &it: m_subscribers) {
        if (it.second.origin == origin && it.second.type == feature)
            it.second.callback.Run(status);
    }

    if (state == QWebEnginePermission::Ask)
        return;

    auto it = m_multiRequests.begin();
    while (it != m_multiRequests.end()) {
        if (it->origin == origin) {
            bool answerable = true;
            std::vector<blink::mojom::PermissionStatus> result;
            result.reserve(it->types.size());
            for (blink::PermissionType permission : it->types) {
                if (toQt(permission) == QWebEnginePermission::Unsupported) {
                    result.push_back(blink::mojom::PermissionStatus::DENIED);
                    continue;
                }

                blink::mojom::PermissionStatus permissionStatus = GetPermissionStatus(permission, gorigin, GURL());
                if (permissionStatus == toBlink(state)) {
                    if (permissionStatus == blink::mojom::PermissionStatus::ASK) {
                        answerable = false;
                        break;
                    }

                    result.push_back(permissionStatus);
                } else {
                    // Reached when the PersistentPermissionsPolicy is set to NoPersistentPermissions
                    result.push_back(toBlink(state));
                }
            }
            if (answerable) {
                std::move(it->callback).Run(result);
                it = m_multiRequests.erase(it);
                continue;
            }
        }
        ++it;
    }
}

QWebEnginePermission::State PermissionManagerQt::getPermissionState(const QUrl &origin, QWebEnginePermission::Feature feature)
{
    return toQt(GetPermissionStatus(toBlink(feature), toGurl(origin), GURL()));
}

QList<QWebEnginePermission> PermissionManagerQt::listPermissions(const QUrl &origin, QWebEnginePermission::Feature feature)
{
    Q_ASSERT(origin.isEmpty() || feature == QWebEnginePermission::Unsupported);
    QList<QWebEnginePermission> returnList;
    GURL gorigin = toGurl(origin).DeprecatedGetOriginAsURL();
    std::string originSpec = gorigin.spec();

    if (!origin.isEmpty() && !gorigin.is_valid())
        return returnList;

    std::vector<QWebEnginePermission::Feature> types;
    if (feature == QWebEnginePermission::Unsupported)
        types = m_featureTypes;
    else
        types.push_back(feature);

    for (auto &type : types) {
        // Transient types may end up in the permission store as an implementation detail,
        // but we do not want to expose them to callers.
        if (QWebEnginePermission::isTransient(type))
            continue;

        auto *pref = m_prefService->FindPreference(featureString(type));
        if (!pref)
            continue;

        auto *prefDict = pref->GetValue()->GetIfDict();
        Q_ASSERT(prefDict);

        for (const auto &entry : *prefDict) {
            if (!originSpec.empty() && entry.first != originSpec)
                continue;

            auto *pvt = new QWebEnginePermissionPrivate(toQt(GURL(std::string_view(entry.first))), type, nullptr, m_profileAdapter.get());
            returnList.push_back(QWebEnginePermission(pvt));
        }
    }

    return returnList;
}

void PermissionManagerQt::commit()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    // Make sure modified permissions are written to disk
    m_prefService->CommitPendingWrite();
}

void PermissionManagerQt::RequestPermissions(content::RenderFrameHost *frameHost,
                                             const content::PermissionRequestDescription &requestDescription,
                                             base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus>&)> callback)
{
    if (requestDescription.requesting_origin.is_empty()) {
        std::move(callback).Run(std::vector<content::PermissionStatus>(requestDescription.permissions.size(), blink::mojom::PermissionStatus::DENIED));
        return;
    }

    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(
        content::WebContents::FromRenderFrameHost(frameHost)->GetDelegate());
    Q_ASSERT(contentsDelegate);

    bool answerable = true;
    std::vector<content::PermissionStatus> result;
    result.reserve(requestDescription.permissions.size());
    for (blink::PermissionType permission : requestDescription.permissions) {
        const QWebEnginePermission::Feature feature = toQt(permission);
        if (feature == QWebEnginePermission::Unsupported) {
            result.push_back(blink::mojom::PermissionStatus::DENIED);
            continue;
        }

        blink::mojom::PermissionStatus permissionStatus = getStatusFromSettings(permission, contentsDelegate->webEngineSettings());
        if (permissionStatus == blink::mojom::PermissionStatus::ASK) {
            permissionStatus = GetPermissionStatus(permission, requestDescription.requesting_origin, GURL());
            if (m_persistence && permissionStatus != blink::mojom::PermissionStatus::ASK) {
                // Automatically grant/deny without prompt if already asked once
                result.push_back(permissionStatus);
            } else {
                answerable = false;
                break;
            }
        } else {
            // Reached when clipboard settings have been set
            result.push_back(permissionStatus);
        }
    }

    if (answerable) {
        std::move(callback).Run(result);
        return;
    }

    int request_id = ++m_requestIdCount;
    auto requestOrigin = toQt(requestDescription.requesting_origin);
    m_multiRequests.push_back({ request_id, requestDescription.permissions, requestOrigin, std::move(callback) });
    for (blink::PermissionType permission : requestDescription.permissions) {
        const QWebEnginePermission::Feature feature = toQt(permission);
        if (!QWebEnginePermission::isTransient(feature))
            contentsDelegate->requestFeaturePermission(feature, requestOrigin);
    }
}

void PermissionManagerQt::RequestPermissionsFromCurrentDocument(content::RenderFrameHost *frameHost,
                                                                const content::PermissionRequestDescription &requestDescription,
                                                                base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus>&)> callback)
{
    RequestPermissions(frameHost, requestDescription, std::move(callback));
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatus(
    blink::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/)
{
    const QWebEnginePermission::Feature feature = toQt(permission);
    if (feature == QWebEnginePermission::Unsupported)
        return blink::mojom::PermissionStatus::DENIED;

    permission = toBlink(toQt(permission)); // Filter out merged/unsupported permissions (e.g. clipboard)
    auto *pref = m_prefService->FindPreference(featureString(toQt(permission)));
    if (!pref)
        return blink::mojom::PermissionStatus::ASK; // Permission type not in database

    const auto *permissions = pref->GetValue()->GetIfDict();
    Q_ASSERT(permissions);

    auto requestedPermission = permissions->FindBool(requesting_origin.DeprecatedGetOriginAsURL().spec());
    if (!requestedPermission)
        return blink::mojom::PermissionStatus::ASK; // Origin is not in the current permission type's database

    // Workaround: local fonts are entirely managed by Chromium, which only calls RequestPermission() _after_
    // it's checked whether the permission has been granted. By always returning ASK, we force the request to
    // come through every time.
    if (permission == blink::PermissionType::LOCAL_FONTS
        && m_profileAdapter->persistentPermissionsPolicy() == ProfileAdapter::NoPersistentPermissions)
        return blink::mojom::PermissionStatus::ASK;

    if (requestedPermission.value())
        return blink::mojom::PermissionStatus::GRANTED;
    return blink::mojom::PermissionStatus::DENIED;
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatusForCurrentDocument(
        blink::PermissionType permission,
        content::RenderFrameHost *render_frame_host)
{
    if (permission == blink::PermissionType::CLIPBOARD_READ_WRITE ||
            permission == blink::PermissionType::CLIPBOARD_SANITIZED_WRITE) {
        WebContentsDelegateQt *delegate = static_cast<WebContentsDelegateQt *>(
                content::WebContents::FromRenderFrameHost(render_frame_host)->GetDelegate());
        Q_ASSERT(delegate);
        auto status = getStatusFromSettings(permission, delegate->webEngineSettings());
        if (status != blink::mojom::PermissionStatus::ASK)
            return status;
    }

    return GetPermissionStatus(
                permission,
                render_frame_host->GetLastCommittedOrigin().GetURL(),
                render_frame_host->GetLastCommittedOrigin().GetURL());
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatusForWorker(
        blink::PermissionType permission,
        content::RenderProcessHost *render_process_host,
        const GURL &url)
{
    return GetPermissionStatus(permission, url, url);
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatusForEmbeddedRequester(
        blink::PermissionType permission,
        content::RenderFrameHost *render_frame_host,
        const url::Origin &requesting_origin)
{
    return GetPermissionStatus(permission, requesting_origin.GetURL(),
                               render_frame_host->GetLastCommittedOrigin().GetURL());
}

content::PermissionResult PermissionManagerQt::GetPermissionResultForOriginWithoutContext(
        blink::PermissionType permission,
        const url::Origin &requesting_origin,
        const url::Origin &embedding_origin)
{
    blink::mojom::PermissionStatus status =
            GetPermissionStatus(permission, requesting_origin.GetURL(), embedding_origin.GetURL());

    return content::PermissionResult(status, content::PermissionStatusSource::UNSPECIFIED);
}

void PermissionManagerQt::ResetPermission(
    blink::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/)
{
    const QWebEnginePermission::Feature feature = toQt(permission);
    if (feature == QWebEnginePermission::Unsupported)
        return;

    ScopedDictPrefUpdate updater(m_prefService.get(), featureString(feature));
    updater.Get().Remove(requesting_origin.spec());
}

content::PermissionControllerDelegate::SubscriptionId
PermissionManagerQt::SubscribeToPermissionStatusChange(
        blink::PermissionType permission, content::RenderProcessHost * /*render_process_host*/,
        content::RenderFrameHost * /* render_frame_host */, const GURL &requesting_origin,
        base::RepeatingCallback<void(blink::mojom::PermissionStatus)> callback)
{
    auto subscriber_id = subscription_id_generator_.GenerateNextId();
    m_subscribers.insert( { subscriber_id,
                            Subscription { toQt(permission), toQt(requesting_origin), std::move(callback) } });
    return subscriber_id;
}

void PermissionManagerQt::UnsubscribeFromPermissionStatusChange(
        content::PermissionControllerDelegate::SubscriptionId subscription_id)
{
    if (!m_subscribers.erase(subscription_id))
        LOG(WARNING) << "PermissionManagerQt::UnsubscribePermissionStatusChange called on unknown subscription id" << subscription_id;
}

void PermissionManagerQt::setPermission(
    blink::PermissionType permission,
    const GURL& requesting_origin,
    bool granted)
{
    const QWebEnginePermission::Feature feature = toQt(permission);
    if (feature == QWebEnginePermission::Unsupported)
        return;

    if (!m_prefService->FindPreference(featureString(feature)))
        return;

    ScopedDictPrefUpdate updater(m_prefService.get(), featureString(feature));
    updater.Get().Set(requesting_origin.spec(), granted);
}

} // namespace QtWebEngineCore
