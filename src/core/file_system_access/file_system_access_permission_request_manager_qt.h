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

#ifndef FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_MANAGER_QT_H
#define FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_MANAGER_QT_H

#include "base/callback_helpers.h"
#include "base/containers/circular_deque.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/file_system_access_permission_context.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "url/origin.h"

namespace permissions {
enum class PermissionAction;
}

namespace QtWebEngineCore {

class FileSystemAccessPermissionRequestManagerQt
    : public content::WebContentsObserver,
      public content::WebContentsUserData<FileSystemAccessPermissionRequestManagerQt>
{
public:
    ~FileSystemAccessPermissionRequestManagerQt() override;

    enum class Access {
        // Only ask for read access.
        kRead = 0x1,
        // Only ask for write access, assuming read access has already been granted.
        kWrite = 0x2,
        // Ask for both read and write access.
        kReadWrite = 0x3
    };

    struct RequestData
    {
        url::Origin origin;
        base::FilePath path;
        content::FileSystemAccessPermissionContext::HandleType handle_type;
        Access access;
    };

    void AddRequest(RequestData request,
                    base::OnceCallback<void(permissions::PermissionAction result)> callback,
                    base::ScopedClosureRunner fullscreen_block);

private:
    friend class content::WebContentsUserData<FileSystemAccessPermissionRequestManagerQt>;

    explicit FileSystemAccessPermissionRequestManagerQt(content::WebContents *web_contents);

    bool IsShowingRequest() const { return m_currentRequest != nullptr; }
    bool CanShowRequest() const;
    void ScheduleShowRequest();
    void DequeueAndShowRequest();

    // content::WebContentsObserver
    void DocumentOnLoadCompletedInMainFrame(content::RenderFrameHost *) override;
    void DidFinishNavigation(content::NavigationHandle *navigation_handle) override;
    void WebContentsDestroyed() override;

    void OnPermissionDialogResult(permissions::PermissionAction result);

    struct Request;
    // Request currently being shown in prompt.
    std::unique_ptr<Request> m_currentRequest;
    // Queued up requests.
    base::circular_deque<std::unique_ptr<Request>> m_queuedRequests;

    base::WeakPtrFactory<FileSystemAccessPermissionRequestManagerQt> m_weakFactory { this };
    WEB_CONTENTS_USER_DATA_KEY_DECL();
};

} // namespace QtWebEngineCore

#endif // FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_MANAGER_QT_H
