// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_CONTROLLER_IMPL_H
#define FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_CONTROLLER_IMPL_H

#include "file_system_access_permission_request_controller.h"
#include "file_system_access_permission_request_manager_qt.h"

namespace QtWebEngineCore {

class FileSystemAccessPermissionRequestControllerImpl final
    : public FileSystemAccessPermissionRequestController
{
public:
    FileSystemAccessPermissionRequestControllerImpl(
            const FileSystemAccessPermissionRequestManagerQt::RequestData &request,
            base::OnceCallback<void(permissions::PermissionAction result)> callback);

    ~FileSystemAccessPermissionRequestControllerImpl();

protected:
    void accepted() override;
    void rejected() override;

private:
    base::OnceCallback<void(permissions::PermissionAction result)> m_callback;
};

} // namespace QtWebEngineCore

#endif // FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_CONTROLLER_IMPL_H
