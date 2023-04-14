// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "permission_manager_qt.h"

#include "content/browser/renderer_host/render_view_host_delegate.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/permission_controller.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "third_party/blink/public/common/permissions/permission_utils.h"

#include "type_conversion.h"
#include "web_contents_delegate_qt.h"
#include "web_engine_settings.h"

namespace QtWebEngineCore {

static ProfileAdapter::PermissionType toQt(blink::PermissionType type)
{
    switch (type) {
    case blink::PermissionType::GEOLOCATION:
        return ProfileAdapter::GeolocationPermission;
    case blink::PermissionType::AUDIO_CAPTURE:
        return ProfileAdapter::AudioCapturePermission;
    case blink::PermissionType::VIDEO_CAPTURE:
        return ProfileAdapter::VideoCapturePermission;
    case blink::PermissionType::CLIPBOARD_READ_WRITE:
        return ProfileAdapter::ClipboardRead;
    case blink::PermissionType::CLIPBOARD_SANITIZED_WRITE:
        return ProfileAdapter::ClipboardWrite;
    case blink::PermissionType::NOTIFICATIONS:
        return ProfileAdapter::NotificationPermission;
    case blink::PermissionType::ACCESSIBILITY_EVENTS:
    case blink::PermissionType::CAMERA_PAN_TILT_ZOOM:
    case blink::PermissionType::WINDOW_MANAGEMENT:
        return ProfileAdapter::UnsupportedPermission;
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
    case blink::PermissionType::LOCAL_FONTS:
    case blink::PermissionType::DISPLAY_CAPTURE:
    case blink::PermissionType::TOP_LEVEL_STORAGE_ACCESS:
    case blink::PermissionType::NUM:
        LOG(INFO) << "Unexpected unsupported permission type: " << static_cast<int>(type);
        break;
    }
    return ProfileAdapter::UnsupportedPermission;
}

static bool canRequestPermissionFor(ProfileAdapter::PermissionType type)
{
    switch (type) {
    case ProfileAdapter::GeolocationPermission:
    case ProfileAdapter::NotificationPermission:
        return true;
    default:
        break;
    }
    return false;
}

static blink::mojom::PermissionStatus toBlink(ProfileAdapter::PermissionState reply)
{
    switch (reply) {
    case ProfileAdapter::AskPermission:
        return blink::mojom::PermissionStatus::ASK;
    case ProfileAdapter::AllowedPermission:
        return blink::mojom::PermissionStatus::GRANTED;
    case ProfileAdapter::DeniedPermission:
        return blink::mojom::PermissionStatus::DENIED;
    }
}

static blink::mojom::PermissionStatus getStatusFromSettings(blink::PermissionType type, WebEngineSettings *settings)
{
    switch (type) {
    case blink::PermissionType::CLIPBOARD_READ_WRITE:
        if (!settings->testAttribute(QWebEngineSettings::JavascriptCanPaste))
            return blink::mojom::PermissionStatus::DENIED;
         Q_FALLTHROUGH();
    case blink::PermissionType::CLIPBOARD_SANITIZED_WRITE:
        if (!settings->testAttribute(QWebEngineSettings::JavascriptCanAccessClipboard))
            return blink::mojom::PermissionStatus::DENIED;
        return blink::mojom::PermissionStatus::GRANTED;
    default:
        return blink::mojom::PermissionStatus::ASK;
    }
}

PermissionManagerQt::PermissionManagerQt()
    : m_requestIdCount(0)
{
}

PermissionManagerQt::~PermissionManagerQt()
{
}

void PermissionManagerQt::permissionRequestReply(const QUrl &url, ProfileAdapter::PermissionType type, ProfileAdapter::PermissionState reply)
{
    // Normalize the QUrl to Chromium origin form.
    const GURL gorigin = toGurl(url).DeprecatedGetOriginAsURL();
    const QUrl origin = gorigin.is_empty() ? url : toQt(gorigin);
    if (origin.isEmpty())
        return;
    QPair<QUrl, ProfileAdapter::PermissionType> key(origin, type);
    if (reply == ProfileAdapter::AskPermission)
        m_permissions.remove(key);
    else
        m_permissions[key] = (reply == ProfileAdapter::AllowedPermission);
    blink::mojom::PermissionStatus status = toBlink(reply);
    if (reply != ProfileAdapter::AskPermission) {
        auto it = m_requests.begin();
        while (it != m_requests.end()) {
            if (it->origin == origin && it->type == type) {
                std::move(it->callback).Run(status);
                it = m_requests.erase(it);
            } else
                ++it;
        }
    }
    for (const auto &it: m_subscribers) {
        if (it.second.origin == origin && it.second.type == type)
            it.second.callback.Run(status);
    }

    if (reply == ProfileAdapter::AskPermission)
        return;

    auto it = m_multiRequests.begin();
    while (it != m_multiRequests.end()) {
        if (it->origin == origin) {
            bool answerable = true;
            std::vector<blink::mojom::PermissionStatus> result;
            result.reserve(it->types.size());
            for (blink::PermissionType permission : it->types) {
                const ProfileAdapter::PermissionType permissionType = toQt(permission);
                if (permissionType == ProfileAdapter::UnsupportedPermission) {
                    result.push_back(blink::mojom::PermissionStatus::DENIED);
                    continue;
                }

                QPair<QUrl, ProfileAdapter::PermissionType> key(origin, permissionType);
                if (!m_permissions.contains(key)) {
                    answerable = false;
                    break;
                }
                if (m_permissions[key])
                    result.push_back(blink::mojom::PermissionStatus::GRANTED);
                else
                    result.push_back(blink::mojom::PermissionStatus::DENIED);
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

bool PermissionManagerQt::checkPermission(const QUrl &origin, ProfileAdapter::PermissionType type)
{
    QPair<QUrl, ProfileAdapter::PermissionType> key(origin, type);
    return m_permissions.contains(key) && m_permissions[key];
}

void PermissionManagerQt::RequestPermission(blink::PermissionType permission,
                                            content::RenderFrameHost *frameHost,
                                            const GURL& requesting_origin,
                                            bool /*user_gesture*/,
                                            base::OnceCallback<void(blink::mojom::PermissionStatus)> callback)
{
    if (requesting_origin.is_empty()) {
        std::move(callback).Run(blink::mojom::PermissionStatus::DENIED);
        return;
    }

    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(
        content::WebContents::FromRenderFrameHost(frameHost)->GetDelegate());
    Q_ASSERT(contentsDelegate);

    ProfileAdapter::PermissionType permissionType = toQt(permission);
    if (permissionType == ProfileAdapter::ClipboardRead || permissionType == ProfileAdapter::ClipboardWrite) {
        std::move(callback).Run(getStatusFromSettings(permission, contentsDelegate->webEngineSettings()));
        return;
    } else if (!canRequestPermissionFor(permissionType)) {
        std::move(callback).Run(blink::mojom::PermissionStatus::DENIED);
        return;
    }

    int request_id = ++m_requestIdCount;
    auto requestOrigin = toQt(requesting_origin);
    m_requests.push_back({ request_id, permissionType, requestOrigin, std::move(callback) });
    contentsDelegate->requestFeaturePermission(permissionType, requestOrigin);
}

void PermissionManagerQt::RequestPermissions(const std::vector<blink::PermissionType> &permissions,
                                             content::RenderFrameHost *frameHost,
                                             const GURL &requesting_origin,
                                             bool /*user_gesture*/,
                                             base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus> &)> callback)
{
    if (requesting_origin.is_empty()) {
        std::move(callback).Run(std::vector<blink::mojom::PermissionStatus>(permissions.size(), blink::mojom::PermissionStatus::DENIED));
        return;
    }

    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(
        content::WebContents::FromRenderFrameHost(frameHost)->GetDelegate());
    Q_ASSERT(contentsDelegate);

    bool answerable = true;
    std::vector<blink::mojom::PermissionStatus> result;
    result.reserve(permissions.size());
    for (blink::PermissionType permission : permissions) {
        const ProfileAdapter::PermissionType permissionType = toQt(permission);
        if (permissionType == ProfileAdapter::UnsupportedPermission)
            result.push_back(blink::mojom::PermissionStatus::DENIED);
        else if (permissionType == ProfileAdapter::ClipboardRead || permissionType == ProfileAdapter::ClipboardWrite)
            result.push_back(getStatusFromSettings(permission, contentsDelegate->webEngineSettings()));
        else {
            answerable = false;
            break;
        }
    }
    if (answerable) {
        std::move(callback).Run(result);
        return;
    }

    int request_id = ++m_requestIdCount;
    auto requestOrigin = toQt(requesting_origin);
    m_multiRequests.push_back({ request_id, permissions, requestOrigin, std::move(callback) });
    for (blink::PermissionType permission : permissions) {
        const ProfileAdapter::PermissionType permissionType = toQt(permission);
        if (canRequestPermissionFor(permissionType))
            contentsDelegate->requestFeaturePermission(permissionType, requestOrigin);
    }
}

void PermissionManagerQt::RequestPermissionsFromCurrentDocument(const std::vector<blink::PermissionType> &permissions,
                                                                content::RenderFrameHost *frameHost,
                                                                bool user_gesture,
                                                                base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus>&)> callback)
{
    RequestPermissions(permissions, frameHost, frameHost->GetLastCommittedOrigin().GetURL(), user_gesture, std::move(callback));
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatus(
    blink::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/)
{
    const ProfileAdapter::PermissionType permissionType = toQt(permission);
    if (permissionType == ProfileAdapter::UnsupportedPermission)
        return blink::mojom::PermissionStatus::DENIED;

    QPair<QUrl, ProfileAdapter::PermissionType> key(toQt(requesting_origin), permissionType);
    if (!m_permissions.contains(key))
        return blink::mojom::PermissionStatus::ASK;
    if (m_permissions[key])
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
        return getStatusFromSettings(permission, delegate->webEngineSettings());
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

content::PermissionResult PermissionManagerQt::GetPermissionResultForOriginWithoutContext(
        blink::PermissionType permission,
        const url::Origin &origin)
{
    blink::mojom::PermissionStatus status =
            GetPermissionStatus(permission, origin.GetURL(), origin.GetURL());

    return content::PermissionResult(status, content::PermissionStatusSource::UNSPECIFIED);
}

void PermissionManagerQt::ResetPermission(
    blink::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/)
{
    const ProfileAdapter::PermissionType permissionType = toQt(permission);
    if (permissionType == ProfileAdapter::UnsupportedPermission)
        return;

    QPair<QUrl, ProfileAdapter::PermissionType> key(toQt(requesting_origin), permissionType);
    m_permissions.remove(key);
}

content::PermissionControllerDelegate::SubscriptionId PermissionManagerQt::SubscribePermissionStatusChange(
    blink::PermissionType permission,
    content::RenderProcessHost * /*render_process_host*/,
    content::RenderFrameHost * /* render_frame_host */,
    const GURL& requesting_origin,
    base::RepeatingCallback<void(blink::mojom::PermissionStatus)> callback)
{
    auto subscriber_id = subscription_id_generator_.GenerateNextId();
    m_subscribers.insert( { subscriber_id,
                            Subscription { toQt(permission), toQt(requesting_origin), std::move(callback) } });
    return subscriber_id;
}

void PermissionManagerQt::UnsubscribePermissionStatusChange(content::PermissionControllerDelegate::SubscriptionId subscription_id)
{
    if (!m_subscribers.erase(subscription_id))
        LOG(WARNING) << "PermissionManagerQt::UnsubscribePermissionStatusChange called on unknown subscription id" << subscription_id;
}

} // namespace QtWebEngineCore
