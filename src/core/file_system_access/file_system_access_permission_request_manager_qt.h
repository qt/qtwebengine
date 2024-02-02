// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_MANAGER_QT_H
#define FILE_SYSTEM_ACCESS_PERMISSION_REQUEST_MANAGER_QT_H

#include "base/containers/circular_deque.h"
#include "base/files/file_path.h"
#include "base/functional/callback_helpers.h"
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
    void DocumentOnLoadCompletedInPrimaryMainFrame() override;
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
