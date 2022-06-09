// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// This file is based on chrome/browser/file_system_access/chrome_file_system_access_permission_context.h:
// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILE_SYSTEM_ACCESS_PERMISSION_CONTEXT_QT_H
#define FILE_SYSTEM_ACCESS_PERMISSION_CONTEXT_QT_H

#include "base/files/file_path.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/file_system_access_permission_context.h"
#include "content/public/browser/global_routing_id.h"

namespace content {
class BrowserContext;
}

namespace QtWebEngineCore {

class FileSystemAccessPermissionContextQt : public content::FileSystemAccessPermissionContext,
                                            public KeyedService
{
public:
    explicit FileSystemAccessPermissionContextQt(content::BrowserContext *context);
    ~FileSystemAccessPermissionContextQt() override;

    enum class GrantType { kRead, kWrite };

    // content::FileSystemAccessPermissionContext:
    scoped_refptr<content::FileSystemAccessPermissionGrant>
    GetReadPermissionGrant(const url::Origin &origin, const base::FilePath &path,
                           HandleType handle_type, UserAction user_action) override;
    scoped_refptr<content::FileSystemAccessPermissionGrant>
    GetWritePermissionGrant(const url::Origin &origin, const base::FilePath &path,
                            HandleType handle_type, UserAction user_action) override;
    void ConfirmSensitiveDirectoryAccess(
            const url::Origin &origin, PathType path_type, const base::FilePath &path,
            HandleType handle_type, content::GlobalRenderFrameHostId frame_id,
            base::OnceCallback<void(SensitiveDirectoryResult)> callback) override;
    void PerformAfterWriteChecks(std::unique_ptr<content::FileSystemAccessWriteItem> item,
                                 content::GlobalRenderFrameHostId frame_id,
                                 base::OnceCallback<void(AfterWriteCheckResult)> callback) override;
    bool CanObtainReadPermission(const url::Origin &origin) override;
    bool CanObtainWritePermission(const url::Origin &origin) override;
    void SetLastPickedDirectory(const url::Origin &origin, const std::string &id,
                                const base::FilePath &path, const PathType type) override;
    FileSystemAccessPermissionContextQt::PathInfo
    GetLastPickedDirectory(const url::Origin &origin, const std::string &id) override;
    base::FilePath GetWellKnownDirectoryPath(blink::mojom::WellKnownDirectory directory) override;

    void NavigatedAwayFromOrigin(const url::Origin &origin);
    content::BrowserContext *profile() const { return m_profile; }

private:
    class PermissionGrantImpl;

    void DidConfirmSensitiveDirectoryAccess(
            const url::Origin &origin, const base::FilePath &path, HandleType handle_type,
            content::GlobalRenderFrameHostId frame_id,
            base::OnceCallback<void(SensitiveDirectoryResult)> callback, bool should_block);

    content::BrowserContext *m_profile;

    // Permission state per origin.
    struct OriginState;
    std::map<url::Origin, OriginState> m_origins;

    std::map<std::string, FileSystemAccessPermissionContextQt::PathInfo> m_lastPickedDirectories;

    base::WeakPtrFactory<FileSystemAccessPermissionContextQt> m_weakFactory { this };
};

} // namespace QtWebEngineCore

#endif // FILE_SYSTEM_ACCESS_PERMISSION_CONTEXT_QT_H
