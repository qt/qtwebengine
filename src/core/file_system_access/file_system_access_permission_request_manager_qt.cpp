// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "file_system_access_permission_request_manager_qt.h"

#include "components/permissions/permission_util.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/navigation_handle.h"

#include "api/qwebenginefilesystemaccessrequest.h"
#include "file_system_access_permission_context_factory_qt.h"
#include "file_system_access_permission_request_controller_impl.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

namespace QtWebEngineCore {

bool RequestsAreIdentical(const FileSystemAccessPermissionRequestManagerQt::RequestData &a,
                          const FileSystemAccessPermissionRequestManagerQt::RequestData &b)
{
    return a.origin == b.origin && a.path == b.path && a.handle_type == b.handle_type
            && a.access == b.access;
}

bool RequestsAreForSamePath(const FileSystemAccessPermissionRequestManagerQt::RequestData &a,
                            const FileSystemAccessPermissionRequestManagerQt::RequestData &b)
{
    return a.origin == b.origin && a.path == b.path && a.handle_type == b.handle_type;
}

struct FileSystemAccessPermissionRequestManagerQt::Request
{
    Request(RequestData data,
            base::OnceCallback<void(permissions::PermissionAction result)> callback,
            base::ScopedClosureRunner fullscreen_block)
        : data(std::move(data))
    {
        callbacks.push_back(std::move(callback));
        fullscreen_blocks.push_back(std::move(fullscreen_block));
    }

    RequestData data;
    std::vector<base::OnceCallback<void(permissions::PermissionAction result)>> callbacks;
    std::vector<base::ScopedClosureRunner> fullscreen_blocks;
};

FileSystemAccessPermissionRequestManagerQt::~FileSystemAccessPermissionRequestManagerQt() = default;

void FileSystemAccessPermissionRequestManagerQt::AddRequest(
        RequestData data, base::OnceCallback<void(permissions::PermissionAction result)> callback,
        base::ScopedClosureRunner fullscreen_block)
{
    // Check if any pending requests are identical to the new request.
    if (m_currentRequest && RequestsAreIdentical(m_currentRequest->data, data)) {
        m_currentRequest->callbacks.push_back(std::move(callback));
        m_currentRequest->fullscreen_blocks.push_back(std::move(fullscreen_block));
        return;
    }
    for (const auto &request : m_queuedRequests) {
        if (RequestsAreIdentical(request->data, data)) {
            request->callbacks.push_back(std::move(callback));
            request->fullscreen_blocks.push_back(std::move(fullscreen_block));
            return;
        }
        if (RequestsAreForSamePath(request->data, data)) {
            // This means access levels are different. Change the existing request
            // to kReadWrite, and add the new callback.
            request->data.access = Access::kReadWrite;
            request->callbacks.push_back(std::move(callback));
            request->fullscreen_blocks.push_back(std::move(fullscreen_block));
            return;
        }
    }

    m_queuedRequests.push_back(std::make_unique<Request>(std::move(data), std::move(callback),
                                                         std::move(fullscreen_block)));
    if (!IsShowingRequest())
        ScheduleShowRequest();
}

FileSystemAccessPermissionRequestManagerQt::FileSystemAccessPermissionRequestManagerQt(
        content::WebContents *web_contents)
    : content::WebContentsObserver(web_contents)
    , content::WebContentsUserData<FileSystemAccessPermissionRequestManagerQt>(*web_contents)
{
}

bool FileSystemAccessPermissionRequestManagerQt::CanShowRequest() const
{
    // Delay showing requests until the main frame is fully loaded.
    // ScheduleShowRequest() will be called again when that happens.
    return web_contents()->IsDocumentOnLoadCompletedInPrimaryMainFrame() && !m_queuedRequests.empty()
            && !m_currentRequest;
}

void FileSystemAccessPermissionRequestManagerQt::ScheduleShowRequest()
{
    if (!CanShowRequest())
        return;

    content::GetUIThreadTaskRunner({})->PostTask(
            FROM_HERE,
            base::BindOnce(&FileSystemAccessPermissionRequestManagerQt::DequeueAndShowRequest,
                           m_weakFactory.GetWeakPtr()));
}

void FileSystemAccessPermissionRequestManagerQt::DequeueAndShowRequest()
{
    if (!CanShowRequest())
        return;

    m_currentRequest = std::move(m_queuedRequests.front());
    m_queuedRequests.pop_front();

    WebContentsAdapterClient *client =
            WebContentsViewQt::from(
                    static_cast<content::WebContentsImpl *>(web_contents())->GetView())
                    ->client();
    if (!client) {
        LOG(ERROR)
                << "Attempt to request file system access from content missing WebContents client";
        for (auto &callback : m_currentRequest->callbacks)
            std::move(callback).Run(permissions::PermissionAction::DENIED);
        return;
    }

    QWebEngineFileSystemAccessRequest request(
            std::make_shared<FileSystemAccessPermissionRequestControllerImpl>(
                    m_currentRequest->data,
                    base::BindOnce(
                            &FileSystemAccessPermissionRequestManagerQt::OnPermissionDialogResult,
                            m_weakFactory.GetWeakPtr())));
    client->runFileSystemAccessRequest(std::move(request));
}

void FileSystemAccessPermissionRequestManagerQt::DocumentOnLoadCompletedInPrimaryMainFrame()
{
    // This is scheduled because while all calls to the browser have been
    // issued at DOMContentLoaded, they may be bouncing around in scheduled
    // callbacks finding the UI thread still. This makes sure we allow those
    // scheduled calls to AddRequest to complete before we show the page-load
    // permissions bubble.
    if (!m_queuedRequests.empty())
        ScheduleShowRequest();
}

void FileSystemAccessPermissionRequestManagerQt::DidFinishNavigation(
        content::NavigationHandle *navigation)
{
    // We only care about top-level navigations that actually committed.
    if (!navigation->IsInMainFrame() || !navigation->HasCommitted())
        return;

    auto src_origin = url::Origin::Create(navigation->GetPreviousPrimaryMainFrameURL());
    auto dest_origin = url::Origin::Create(navigation->GetURL());
    if (src_origin == dest_origin)
        return;

    // Navigated away from |src_origin|, tell permission context to check if
    // permissions need to be revoked.
    auto *context = FileSystemAccessPermissionContextFactoryQt::GetForProfileIfExists(
            web_contents()->GetBrowserContext());
    if (context)
        context->NavigatedAwayFromOrigin(src_origin);
}

void FileSystemAccessPermissionRequestManagerQt::WebContentsDestroyed()
{
    auto src_origin = web_contents()->GetPrimaryMainFrame()->GetLastCommittedOrigin();

    // Navigated away from |src_origin|, tell permission context to check if
    // permissions need to be revoked.
    auto *context = FileSystemAccessPermissionContextFactoryQt::GetForProfileIfExists(
            web_contents()->GetBrowserContext());
    if (context)
        context->NavigatedAwayFromOrigin(src_origin);
}

void FileSystemAccessPermissionRequestManagerQt::OnPermissionDialogResult(
        permissions::PermissionAction result)
{
    DCHECK(m_currentRequest);
    for (auto &callback : m_currentRequest->callbacks)
        std::move(callback).Run(result);

    m_currentRequest = nullptr;
    if (!m_queuedRequests.empty())
        ScheduleShowRequest();
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(FileSystemAccessPermissionRequestManagerQt);

} // namespace QtWebEngineCore
