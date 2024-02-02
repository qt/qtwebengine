// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PERMISSION_MANAGER_QT_H
#define PERMISSION_MANAGER_QT_H

#include "base/functional/callback.h"
#include "content/public/browser/permission_controller_delegate.h"

#include "profile_adapter.h"

#include <map>

namespace QtWebEngineCore {

class PermissionManagerQt : public content::PermissionControllerDelegate
{
public:
    PermissionManagerQt();
    ~PermissionManagerQt();

    void permissionRequestReply(const QUrl &origin, ProfileAdapter::PermissionType type, ProfileAdapter::PermissionState reply);
    bool checkPermission(const QUrl &origin, ProfileAdapter::PermissionType type);

    // content::PermissionManager implementation:
    void RequestPermission(
        blink::PermissionType permission,
        content::RenderFrameHost* render_frame_host,
        const GURL& requesting_origin,
        bool user_gesture,
        base::OnceCallback<void(blink::mojom::PermissionStatus)> callback) override;

    blink::mojom::PermissionStatus GetPermissionStatus(
        blink::PermissionType permission,
        const GURL& requesting_origin,
        const GURL& embedding_origin) override;

    blink::mojom::PermissionStatus GetPermissionStatusForCurrentDocument(blink::PermissionType, content::RenderFrameHost *) override;

    blink::mojom::PermissionStatus GetPermissionStatusForWorker(blink::PermissionType, content::RenderProcessHost *, const GURL &) override;

    content::PermissionResult GetPermissionResultForOriginWithoutContext(blink::PermissionType, const url::Origin &) override;

    void ResetPermission(
        blink::PermissionType permission,
        const GURL& requesting_origin,
        const GURL& embedding_origin) override;

    void RequestPermissions(
        const std::vector<blink::PermissionType>& permission,
        content::RenderFrameHost* render_frame_host,
        const GURL& requesting_origin,
        bool user_gesture,
        base::OnceCallback<void(
            const std::vector<blink::mojom::PermissionStatus>&)> callback) override;

    void RequestPermissionsFromCurrentDocument(
        const std::vector<blink::PermissionType>& permissions,
        content::RenderFrameHost* render_frame_host,
        bool user_gesture,
        base::OnceCallback<void(
            const std::vector<blink::mojom::PermissionStatus>&)> callback) override;

    content::PermissionControllerDelegate::SubscriptionId SubscribePermissionStatusChange(
        blink::PermissionType permission,
        content::RenderProcessHost* render_process_host,
        content::RenderFrameHost* render_frame_host,
        const GURL& requesting_origin,
        const base::RepeatingCallback<void(blink::mojom::PermissionStatus)> callback) override;

    void UnsubscribePermissionStatusChange(content::PermissionControllerDelegate::SubscriptionId subscription_id) override;

private:
    QHash<QPair<QUrl, ProfileAdapter::PermissionType>, bool> m_permissions;
    struct Request {
        int id;
        ProfileAdapter::PermissionType type;
        QUrl origin;
        base::OnceCallback<void(blink::mojom::PermissionStatus)> callback;
    };
    struct MultiRequest {
        int id;
        std::vector<blink::PermissionType> types;
        QUrl origin;
        base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus>&)> callback;
    };
    struct Subscription {
        ProfileAdapter::PermissionType type;
        QUrl origin;
        base::RepeatingCallback<void(blink::mojom::PermissionStatus)> callback;
    };
    std::vector<Request> m_requests;
    std::vector<MultiRequest> m_multiRequests;
    std::map<content::PermissionControllerDelegate::SubscriptionId, Subscription> m_subscribers;
    content::PermissionControllerDelegate::SubscriptionId::Generator subscription_id_generator_;
    int m_requestIdCount;

};

} // namespace QtWebEngineCore

#endif // PERMISSION_MANAGER_QT_H
