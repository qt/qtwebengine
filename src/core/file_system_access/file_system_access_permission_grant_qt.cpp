/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

void FileSystemAccessPermissionGrantQt::RequestPermission(
        content::GlobalRenderFrameHostId frame_id, UserActivationState user_activation_state,
        base::OnceCallback<void(PermissionRequestOutcome)> callback)
{
    // Check if a permission request has already been processed previously. This
    // check is done first because we don't want to reset the status of a
    // permission if it has already been granted.
    if (GetStatus() != PermissionStatus::ASK || !m_context) {
        if (GetStatus() == PermissionStatus::GRANTED)
            SetStatus(PermissionStatus::GRANTED);
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

void FileSystemAccessPermissionGrantQt::SetStatus(PermissionStatus status)
{
    bool should_notify = m_status != status;
    m_status = status;
    if (should_notify)
        NotifyPermissionStatusChanged();
}

void FileSystemAccessPermissionGrantQt::OnPermissionRequestResult(
        base::OnceCallback<void(PermissionRequestOutcome)> callback, PermissionAction result)
{
    switch (result) {
    case PermissionAction::GRANTED:
        SetStatus(PermissionStatus::GRANTED);
        std::move(callback).Run(PermissionRequestOutcome::kUserGranted);
        break;
    case PermissionAction::DENIED:
        SetStatus(PermissionStatus::DENIED);
        std::move(callback).Run(PermissionRequestOutcome::kUserDenied);
        break;
    case PermissionAction::DISMISSED:
    case PermissionAction::IGNORED:
        std::move(callback).Run(PermissionRequestOutcome::kUserDismissed);
        break;
    case PermissionAction::REVOKED:
    case PermissionAction::GRANTED_ONCE:
    case PermissionAction::NUM:
        NOTREACHED();
        break;
    }
}

} // namespace QtWebEngineCore
