// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "file_system_access_permission_grant_qt.h"

#include "file_system_access_permission_request_manager_qt.h"

#include "components/permissions/permission_util.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/disallow_activation_reason.h"
#include "url/origin.h"

namespace QtWebEngineCore {

FileSystemAccessPermissionGrantQt::FileSystemAccessPermissionGrantQt(
        base::WeakPtr<FileSystemAccessPermissionContextQt> context, const url::Origin &origin,
        const base::FilePath &path, HandleType handle_type, GrantType type)
    : m_context(context), m_origin(origin), m_path(path), m_handleType(handle_type), m_type(type)
{
}
FileSystemAccessPermissionGrantQt::~FileSystemAccessPermissionGrantQt()
{
    if (m_context)
        m_context->PermissionGrantDestroyed(this);
}
void FileSystemAccessPermissionGrantQt::RequestPermission(
        content::GlobalRenderFrameHostId frame_id, UserActivationState user_activation_state,
        base::OnceCallback<void(PermissionRequestOutcome)> callback)
{
    // Check if a permission request has already been processed previously. This
    // check is done first because we don't want to reset the status of a
    // permission if it has already been granted.
    if (GetStatus() != blink::mojom::PermissionStatus::ASK || !m_context) {
        if (GetStatus() == blink::mojom::PermissionStatus::GRANTED)
            SetStatus(blink::mojom::PermissionStatus::GRANTED);
        std::move(callback).Run(PermissionRequestOutcome::kRequestAborted);
        return;
    }

    // Otherwise, perform checks and ask the user for permission.

    content::RenderFrameHost *rfh = content::RenderFrameHost::FromID(frame_id);
    if (!rfh) {
        // Requested from a no longer valid render frame host.
        std::move(callback).Run(PermissionRequestOutcome::kInvalidFrame);
        return;
    }

    // Don't show request permission UI for an inactive RenderFrameHost as the
    // page might not distinguish properly between user denying the permission
    // and automatic rejection, leading to an inconsistent UX once the page
    // becomes active again.
    // - If this is called when RenderFrameHost is in BackForwardCache, evict
    //   the document from the cache.
    // - If this is called when RenderFrameHost is in prerendering, cancel
    //   prerendering.
    if (rfh->IsInactiveAndDisallowActivation(
                content::DisallowActivationReasonId::kFileSystemAccessPermissionRequest)) {
        std::move(callback).Run(PermissionRequestOutcome::kInvalidFrame);
        return;
    }

    if (user_activation_state == UserActivationState::kRequired
        && !rfh->HasTransientUserActivation()) {
        // No permission prompts without user activation.
        std::move(callback).Run(PermissionRequestOutcome::kNoUserActivation);
        return;
    }

    content::WebContents *web_contents = content::WebContents::FromRenderFrameHost(rfh);
    if (!web_contents) {
        // Requested from a worker, or a no longer existing tab.
        std::move(callback).Run(PermissionRequestOutcome::kInvalidFrame);
        return;
    }

    url::Origin embedding_origin = url::Origin::Create(web_contents->GetLastCommittedURL());
    if (embedding_origin != m_origin) {
        // Third party iframes are not allowed to request more permissions.
        std::move(callback).Run(PermissionRequestOutcome::kThirdPartyContext);
        return;
    }

    auto *request_manager =
            FileSystemAccessPermissionRequestManagerQt::FromWebContents(web_contents);
    if (!request_manager) {
        std::move(callback).Run(PermissionRequestOutcome::kRequestAborted);
        return;
    }

    // Drop fullscreen mode so that the user sees the URL bar.
    base::ScopedClosureRunner fullscreen_block = web_contents->ForSecurityDropFullscreen();

    FileSystemAccessPermissionRequestManagerQt::Access access = m_type == GrantType::kRead
            ? FileSystemAccessPermissionRequestManagerQt::Access::kRead
            : FileSystemAccessPermissionRequestManagerQt::Access::kWrite;

    // If a website wants both read and write access, code in content will
    // request those as two separate requests. The |request_manager| will then
    // detect this and combine the two requests into one prompt. As such this
    // code does not have to have any way to request Access::kReadWrite.

    request_manager->AddRequest(
            { m_origin, m_path, m_handleType, access },
            base::BindOnce(&FileSystemAccessPermissionGrantQt::OnPermissionRequestResult, this,
                           std::move(callback)),
            std::move(fullscreen_block));
}

void FileSystemAccessPermissionGrantQt::SetStatus(blink::mojom::PermissionStatus status)
{
    bool should_notify = m_status != status;
    m_status = status;
    if (should_notify)
        NotifyPermissionStatusChanged();
}

void FileSystemAccessPermissionGrantQt::OnPermissionRequestResult(
        base::OnceCallback<void(PermissionRequestOutcome)> callback, permissions::PermissionAction result)
{
    switch (result) {
    case permissions::PermissionAction::GRANTED:
        SetStatus(blink::mojom::PermissionStatus::GRANTED);
        std::move(callback).Run(PermissionRequestOutcome::kUserGranted);
        break;
    case permissions::PermissionAction::DENIED:
        SetStatus(blink::mojom::PermissionStatus::DENIED);
        std::move(callback).Run(PermissionRequestOutcome::kUserDenied);
        break;
    case permissions::PermissionAction::DISMISSED:
    case permissions::PermissionAction::IGNORED:
        std::move(callback).Run(PermissionRequestOutcome::kUserDismissed);
        break;
    case permissions::PermissionAction::REVOKED:
    case permissions::PermissionAction::GRANTED_ONCE:
    case permissions::PermissionAction::NUM:
        NOTREACHED();
        break;
    }
}

} // namespace QtWebEngineCore
