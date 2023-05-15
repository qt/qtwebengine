// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_CONTROLLER_H
#define FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_CONTROLLER_H

#include "api/qwebenginefilesystemaccessrequest.h"
#include "request_controller.h"

using HandleType = QWebEngineFileSystemAccessRequest::HandleType;
using AccessFlags = QWebEngineFileSystemAccessRequest::AccessFlags;

namespace QtWebEngineCore {

class FileSystemAccessPermissionRequestController : public RequestController
{
public:
    FileSystemAccessPermissionRequestController(const QUrl &origin, const QUrl &filePath,
                                                HandleType handleType, AccessFlags accessType)
        : RequestController(origin)
        , m_filePath(filePath)
        , m_handleType(handleType)
        , m_accessType(accessType)
    {
    }

    QUrl filePath() const { return m_filePath; }
    HandleType handleType() const { return m_handleType; }
    AccessFlags accessFlags() const { return m_accessType; }

private:
    QUrl m_filePath;
    HandleType m_handleType;
    AccessFlags m_accessType;
};

} // namespace QtWebEngineCore

#endif // FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_CONTROLLER_H
