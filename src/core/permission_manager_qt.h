// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PERMISSION_MANAGER_QT_H
#define PERMISSION_MANAGER_QT_H

#include "base/functional/callback.h"
#include "content/public/browser/global_routing_id.h"
#include "content/public/browser/permission_controller_delegate.h"
#include "content/public/browser/render_frame_host.h"

#include <QtWebEngineCore/qwebenginepermission.h>
#include "profile_adapter.h"

#include <map>
#include <tuple>

class PrefService;

namespace QtWebEngineCore {

class PermissionManagerQt : public content::PermissionControllerDelegate
{
public:
    PermissionManagerQt(ProfileAdapter *adapter);
    ~PermissionManagerQt();

    void setPermission(
        const QUrl &origin,
        QWebEnginePermission::PermissionType permissionType,
        QWebEnginePermission::State state,
        content::RenderFrameHost *rfh = nullptr);
    QWebEnginePermission::State getPermissionState(const QUrl &origin, QWebEnginePermission::PermissionType permissionType,
        content::RenderFrameHost *rfh = nullptr);
    QList<QWebEnginePermission> listPermissions(const QUrl &origin, QWebEnginePermission::PermissionType permissionType);

    void commit();

    // content::PermissionManager implementation:
    blink::mojom::PermissionStatus GetPermissionStatus(
        blink::PermissionType permission,
        const GURL& requesting_origin,
        const GURL& embedding_origin) override;

    blink::mojom::PermissionStatus GetPermissionStatusForCurrentDocument(blink::PermissionType, content::RenderFrameHost *) override;

    blink::mojom::PermissionStatus GetPermissionStatusForWorker(blink::PermissionType, content::RenderProcessHost *, const GURL &) override;

    blink::mojom::PermissionStatus GetPermissionStatusForEmbeddedRequester(blink::PermissionType, content::RenderFrameHost*, const url::Origin&) override;

    content::PermissionResult GetPermissionResultForOriginWithoutContext(blink::PermissionType, const url::Origin&, const url::Origin&) override;

    void ResetPermission(
        blink::PermissionType permission,
        const GURL& requesting_origin,
        const GURL& embedding_origin) override;

    void RequestPermissions(
            content::RenderFrameHost *render_frame_host,
            const content::PermissionRequestDescription &request_description,
            base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus>&)> callback) override;

    void RequestPermissionsFromCurrentDocument(
            content::RenderFrameHost *render_frame_host,
            const content::PermissionRequestDescription &request_description,
            base::OnceCallback<void(const std::vector<blink::mojom::PermissionStatus> &)> callback) override;

    content::PermissionControllerDelegate::SubscriptionId SubscribeToPermissionStatusChange(
            blink::PermissionType permission, content::RenderProcessHost *render_process_host,
            content::RenderFrameHost *render_frame_host, const GURL &requesting_origin,
            const base::RepeatingCallback<void(blink::mojom::PermissionStatus)> callback) override;

    void UnsubscribeFromPermissionStatusChange(
            content::PermissionControllerDelegate::SubscriptionId subscription_id) override;

private:
    struct Request {
        int id;
        QWebEnginePermission::PermissionType type;
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
        QWebEnginePermission::PermissionType type;
        QUrl origin;
        base::RepeatingCallback<void(blink::mojom::PermissionStatus)> callback;
    };

    blink::mojom::PermissionStatus getTransientPermissionStatus(blink::PermissionType permission,
        const GURL& requesting_origin,
        content::GlobalRenderFrameHostToken token);

    void setPersistentPermission(blink::PermissionType permission,
        const GURL& requesting_origin,
        bool granted);

    void setTransientPermission(blink::PermissionType permission,
        const GURL& requesting_origin,
        bool granted,
        content::GlobalRenderFrameHostToken token);

    void resetTransientPermission(blink::PermissionType permission,
        const GURL& requesting_origin,
        content::GlobalRenderFrameHostToken token);

    std::vector<Request> m_requests;
    std::vector<MultiRequest> m_multiRequests;
    std::vector<QWebEnginePermission::PermissionType> m_permissionTypes;
    std::map<content::GlobalRenderFrameHostToken,
        QList<std::tuple<GURL, blink::PermissionType, bool>>> m_transientPermissions;
    std::map<content::PermissionControllerDelegate::SubscriptionId, Subscription> m_subscribers;
    content::PermissionControllerDelegate::SubscriptionId::Generator subscription_id_generator_;
    int m_requestIdCount;
    int m_transientWriteCount;
    std::unique_ptr<PrefService> m_prefService;
    QPointer<QtWebEngineCore::ProfileAdapter> m_profileAdapter;
    bool m_persistence;
};

} // namespace QtWebEngineCore

#endif // PERMISSION_MANAGER_QT_H
