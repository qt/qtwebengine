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

using namespace Qt::StringLiterals;

namespace QtWebEngineCore {

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
        return QWebEnginePermission::PermissionType::DesktopVideoCapture;
        // We treat these both as read/write since we do not currently have a
        // ClipboardSanitizedWrite permission type.
    case blink::PermissionType::CLIPBOARD_READ_WRITE:
    case blink::PermissionType::CLIPBOARD_SANITIZED_WRITE:
        return QWebEnginePermission::PermissionType::ClipboardReadWrite;
    case blink::PermissionType::NOTIFICATIONS:
        return QWebEnginePermission::PermissionType::Notifications;
    case blink::PermissionType::LOCAL_FONTS:
        return QWebEnginePermission::PermissionType::LocalFontsAccess;
    case blink::PermissionType::POINTER_LOCK:
        return QWebEnginePermission::PermissionType::MouseLock;
    case blink::PermissionType::CAMERA_PAN_TILT_ZOOM:
    case blink::PermissionType::WINDOW_MANAGEMENT:
    case blink::PermissionType::BACKGROUND_SYNC:
    case blink::PermissionType::NUM:
    case blink::PermissionType::TOP_LEVEL_STORAGE_ACCESS:
    case blink::PermissionType::SPEAKER_SELECTION:
        return QWebEnginePermission::PermissionType::Unsupported;
    case blink::PermissionType::MIDI_SYSEX:
    case blink::PermissionType::PROTECTED_MEDIA_IDENTIFIER:
    case blink::PermissionType::MIDI:
    case blink::PermissionType::DURABLE_STORAGE:
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
    case blink::PermissionType::CAPTURED_SURFACE_CONTROL:
    case blink::PermissionType::SMART_CARD:
    case blink::PermissionType::WEB_PRINTING:
    case blink::PermissionType::KEYBOARD_LOCK:
    case blink::PermissionType::AUTOMATIC_FULLSCREEN:
    case blink::PermissionType::HAND_TRACKING:
    case blink::PermissionType::WEB_APP_INSTALLATION:
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
        return blink::PermissionType::POINTER_LOCK;
    case QWebEnginePermission::PermissionType::Unsupported:
        return blink::PermissionType::NUM;
    case QWebEnginePermission::PermissionType::MediaAudioVideoCapture:
        break;
    }

    Q_UNREACHABLE_RETURN(blink::PermissionType::NUM);
}

static std::vector<QWebEnginePermission::PermissionType> toQt(
        const std::vector<blink::PermissionType> &blinkPermissions)
{
    // This function handles the edge case differences between our permission types and Blink's;
    // namely, MediaAudioVideoCapture and DesktopAudioVideoCapture
    std::vector<QWebEnginePermission::PermissionType> permissions;
    for (auto &p : blinkPermissions) {
        permissions.push_back(toQt(p));
    }

    for (auto i1 = permissions.begin(); i1 != permissions.end(); ++i1) {
        if (*i1 == QWebEnginePermission::PermissionType::MediaAudioCapture) {
            for (auto i2 = permissions.begin(); i2 != permissions.end(); ++i2) {
                if (*i2 == QWebEnginePermission::PermissionType::MediaVideoCapture) {
                    // Merge MediaAudioCapture and MediaVideoCapture into MediaAudioVideoCapture
                    *i1 = QWebEnginePermission::PermissionType::MediaAudioVideoCapture;
                    permissions.erase(i2);
                    break;
                }
            }
        } else if (*i1 == QWebEnginePermission::PermissionType::DesktopVideoCapture) {
            for (auto i2 = i1 + 1; i2 != permissions.end(); ++i2) {
                if (*i2 == QWebEnginePermission::PermissionType::DesktopVideoCapture) {
                    // Double DesktopVideoCapture means we actually need DesktopAudioVideoCapture
                    *i2 = QWebEnginePermission::PermissionType::DesktopAudioVideoCapture;
                    i1 = permissions.erase(i1);
                    break;
                }
            }
        }
    }

    return permissions;
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
    case QWebEnginePermission::PermissionType::DesktopVideoCapture:
        return "DesktopVideoCapture";
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
        Q_UNREACHABLE_RETURN(nullptr);
    }
}

static blink::mojom::PermissionStatus getStatusFromSettings(blink::PermissionType type, WebEngineSettings *settings)
{
    switch (type) {
    case blink::PermissionType::CLIPBOARD_READ_WRITE:
        if (settings->testAttribute(QWebEngineSettings::JavascriptCanPaste)
            && settings->testAttribute(QWebEngineSettings::JavascriptCanAccessClipboard))
            return blink::mojom::PermissionStatus::GRANTED;
        return blink::mojom::PermissionStatus::ASK;
    case blink::PermissionType::CLIPBOARD_SANITIZED_WRITE:
        if (settings->testAttribute(QWebEngineSettings::JavascriptCanAccessClipboard))
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

    QString userPrefStorePath;
    userPrefStorePath += profileAdapter->dataPath();
    auto prefRegistry = base::MakeRefCounted<PrefRegistrySimple>();

    auto policy = profileAdapter->persistentPermissionsPolicy();
    if (!profileAdapter->isOffTheRecord() && policy == ProfileAdapter::PersistentPermissionsPolicy::StoreOnDisk &&
            !userPrefStorePath.isEmpty() && profileAdapter->ensureDataPathExists()) {
        userPrefStorePath += QDir::separator();
        userPrefStorePath += "permissions.json"_L1;
        factory.set_user_prefs(base::MakeRefCounted<JsonPrefStore>(toFilePath(userPrefStorePath)));
    } else {
        factory.set_user_prefs(new InMemoryPrefStore);
    }

    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::MediaAudioCapture);
    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::MediaVideoCapture);
    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::DesktopAudioVideoCapture);
    m_permissionTypes.push_back(QWebEnginePermission::PermissionType::DesktopVideoCapture);
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

// static
content::GlobalRenderFrameHostToken PermissionManagerQt::deserializeToken(int childId, const std::string &serializedToken)
{
    auto maybeToken = base::UnguessableToken::DeserializeFromString(serializedToken);
    if (maybeToken)
        return content::GlobalRenderFrameHostToken(childId, blink::LocalFrameToken(maybeToken.value()));

    return content::GlobalRenderFrameHostToken();
}

void PermissionManagerQt::setPermission(
    const QUrl &url,
    const QWebEnginePermission::PermissionType permissionType,
    const QWebEnginePermission::State state,
    const content::GlobalRenderFrameHostToken &token)
{
    if (permissionType == QWebEnginePermission::PermissionType::MediaAudioVideoCapture) {
        setPermissionImpl(url, QWebEnginePermission::PermissionType::MediaAudioCapture, state, token);
        setPermissionImpl(url, QWebEnginePermission::PermissionType::MediaVideoCapture, state, token);
        return;
    }

    setPermissionImpl(url, permissionType, state, token);
}

void PermissionManagerQt::setPermission(
    const QUrl &url,
    const QWebEnginePermission::PermissionType permissionType,
    const QWebEnginePermission::State state,
    int childId, const std::string &serializedToken)
{
    content::GlobalRenderFrameHostToken token;
    auto maybeToken = base::UnguessableToken::DeserializeFromString(serializedToken);
    if (maybeToken)
        token = content::GlobalRenderFrameHostToken(childId, blink::LocalFrameToken(maybeToken.value()));

    setPermission(url, permissionType, state, token);
}

void PermissionManagerQt::setPermissionImpl(
        const QUrl &url,
        const QWebEnginePermission::PermissionType permissionTypeQt,
        const QWebEnginePermission::State permissionStateQt,
        const content::GlobalRenderFrameHostToken &frameToken)
{
    const blink::PermissionType permissionTypeBlink = toBlink(permissionTypeQt);
    const blink::mojom::PermissionStatus permissionStateBlink = toBlink(permissionStateQt);

    // Normalize the QUrl to Chromium origin form.
    const GURL gorigin = toGurl(url).DeprecatedGetOriginAsURL();
    const QUrl origin = gorigin.is_empty() ? url : toQt(gorigin);
    if (origin.isEmpty())
        return;

    // Send eligible permissions with an associated frameToken to the transient store. When pre-granting
    // a non-persistent permission (or pre-granting any permission in AskEveryTime mode), it is allowed
    // to pass through the persistent store. It will be moved to the transient store and associated
    // with a frameToken the next time its status is requested.
    bool inTransientStore = frameToken.child_id != content::kInvalidChildProcessUniqueId
        && (!QWebEnginePermission::isPersistent(permissionTypeQt) || !m_persistence);

    blink::mojom::PermissionStatus blinkStatus = permissionStateBlink;
    if (permissionStateQt == QWebEnginePermission::State::Ask) {
        if (inTransientStore)
            resetTransientPermission(permissionTypeBlink, gorigin, frameToken);
        else
            ResetPermission(permissionTypeBlink, gorigin, gorigin);
    } else {
        if (inTransientStore)
            setTransientPermission(permissionTypeBlink, gorigin,
                permissionStateQt == QWebEnginePermission::State::Granted, frameToken);
        else
            setPersistentPermission(permissionTypeBlink,
                gorigin, permissionStateQt == QWebEnginePermission::State::Granted);
        auto it = m_requests.begin();
        while (it != m_requests.end()) {
            if (it->origin == origin && it->type == permissionTypeQt) {
                std::move(it->callback).Run(blinkStatus);
                it = m_requests.erase(it);
            } else
                ++it;
        }
    }

    // Notify subscribers
    if (subscriptions()) {
        std::vector<base::OnceClosure> callbacks;
        callbacks.reserve(subscriptions()->size());
        for (content::PermissionController::SubscriptionsMap::iterator iter(subscriptions());
             !iter.IsAtEnd(); iter.Advance()) {
            content::PermissionStatusSubscription *subscription = iter.GetCurrentValue();
            if (!subscription)
                continue;
            content::RenderFrameHost *targetRfh = content::RenderFrameHost::FromID(
                    subscription->render_process_id, subscription->render_frame_id);

            if (subscription->embedding_origin != gorigin)
                continue;
            if (subscription->permission != permissionTypeBlink)
                continue;
            if ((!QWebEnginePermission::isPersistent(permissionTypeQt) || !m_persistence)
                    && targetRfh && targetRfh != content::RenderFrameHost::FromFrameToken(frameToken))
                continue;

            // Behavior in callbacks may differ depending on the denial reason. Until we have
            // a good reason to not do so, we just pass UNSPECIFIED to get the default behavior everywhere.
            content::PermissionResult new_value(blinkStatus, content::PermissionStatusSource::UNSPECIFIED);
            if (subscription->permission_result && subscription->permission_result->status == new_value.status)
                continue;
            subscription->permission_result = new_value;

            callbacks.push_back(base::BindOnce(subscription->callback, blinkStatus,
                                               /*ignore_status_override=*/false));
        }
        for (auto &callback : callbacks)
            std::move(callback).Run();
    }

    if (permissionStateQt == QWebEnginePermission::State::Ask)
        return;

    auto it = m_multiRequests.begin();
    while (it != m_multiRequests.end()) {
        if (it->origin == origin) {
            bool answerable = true;
            std::vector<blink::mojom::PermissionStatus> result;
            result.reserve(it->types.size());
            for (blink::PermissionType currentPermissionType : it->types) {
                if (toQt(currentPermissionType) == QWebEnginePermission::PermissionType::Unsupported) {
                    result.push_back(blink::mojom::PermissionStatus::DENIED);
                    continue;
                }

                blink::mojom::PermissionStatus permissionStatus;
                if (inTransientStore)
                    permissionStatus = toBlink(getPermissionState(url, toQt(currentPermissionType), frameToken));
                else
                    permissionStatus = GetPermissionStatus(currentPermissionType, gorigin, GURL());

                if (permissionStatus == permissionStateBlink) {
                    if (permissionStatus == blink::mojom::PermissionStatus::ASK) {
                        answerable = false;
                        break;
                    }

                    result.push_back(permissionStatus);
                } else if (!m_persistence) {
                    // Reached when the PersistentPermissionsPolicy is set to AskEveryTime
                    result.push_back(permissionStateBlink);
                } else {
                    // Not all of the permissions in this request have been set yet, bail and wait for the next setPermission() call
                    answerable = false;
                    break;
                }
            }
            if (answerable) {
                if (!it->callback.is_null())
                    std::move(it->callback).Run(result);
                it = m_multiRequests.erase(it);
                continue;
            }
        }
        ++it;
    }
}

QWebEnginePermission::State PermissionManagerQt::getPermissionState(
        const QUrl &origin,
        const QWebEnginePermission::PermissionType permissionType,
        const content::GlobalRenderFrameHostToken &frameToken)
{
    std::vector<QWebEnginePermission::PermissionType> types;
    if (permissionType == QWebEnginePermission::PermissionType::MediaAudioVideoCapture) {
        types.push_back(QWebEnginePermission::PermissionType::MediaAudioCapture);
        types.push_back(QWebEnginePermission::PermissionType::MediaVideoCapture);
    } else {
        types.push_back(permissionType);
    }

    auto *rfh = content::RenderFrameHost::FromFrameToken(frameToken);
    QWebEnginePermission::State returnState = QWebEnginePermission::State::Invalid;
    for (auto type : types) {
        QWebEnginePermission::State state = rfh
            ? toQt(GetPermissionStatusForCurrentDocument(toBlink(type), rfh, false))
            : toQt(GetPermissionStatus(toBlink(type), toGurl(origin), GURL()));

        if (returnState == QWebEnginePermission::State::Invalid)
            returnState = state;
        else if (returnState != state)
            returnState = QWebEnginePermission::State::Ask;
    }

    return returnState;
}

QList<QWebEnginePermission> PermissionManagerQt::listPermissions(
        const QUrl &origin,
        const QWebEnginePermission::PermissionType permissionType)
{
    Q_ASSERT(origin.isEmpty() || permissionType == QWebEnginePermission::PermissionType::Unsupported);

    QList<QWebEnginePermission> returnList;
    const GURL gorigin = toGurl(origin).DeprecatedGetOriginAsURL();
    const std::string originSpec = gorigin.spec();

    if (!origin.isEmpty() && !gorigin.is_valid())
        return returnList;

    std::vector<QWebEnginePermission::PermissionType> types;
    if (permissionType == QWebEnginePermission::PermissionType::Unsupported)
        types = m_permissionTypes;
    else
        types.push_back(permissionType);

    for (const auto &type : types) {
        // Transient types may end up in the permission store as an implementation detail,
        // but we do not want to expose them to callers.
        if (!QWebEnginePermission::isPersistent(type))
            continue;

        auto *pref = m_prefService->FindPreference(permissionTypeString(type));
        if (!pref)
            continue;

        auto *prefDict = pref->GetValue()->GetIfDict();
        Q_ASSERT(prefDict);

        for (auto &&entry : *prefDict) {
            if (!originSpec.empty() && entry.first != originSpec)
                continue;

            auto *pvt = new QWebEnginePermissionPrivate(
                toQt(GURL(std::string_view(entry.first))), type, m_profileAdapter.get());
            returnList.push_back(QWebEnginePermission(pvt));
        }
    }

    return returnList;
}

void PermissionManagerQt::requestMediaPermissions(
        content::RenderFrameHost *render_frame_host,
        const WebContentsAdapterClient::MediaRequestFlags flags,
        base::OnceCallback<void(WebContentsAdapterClient::MediaRequestFlags authorizationFlags)> callback)
{
    std::vector<blink::PermissionType> permissionTypesBlink;
    if (flags.testFlag(WebContentsAdapterClient::MediaAudioCapture))
        permissionTypesBlink.push_back(blink::PermissionType::AUDIO_CAPTURE);
    if (flags.testFlag(WebContentsAdapterClient::MediaVideoCapture))
        permissionTypesBlink.push_back(blink::PermissionType::VIDEO_CAPTURE);
    if (flags.testFlag(WebContentsAdapterClient::MediaDesktopAudioCapture)
            || flags.testFlag(WebContentsAdapterClient::MediaDesktopVideoCapture)) {
        permissionTypesBlink.push_back(blink::PermissionType::DISPLAY_CAPTURE);
        if (flags.testFlag(WebContentsAdapterClient::MediaDesktopAudioCapture)) {
            // Inject a second copy of the permission type into the request,
            // so we can distinguish between DesktopVideoCapture and DesktopAudioVideoCapture.
            permissionTypesBlink.push_back(blink::PermissionType::DISPLAY_CAPTURE);
        }
    }

    content::PermissionRequestDescription description(permissionTypesBlink, false, render_frame_host->GetLastCommittedOrigin().GetURL());

    RequestPermissions(render_frame_host, description, base::BindOnce([](
                std::vector<blink::PermissionType> permissionTypesBlink,
                base::OnceCallback<void(WebContentsAdapterClient::MediaRequestFlags authorizationFlags)> callback,
                const std::vector<blink::mojom::PermissionStatus> &statuses)
    {
        // This callback converts the Blink permission types to MediaRequestFlags,
        // and then runs the callback initially passed to requestMediaPermissions().
        DCHECK(permissionTypesBlink.size() == statuses.size());
        WebContentsAdapterClient::MediaRequestFlags flags = WebContentsAdapterClient::MediaRequestFlag::MediaNone;
        for (uint i = 0; i < statuses.size(); ++i) {
            if (statuses[i] == blink::mojom::PermissionStatus::GRANTED) {
                switch (permissionTypesBlink[i]) {
                case blink::PermissionType::AUDIO_CAPTURE:
                    flags.setFlag(WebContentsAdapterClient::MediaRequestFlag::MediaAudioCapture);
                    break;
                case blink::PermissionType::VIDEO_CAPTURE:
                    flags.setFlag(WebContentsAdapterClient::MediaRequestFlag::MediaVideoCapture);
                    break;
                case blink::PermissionType::DISPLAY_CAPTURE:
                    flags.setFlag(WebContentsAdapterClient::MediaRequestFlag::MediaDesktopAudioCapture);
                    flags.setFlag(WebContentsAdapterClient::MediaRequestFlag::MediaDesktopVideoCapture);
                    break;
                default:
                    Q_UNREACHABLE();
                    break;
                }
            }
        }
        std::move(callback).Run(flags);
    }, permissionTypesBlink, std::move(callback)));
}

// Needed for the rare cases where a RenderFrameHost remains the same even after
// a cross-origin navigation (e.g. inside an iframe). Needs to be called every
// time transient permissions are accessed.
void PermissionManagerQt::onCrossOriginNavigation(content::RenderFrameHost *render_frame_host)
{
    if (!render_frame_host)
        return;

    auto frameToken = render_frame_host->GetGlobalFrameToken();
    auto &permissionsForToken = m_transientPermissions[frameToken];
    if (!permissionsForToken.size())
        return;

    GURL savedOrigin = get<0>(permissionsForToken[0]);
    if (render_frame_host->GetLastCommittedOrigin().GetURL() != savedOrigin)
        m_transientPermissions.erase(frameToken);
}

void PermissionManagerQt::commit()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    // Make sure modified permissions are written to disk
    m_prefService->CommitPendingWrite();
}

void PermissionManagerQt::RequestPermissions(
        content::RenderFrameHost *frameHost,
        const content::PermissionRequestDescription &requestDescription,
        base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus>&)> callback)
{
    if (requestDescription.requesting_origin.is_empty()) {
        std::move(callback).Run(std::vector<content::PermissionStatus>(requestDescription.permissions.size(),
            blink::mojom::PermissionStatus::DENIED));
        return;
    }

    const auto frameToken = frameHost->GetGlobalFrameToken();
    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(
        content::WebContents::FromRenderFrameHost(frameHost)->GetDelegate());
    Q_ASSERT(contentsDelegate);

    bool answerable = true;
    std::vector<content::PermissionStatus> result;
    result.reserve(requestDescription.permissions.size());
    for (const blink::PermissionType permissionTypeBlink : requestDescription.permissions) {
        const QWebEnginePermission::PermissionType permissionTypeQt = toQt(permissionTypeBlink);
        if (permissionTypeQt == QWebEnginePermission::PermissionType::Unsupported) {
            result.push_back(blink::mojom::PermissionStatus::DENIED);
            continue;
        }

        blink::mojom::PermissionStatus permissionStatusBlink = getStatusFromSettings(
            permissionTypeBlink, contentsDelegate->webEngineSettings());
        if (permissionStatusBlink == blink::mojom::PermissionStatus::ASK) {
            const GURL &rorigin = requestDescription.requesting_origin;
            bool maybePreGranted = false;

            if (!m_persistence) {
                maybePreGranted = true;
            }

            bool inTransientStore = !QWebEnginePermission::isPersistent(permissionTypeQt) || maybePreGranted;
            if (inTransientStore) {
                permissionStatusBlink = getTransientPermissionStatus(permissionTypeBlink, rorigin, frameToken);

                if (permissionStatusBlink != blink::mojom::PermissionStatus::ASK) {
                    result.push_back(permissionStatusBlink);
                    continue;
                }

                // Fall through to check if permission was pre-granted (and thus landed in the permanent store)
            }

            permissionStatusBlink = GetPermissionStatus(permissionTypeBlink, rorigin, rorigin);

            if (inTransientStore && permissionStatusBlink != blink::mojom::PermissionStatus::ASK) {
                // Move the pre-granted permission to the transient store and associate it with a frame token
                ResetPermission(permissionTypeBlink, rorigin, rorigin);
                setTransientPermission(permissionTypeBlink, rorigin,
                    permissionStatusBlink == blink::mojom::PermissionStatus::GRANTED, frameToken);
            }

            if (permissionStatusBlink != blink::mojom::PermissionStatus::ASK) {
                // Automatically grant/deny without prompt if already asked once
                result.push_back(permissionStatusBlink);
            } else {
                answerable = false;
                break;
            }
        } else {
            // Reached when clipboard settings have been set
            result.push_back(permissionStatusBlink);
        }
    }

    if (answerable) {
        std::move(callback).Run(result);
        return;
    }

    int request_id = ++m_requestIdCount;
    const auto requestOrigin = toQt(requestDescription.requesting_origin);
    m_multiRequests.push_back({ request_id, requestDescription.permissions, requestOrigin, std::move(callback) });
    auto qtPermissions = toQt(requestDescription.permissions);
    for (const QWebEnginePermission::PermissionType permissionTypeQt : qtPermissions) {
        contentsDelegate->requestFeaturePermission(permissionTypeQt, requestOrigin, frameToken);
    }
}

void PermissionManagerQt::RequestPermissionsFromCurrentDocument(
    content::RenderFrameHost *frameHost,
    const content::PermissionRequestDescription &requestDescription,
    base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus>&)> callback)
{
    RequestPermissions(frameHost, requestDescription, std::move(callback));
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatus(
    blink::PermissionType permissionTypeBlink,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/)
{
    const QWebEnginePermission::PermissionType permissionTypeQt = toQt(permissionTypeBlink);
    if (permissionTypeQt == QWebEnginePermission::PermissionType::Unsupported)
        return blink::mojom::PermissionStatus::DENIED;

    permissionTypeBlink = toBlink(toQt(permissionTypeBlink)); // Filter out merged/unsupported permissions (e.g. clipboard)
    auto *pref = m_prefService->FindPreference(permissionTypeString(permissionTypeQt));
    if (!pref)
        return blink::mojom::PermissionStatus::ASK; // Permission type not in database

    const auto *permissionsDict = pref->GetValue()->GetIfDict();
    Q_ASSERT(permissionsDict);

    const auto requestedPermission = permissionsDict->FindBool(requesting_origin.DeprecatedGetOriginAsURL().spec());
    if (!requestedPermission)
        return blink::mojom::PermissionStatus::ASK; // Origin is not in the current permission type's database

    if (requestedPermission.value())
        return blink::mojom::PermissionStatus::GRANTED;
    return blink::mojom::PermissionStatus::DENIED;
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatusForCurrentDocument(
        blink::PermissionType permissionTypeBlink,
        content::RenderFrameHost *render_frame_host, bool)
{
    Q_ASSERT(render_frame_host);

    if (permissionTypeBlink == blink::PermissionType::CLIPBOARD_READ_WRITE ||
        permissionTypeBlink == blink::PermissionType::CLIPBOARD_SANITIZED_WRITE) {
        WebContentsDelegateQt *delegate = static_cast<WebContentsDelegateQt *>(
                content::WebContents::FromRenderFrameHost(render_frame_host)->GetDelegate());
        Q_ASSERT(delegate);
        auto status = getStatusFromSettings(permissionTypeBlink, delegate->webEngineSettings());
        if (status != blink::mojom::PermissionStatus::ASK)
            return status;
    }

    permissionTypeBlink = toBlink(toQt(permissionTypeBlink)); // Filter out merged/unsupported permissions (e.g. clipboard)
    QWebEnginePermission::PermissionType permissionTypeQt = toQt(permissionTypeBlink);
    if (permissionTypeQt == QWebEnginePermission::PermissionType::Unsupported)
        return blink::mojom::PermissionStatus::DENIED;

    GURL origin = render_frame_host->GetLastCommittedOrigin().GetURL();
    auto status = blink::mojom::PermissionStatus::ASK;

    const bool inTransientStore = !QWebEnginePermission::isPersistent(permissionTypeQt) || !m_persistence;
    if (inTransientStore) {
        status = getTransientPermissionStatus(permissionTypeBlink, origin, render_frame_host->GetGlobalFrameToken());

        if (status != blink::mojom::PermissionStatus::ASK) {
            return status;
        }

        // Fall through to check if permission was pre-granted (and thus landed in the permanent store)
    }

    status = GetPermissionStatus(permissionTypeBlink, origin, origin);

    if (inTransientStore && status != blink::mojom::PermissionStatus::ASK) {
        // Move the pre-granted permission to the transient store and associate it with the rfh
        ResetPermission(permissionTypeBlink, origin, origin);
        setTransientPermission(permissionTypeBlink, origin, status == blink::mojom::PermissionStatus::GRANTED,
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

blink::mojom::PermissionStatus PermissionManagerQt::getTransientPermissionStatus(
        blink::PermissionType permissionTypeBlink,
        const GURL& requesting_origin,
        content::GlobalRenderFrameHostToken token)
{
    if (toQt(permissionTypeBlink) == QWebEnginePermission::PermissionType::Unsupported)
        return blink::mojom::PermissionStatus::DENIED;

    if (!m_transientPermissions.contains(token))
        return blink::mojom::PermissionStatus::ASK;

    auto &permissionsForToken = m_transientPermissions[token];
    for (auto p = permissionsForToken.begin(); p != permissionsForToken.end(); ++p) {
        if (get<0>(*p) == requesting_origin && get<1>(*p) == permissionTypeBlink) {
            return get<2>(*p)
                ? blink::mojom::PermissionStatus::GRANTED
                : blink::mojom::PermissionStatus::DENIED;
        }
    }

    return blink::mojom::PermissionStatus::ASK;
}

void PermissionManagerQt::setPersistentPermission(
        blink::PermissionType permissionTypeBlink,
        const GURL& requesting_origin,
        bool granted)
{
    const QWebEnginePermission::PermissionType permissionTypeQt = toQt(permissionTypeBlink);
    if (permissionTypeQt == QWebEnginePermission::PermissionType::Unsupported)
        return;

    if (!m_prefService->FindPreference(permissionTypeString(permissionTypeQt)))
        return;

    ScopedDictPrefUpdate updater(m_prefService.get(), permissionTypeString(permissionTypeQt));
    updater.Get().Set(requesting_origin.spec(), granted);

    m_prefService->SchedulePendingLossyWrites();
}

void PermissionManagerQt::setTransientPermission(
        blink::PermissionType permissionTypeBlink,
        const GURL& requesting_origin,
        bool granted,
        content::GlobalRenderFrameHostToken token)
{
    const QWebEnginePermission::PermissionType permissionTypeQt = toQt(permissionTypeBlink);
    if (permissionTypeQt == QWebEnginePermission::PermissionType::Unsupported)
        return;

    auto &permissionsForToken = m_transientPermissions[token];
    for (auto &p : permissionsForToken) {
        if (get<0>(p) == requesting_origin && get<1>(p) == permissionTypeBlink) {
            get<2>(p) = granted;
            return;
        }
    }

    permissionsForToken.push_back({requesting_origin, permissionTypeBlink, granted});

    // Render frame hosts get discarded often, so the map will eventualy fill up with junk unless
    // periodically cleaned. The number 25 was chosen arbitrarily.
    if (++m_transientWriteCount > 25) {
        content::GetUIThreadTaskRunner({})->PostTask(FROM_HERE,
            base::BindOnce([](PermissionManagerQt *p)
            {
                for (auto i = p->m_transientPermissions.begin(); i != p->m_transientPermissions.end(); ++i) {
                    if (content::RenderFrameHost::FromFrameToken(i->first) == nullptr) {
                        i = p->m_transientPermissions.erase(i);
                    }
                }
            }, this));
        m_transientWriteCount = 0;
    }
}

void PermissionManagerQt::resetTransientPermission(
        blink::PermissionType permissionTypeBlink,
        const GURL& requesting_origin,
        content::GlobalRenderFrameHostToken token)
{
    const QWebEnginePermission::PermissionType permissionTypeQt = toQt(permissionTypeBlink);
    if (permissionTypeQt == QWebEnginePermission::PermissionType::Unsupported)
        return;

    auto &permissionsForToken = m_transientPermissions[token];
    for (auto i = permissionsForToken.begin(); i != permissionsForToken.end(); ++i) {
        if (get<0>(*i) == requesting_origin && get<1>(*i) == permissionTypeBlink) {
            permissionsForToken.erase(i);
            return;
        }
    }
}

} // namespace QtWebEngineCore
