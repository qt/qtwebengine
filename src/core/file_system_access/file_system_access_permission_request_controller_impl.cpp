// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "file_system_access_permission_request_controller_impl.h"

#include "components/permissions/permission_util.h"
#include "content/public/browser/file_system_access_permission_context.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

ASSERT_ENUMS_MATCH(content::FileSystemAccessPermissionContext::HandleType::kFile,
                   QWebEngineFileSystemAccessRequest::HandleType::File);
ASSERT_ENUMS_MATCH(content::FileSystemAccessPermissionContext::HandleType::kDirectory,
                   QWebEngineFileSystemAccessRequest::HandleType::Directory);

ASSERT_ENUMS_MATCH(FileSystemAccessPermissionRequestManagerQt::Access::kRead,
                   QWebEngineFileSystemAccessRequest::AccessFlag::Read);
ASSERT_ENUMS_MATCH(FileSystemAccessPermissionRequestManagerQt::Access::kWrite,
                   QWebEngineFileSystemAccessRequest::AccessFlag::Write);

FileSystemAccessPermissionRequestControllerImpl::FileSystemAccessPermissionRequestControllerImpl(
        const FileSystemAccessPermissionRequestManagerQt::RequestData &request,
        base::OnceCallback<void(permissions::PermissionAction result)> callback)
    : FileSystemAccessPermissionRequestController(
            toQt(request.origin.GetURL()), QUrl::fromLocalFile(toQt(request.path.value())),
            (HandleType)request.handle_type, AccessFlags((int)request.access))
    , m_callback(std::move(callback))
{
}

FileSystemAccessPermissionRequestControllerImpl::~FileSystemAccessPermissionRequestControllerImpl()
{
    if (m_callback)
        std::move(m_callback).Run(permissions::PermissionAction::IGNORED);
}

void FileSystemAccessPermissionRequestControllerImpl::accepted()
{
    std::move(m_callback).Run(permissions::PermissionAction::GRANTED);
}

void FileSystemAccessPermissionRequestControllerImpl::rejected()
{
    std::move(m_callback).Run(permissions::PermissionAction::DENIED);
}

} // namespace QtWebEngineCore
