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
