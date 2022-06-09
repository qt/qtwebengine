// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef FILE_SYSTEM_ACCESS_PERMISSION_GRANT_QT_H
#define FILE_SYSTEM_ACCESS_PERMISSION_GRANT_QT_H

#include "content/public/browser/file_system_access_permission_grant.h"
#include "url/origin.h"
#include "file_system_access_permission_context_qt.h"
#include "content/public/browser/global_routing_id.h"
#include "components/permissions/permission_util.h"

namespace QtWebEngineCore {

using HandleType = content::FileSystemAccessPermissionContext::HandleType;
using GrantType = FileSystemAccessPermissionContextQt::GrantType;
using blink::mojom::PermissionStatus;
using permissions::PermissionAction;

class FileSystemAccessPermissionGrantQt : public content::FileSystemAccessPermissionGrant
{
public:
    FileSystemAccessPermissionGrantQt(base::WeakPtr<FileSystemAccessPermissionContextQt> context,
                                      const url::Origin &origin, const base::FilePath &path,
                                      HandleType handle_type, GrantType type);

    // content::FileSystemAccessPermissionGrant:
    PermissionStatus GetStatus() override { return m_status; }
    base::FilePath GetPath() override { return m_path; }
    void RequestPermission(content::GlobalRenderFrameHostId frame_id,
                           UserActivationState user_activation_state,
                           base::OnceCallback<void(PermissionRequestOutcome)> callback) override;

    const url::Origin &origin() const { return m_origin; }
    HandleType handleType() const { return m_handleType; }
    const base::FilePath &path() const { return m_path; }
    GrantType type() const { return m_type; }

    void SetStatus(PermissionStatus status);

private:
    void OnPermissionRequestResult(base::OnceCallback<void(PermissionRequestOutcome)> callback,
                                   PermissionAction result);

    base::WeakPtr<FileSystemAccessPermissionContextQt> const m_context;
    const url::Origin m_origin;
    const base::FilePath m_path;
    const HandleType m_handleType;
    const GrantType m_type;

    // This member should only be updated via SetStatus(), to make sure
    // observers are properly notified about any change in status.
    PermissionStatus m_status = PermissionStatus::ASK;
};

} // namespace QtWebEngineCore

#endif // FILE_SYSTEM_ACCESS_PERMISSION_GRANT_QT_H
