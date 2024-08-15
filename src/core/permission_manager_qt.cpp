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

// Extra permission types that don't exist on the Chromium side.
enum class ExtraPermissionType {
    POINTER_LOCK = 39, // TODO this exists in Chromium 126, remove after we merge
};

static QWebEnginePermission::PermissionType toQt(blink::PermissionType type)
{
    switch (type) {
    case blink::PermissionType::GEOLOCATION:
        return QWebEnginePermission::PermissionType::Geolocation;
    case blink::PermissionType::AUDIO_CAPTURE:
        return QWebEnginePermission::PermissionType::MediaAudioCapture;
    case blink::PermissionType::VIDEO_CAPTURE:
        return QWebEnginePermission::PermissionType::MediaVideoCapture;
    case blink::PermissionType::DISPLAY_CAPTURE:
        return QWebEnginePermission::PermissionType::DesktopAudioVideoCapture;
        // We treat these both as read/write since we do not currently have a
        // ClipboardSanitizedWrite permission type.
    case blink::PermissionType::CLIPBOARD_READ_WRITE:
    case blink::PermissionType::CLIPBOARD_SANITIZED_WRITE:
        return QWebEnginePermission::PermissionType::ClipboardReadWrite;
    case blink::PermissionType::NOTIFICATIONS:
        return QWebEnginePermission::PermissionType::Notifications;
    case blink::PermissionType::LOCAL_FONTS:
        return QWebEnginePermission::PermissionType::LocalFontsAccess;
    case (blink::PermissionType)ExtraPermissionType::POINTER_LOCK:
        return QWebEnginePermission::PermissionType::MouseLock;
    case blink::PermissionType::ACCESSIBILITY_EVENTS:
    case blink::PermissionType::CAMERA_PAN_TILT_ZOOM:
    case blink::PermissionType::WINDOW_MANAGEMENT:
        return QWebEnginePermission::PermissionType::Unsupported;
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
    return QWebEnginePermission::PermissionType::Unsupported;
}

static blink::PermissionType toBlink(QWebEnginePermission::PermissionType permissionType)
{
    switch (permissionType) {
    case QWebEnginePermission::PermissionType::Notifications:
        return blink::PermissionType::NOTIFICATIONS;
    case QWebEnginePermission::PermissionType::Geolocation:
        return blink::PermissionType::GEOLOCATION;
    case QWebEnginePermission::PermissionType::MediaAudioCapture:
        return blink::PermissionType::AUDIO_CAPTURE;
    case QWebEnginePermission::PermissionType::MediaVideoCapture:
        return blink::PermissionType::VIDEO_CAPTURE;
    case QWebEnginePermission::PermissionType::DesktopVideoCapture:
    case QWebEnginePermission::PermissionType::DesktopAudioVideoCapture:
        return blink::PermissionType::DISPLAY_CAPTURE;
    case QWebEnginePermission::PermissionType::ClipboardReadWrite:
        return blink::PermissionType::CLIPBOARD_READ_WRITE;
    case QWebEnginePermission::PermissionType::LocalFontsAccess:
        return blink::PermissionType::LOCAL_FONTS;
    case QWebEnginePermission::PermissionType::MouseLock:
        return (blink::PermissionType)ExtraPermissionType::POINTER_LOCK;
    case QWebEnginePermission::PermissionType::MediaAudioVideoCapture:
    case QWebEnginePermission::PermissionType::Unsupported:
        LOG(INFO) << "Unexpected unsupported WebEngine permission type: " << static_cast<int>(permissionType);
        return blink::PermissionType::NUM;
    }

    Q_UNREACHABLE();
}

static QWebEnginePermission::State toQt(blink::mojom::PermissionStatus state)
{
    switch (state) {
    case blink::mojom::PermissionStatus::ASK:
        return QWebEnginePermission::State::Ask;
    case blink::mojom::PermissionStatus::GRANTED:
        return QWebEnginePermission::State::Granted;
    case blink::mojom::PermissionStatus::DENIED:
        return QWebEnginePermission::State::Denied;
    }
}

static blink::mojom::PermissionStatus toBlink(QWebEnginePermission::State state)
{
    switch (state) {
    case QWebEnginePermission::State::Invalid:
    case QWebEnginePermission::State::Ask:
        return blink::mojom::PermissionStatus::ASK;
    case QWebEnginePermission::State::Granted:
        return blink::mojom::PermissionStatus::GRANTED;
    case QWebEnginePermission::State::Denied:
        return blink::mojom::PermissionStatus::DENIED;
    }
}

std::string permissionTypeString(QWebEnginePermission::PermissionType permissionType)
{
    // This is separate from blink::permissionTypeString() for the sake of future-proofing;
    // e.g. in case we add extra Features that do not correspond to a PermissionType, and
    // we need to store them.
    switch (permissionType) {
    case QWebEnginePermission::PermissionType::MediaAudioCapture:
        return "MediaAudioCapture";
    case QWebEnginePermission::PermissionType::MediaVideoCapture:
        return "MediaVideoCapture";
    case QWebEnginePermission::PermissionType::DesktopAudioVideoCapture:
        return "DesktopAudioVideoCapture";
    case QWebEnginePermission::PermissionType::MouseLock:
        return "MouseLock";
    case QWebEnginePermission::PermissionType::Notifications:
        return "Notifications";
    case QWebEnginePermission::PermissionType::Geolocation:
        return "Geolocation";
    case QWebEnginePermission::PermissionType::ClipboardReadWrite:
        return "ClipboardReadWrite";
    case QWebEnginePermission::PermissionType::LocalFontsAccess:
        return "LocalFontsAccess";
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
    , m_transientWriteCount(0)
    , m_profileAdapter(profileAdapter)
    , m_persistence(true)
{
    PrefServiceFactory factory;
    factory.set_async(false);
    factory.set_command_line_prefs(base::MakeRefCounted<ChromeCommandLinePrefStore>(
            base::CommandLine::ForCurrentProcess()));

    QString userPrefStorePath = profileAdapter->dataPath();
    auto prefRegistry = base::MakeRefCounted<PrefRegistrySimple>();

    auto policy = profileAdapter->persistentPermissionsPolicy();
    if (!profileAdapter->isOffTheRecord() && policy == ProfileAdapter::PersistentPermissionsPolicy::StoreOnDisk &&
            !userPrefStorePath.isEmpty() && profileAdapter->ensureDataPathExists()) {
        userPrefStorePath += QDir::separator();
        userPrefStorePath += QStringLiteral("permissions.json");
        factory.set_user_prefs(base::MakeRefCounted<JsonPrefStore>(toFilePath(userPrefStorePath)));
    } else {
        factory.set_user_prefs(new InMemoryPrefStore);
    }

    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::MediaAudioCapture);
    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::MediaVideoCapture);
    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::MouseLock);
    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::Notifications);
    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::Geolocation);
    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::ClipboardReadWrite);
    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::LocalFontsAccess);

    // Register all preference types as keys prior to doing anything else
    for (auto &type : m_permissionTypes) {
        prefRegistry->RegisterDictionaryPref(permissionTypeString(type));
    }
    PrefProxyConfigTrackerImpl::RegisterPrefs(prefRegistry.get());

    if (policy == ProfileAdapter::PersistentPermissionsPolicy::AskEveryTime)
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

void PermissionManagerQt::setPermission(
    const QUrl &url,
    QWebEnginePermission::PermissionType permissionType,
    QWebEnginePermission::State state,
    content::RenderFrameHost *rfh)
{
    // Normalize the QUrl to Chromium origin form.
    const GURL gorigin = toGurl(url).DeprecatedGetOriginAsURL();
    const QUrl origin = gorigin.is_empty() ? url : toQt(gorigin);
    if (origin.isEmpty())
        return;

    // Send eligible permissions with an associated rfh to the transient store. When pre-granting
    // a non-persistent permission (or pre-granting any permission in AskEveryTime mode), it is allowed
    // to pass through the persistent store. It will be moved to the transient store and associated
    // with a rfh the next time its status is requested.
    bool inTransientStore = rfh && (!QWebEnginePermission::isPersistent(permissionType) || !m_persistence);

    blink::mojom::PermissionStatus blinkStatus = toBlink(state);
    if (state == QWebEnginePermission::State::Ask) {
        if (inTransientStore)
            resetTransientPermission(toBlink(permissionType), gorigin, rfh->GetGlobalFrameToken());
        else
            ResetPermission(toBlink(permissionType), gorigin, gorigin);
    } else {
        if (inTransientStore)
            setTransientPermission(toBlink(permissionType), gorigin, state == QWebEnginePermission::State::Granted, rfh->GetGlobalFrameToken());
        else
            setPersistentPermission(toBlink(permissionType), gorigin, state == QWebEnginePermission::State::Granted);
        auto it = m_requests.begin();
        while (it != m_requests.end()) {
            if (it->origin == origin && it->type == permissionType) {
                std::move(it->callback).Run(blinkStatus);
                it = m_requests.erase(it);
            } else
                ++it;
        }
    }

    for (const auto &it : m_subscribers) {
        if (it.second.origin == origin && it.second.type == permissionType)
            it.second.callback.Run(blinkStatus);
    }

    if (state == QWebEnginePermission::State::Ask)
        return;

    auto it = m_multiRequests.begin();
    while (it != m_multiRequests.end()) {
        if (it->origin == origin) {
            bool answerable = true;
            std::vector<blink::mojom::PermissionStatus> result;
            result.reserve(it->types.size());
            for (blink::PermissionType permission : it->types) {
                if (toQt(permission) == QWebEnginePermission::PermissionType::Unsupported) {
                    result.push_back(blink::mojom::PermissionStatus::DENIED);
                    continue;
                }

                blink::mojom::PermissionStatus permissionStatus;
                if (inTransientStore)
                    permissionStatus = toBlink(getPermissionState(url, permissionType, rfh));
                else
                    permissionStatus = GetPermissionStatus(permission, gorigin, GURL());

                if (permissionStatus == toBlink(state)) {
                    if (permissionStatus == blink::mojom::PermissionStatus::ASK) {
                        answerable = false;
                        break;
                    }

                    result.push_back(permissionStatus);
                } else {
                    // Reached when the PersistentPermissionsPolicy is set to AskEveryTime
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

QWebEnginePermission::State PermissionManagerQt::getPermissionState(const QUrl &origin, QWebEnginePermission::PermissionType permissionType,
    content::RenderFrameHost *rfh)
{
    if (rfh) {
        // Ignore the origin parameter
        return toQt(GetPermissionStatusForCurrentDocument(toBlink(permissionType), rfh));
    }

    return toQt(GetPermissionStatus(toBlink(permissionType), toGurl(origin), GURL()));
}

QList<QWebEnginePermission> PermissionManagerQt::listPermissions(const QUrl &origin, QWebEnginePermission::PermissionType permissionType)
{
    Q_ASSERT(origin.isEmpty() || permissionType == QWebEnginePermission::PermissionType::Unsupported);
    QList<QWebEnginePermission> returnList;
    GURL gorigin = toGurl(origin).DeprecatedGetOriginAsURL();
    std::string originSpec = gorigin.spec();

    if (!origin.isEmpty() && !gorigin.is_valid())
        return returnList;

    std::vector<QWebEnginePermission::PermissionType> types;
    if (permissionType == QWebEnginePermission::PermissionType::Unsupported)
        types = m_permissionTypes;
    else
        types.push_back(permissionType);

    for (auto &type : types) {
        // Transient types may end up in the permission store as an implementation detail,
        // but we do not want to expose them to callers.
        if (!QWebEnginePermission::isPersistent(type))
            continue;

        auto *pref = m_prefService->FindPreference(permissionTypeString(type));
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
        const QWebEnginePermission::PermissionType permissionType = toQt(permission);
        if (permissionType == QWebEnginePermission::PermissionType::Unsupported) {
            result.push_back(blink::mojom::PermissionStatus::DENIED);
            continue;
        }

        blink::mojom::PermissionStatus permissionStatus = getStatusFromSettings(permission, contentsDelegate->webEngineSettings());
        if (permissionStatus == blink::mojom::PermissionStatus::ASK) {
            const GURL &rorigin = requestDescription.requesting_origin;

            if (!m_persistence) {
                answerable = false;
                break;
            }

            bool inTransientStore = !QWebEnginePermission::isPersistent(toQt(permission));
            if (inTransientStore) {
                permissionStatus = getTransientPermissionStatus(permission, rorigin, frameHost->GetGlobalFrameToken());

                if (permissionStatus != blink::mojom::PermissionStatus::ASK) {
                    result.push_back(permissionStatus);
                    continue;
                }

                // Fall through to check if permission was pre-granted (and thus landed in the permanent store)
            }

            permissionStatus = GetPermissionStatus(permission, rorigin, rorigin);

            if (inTransientStore && permissionStatus != blink::mojom::PermissionStatus::ASK) {
                // Move the pre-granted permission to the transient store and associate it with the rfh
                ResetPermission(permission, rorigin, rorigin);
                setTransientPermission(permission, rorigin, permissionStatus == blink::mojom::PermissionStatus::GRANTED,
                    frameHost->GetGlobalFrameToken());
            }

            if (permissionStatus != blink::mojom::PermissionStatus::ASK) {
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
        const QWebEnginePermission::PermissionType permissionType = toQt(permission);
        if (QWebEnginePermission::isPersistent(permissionType))
            contentsDelegate->requestFeaturePermission(permissionType, requestOrigin);
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
    const QWebEnginePermission::PermissionType permissionType = toQt(permission);
    if (permissionType == QWebEnginePermission::PermissionType::Unsupported)
        return blink::mojom::PermissionStatus::DENIED;

    permission = toBlink(toQt(permission)); // Filter out merged/unsupported permissions (e.g. clipboard)
    auto *pref = m_prefService->FindPreference(permissionTypeString(toQt(permission)));
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
    if (permission == blink::PermissionType::LOCAL_FONTS && !m_persistence)
        return blink::mojom::PermissionStatus::ASK;

    if (requestedPermission.value())
        return blink::mojom::PermissionStatus::GRANTED;
    return blink::mojom::PermissionStatus::DENIED;
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatusForCurrentDocument(
        blink::PermissionType permission,
        content::RenderFrameHost *render_frame_host)
{
    Q_ASSERT(render_frame_host);

    if (permission == blink::PermissionType::CLIPBOARD_READ_WRITE ||
            permission == blink::PermissionType::CLIPBOARD_SANITIZED_WRITE) {
        WebContentsDelegateQt *delegate = static_cast<WebContentsDelegateQt *>(
                content::WebContents::FromRenderFrameHost(render_frame_host)->GetDelegate());
        Q_ASSERT(delegate);
        auto status = getStatusFromSettings(permission, delegate->webEngineSettings());
        if (status != blink::mojom::PermissionStatus::ASK)
            return status;
    }

    permission = toBlink(toQt(permission)); // Filter out merged/unsupported permissions (e.g. clipboard)
    if (toQt(permission) == QWebEnginePermission::PermissionType::Unsupported)
        return blink::mojom::PermissionStatus::DENIED;

    GURL origin = render_frame_host->GetLastCommittedOrigin().GetURL();
    auto status = blink::mojom::PermissionStatus::ASK;

    bool inTransientStore = !QWebEnginePermission::isPersistent(toQt(permission)) || !m_persistence;
    if (inTransientStore) {
        status = getTransientPermissionStatus(permission, origin, render_frame_host->GetGlobalFrameToken());

        if (status != blink::mojom::PermissionStatus::ASK) {
            return status;
        }

        // Fall through to check if permission was pre-granted (and thus landed in the permanent store)
    }

    status = GetPermissionStatus(permission, origin, origin);

    if (inTransientStore && status != blink::mojom::PermissionStatus::ASK) {
        // Move the pre-granted permission to the transient store and associate it with the rfh
        ResetPermission(permission, origin, origin);
        setTransientPermission(permission, origin, status == blink::mojom::PermissionStatus::GRANTED,
            render_frame_host->GetGlobalFrameToken());
    }

    return status;
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatusForWorker(
        blink::PermissionType permission,
        content::RenderProcessHost *render_process_host,
        const GURL &url)
{
    Q_UNUSED(render_process_host);
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
    const QWebEnginePermission::PermissionType permissionType = toQt(permission);
    if (permissionType == QWebEnginePermission::PermissionType::Unsupported)
        return;

    ScopedDictPrefUpdate updater(m_prefService.get(), permissionTypeString(permissionType));
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

blink::mojom::PermissionStatus PermissionManagerQt::getTransientPermissionStatus(blink::PermissionType permission,
        const GURL& requesting_origin,
        content::GlobalRenderFrameHostToken token)
{
    const QWebEnginePermission::PermissionType permissionType = toQt(permission);
    if (permissionType == QWebEnginePermission::PermissionType::Unsupported)
        return blink::mojom::PermissionStatus::DENIED;

    if (!m_transientPermissions.contains(token))
        return blink::mojom::PermissionStatus::ASK;

    auto &permissionsForToken = m_transientPermissions[token];
    for (auto p = permissionsForToken.begin(); p != permissionsForToken.end(); ++p) {
        if (get<0>(*p) == requesting_origin && get<1>(*p) == permission) {
            return get<2>(*p) ? blink::mojom::PermissionStatus::GRANTED : blink::mojom::PermissionStatus::DENIED;
        }
    }

    return blink::mojom::PermissionStatus::ASK;
}

void PermissionManagerQt::setPersistentPermission(
    blink::PermissionType permission,
    const GURL& requesting_origin,
    bool granted)
{
    const QWebEnginePermission::PermissionType permissionType = toQt(permission);
    if (permissionType == QWebEnginePermission::PermissionType::Unsupported)
        return;

    if (!m_prefService->FindPreference(permissionTypeString(permissionType)))
        return;

    ScopedDictPrefUpdate updater(m_prefService.get(), permissionTypeString(permissionType));
    updater.Get().Set(requesting_origin.spec(), granted);

    m_prefService->SchedulePendingLossyWrites();
}

void PermissionManagerQt::setTransientPermission(blink::PermissionType permission,
        const GURL& requesting_origin,
        bool granted,
        content::GlobalRenderFrameHostToken token)
{
    const QWebEnginePermission::PermissionType permissionType = toQt(permission);
    if (permissionType == QWebEnginePermission::PermissionType::Unsupported)
        return;

    auto &permissionsForToken = m_transientPermissions[token];
    for (auto &p : permissionsForToken) {
        if (get<0>(p) == requesting_origin && get<1>(p) == permission) {
            get<2>(p) = granted;
            return;
        }
    }

    permissionsForToken.push_back({requesting_origin, permission, granted});

    // Render frame hosts get discarded often, so the map will eventualy fill up with junk unless
    // periodically cleaned. The number 25 was chosen arbitrarily.
    if (++m_transientWriteCount > 25) {
        content::GetUIThreadTaskRunner({})->PostTask(FROM_HERE,
            base::BindOnce([](PermissionManagerQt *p){
                for (auto i = p->m_transientPermissions.begin(); i != p->m_transientPermissions.end(); ++i) {
                    if (content::RenderFrameHost::FromFrameToken(i->first) == nullptr) {
                        i = p->m_transientPermissions.erase(i);
                    }
                }
            }, this));
        m_transientWriteCount = 0;
    }
}

void PermissionManagerQt::resetTransientPermission(blink::PermissionType permission,
        const GURL& requesting_origin,
        content::GlobalRenderFrameHostToken token)
{
    const QWebEnginePermission::PermissionType permissionType = toQt(permission);
    if (permissionType == QWebEnginePermission::PermissionType::Unsupported)
        return;

    auto &permissionsForToken = m_transientPermissions[token];
    for (auto i = permissionsForToken.begin(); i != permissionsForToken.end(); ++i) {
        if (get<0>(*i) == requesting_origin && get<1>(*i) == permission) {
            permissionsForToken.erase(i);
            return;
        }
    }
}

} // namespace QtWebEngineCore
