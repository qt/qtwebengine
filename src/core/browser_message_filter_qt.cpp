// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "browser_message_filter_qt.h"

#include "chrome/browser/profiles/profile.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"

#include "common/qt_messages.h"
#include "profile_io_data_qt.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

BrowserMessageFilterQt::BrowserMessageFilterQt(int /*render_process_id*/, Profile *profile)
    : BrowserMessageFilter(QtMsgStart)
    , m_profileData(ProfileIODataQt::FromBrowserContext(profile))
{
}

bool BrowserMessageFilterQt::OnMessageReceived(const IPC::Message& message)
{
    IPC_BEGIN_MESSAGE_MAP(BrowserMessageFilterQt, message)
        IPC_MESSAGE_HANDLER(QtWebEngineHostMsg_AllowStorageAccess, OnAllowStorageAccess)
        IPC_MESSAGE_HANDLER_DELAY_REPLY(QtWebEngineHostMsg_RequestStorageAccessSync,
                                        OnRequestStorageAccessSync)
        IPC_MESSAGE_HANDLER(QtWebEngineHostMsg_RequestStorageAccessAsync,
                            OnRequestStorageAccessAsync)
        IPC_MESSAGE_UNHANDLED(return false)
    IPC_END_MESSAGE_MAP()
    return true;
}

void BrowserMessageFilterQt::OnAllowStorageAccess(int /*render_frame_id*/,
                                                  const GURL &origin_url,
                                                  const GURL &top_origin_url,
                                                  int /*storage_type*/,
                                                  bool *allowed)
{
    *allowed = m_profileData->canGetCookies(toQt(top_origin_url), toQt(origin_url));
}

void BrowserMessageFilterQt::OnRequestStorageAccessSync(int render_frame_id,
                                                        const GURL& origin_url,
                                                        const GURL& top_origin_url,
                                                        int storage_type,
                                                        IPC::Message* reply_msg)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    auto callback = base::BindOnce(
            &BrowserMessageFilterQt::OnRequestStorageAccessSyncResponse,
            base::WrapRefCounted(this), reply_msg);
    OnRequestStorageAccess(render_frame_id,
                           origin_url,
                           top_origin_url,
                           storage_type,
                           std::move(callback));
}

void BrowserMessageFilterQt::OnRequestStorageAccessSyncResponse(IPC::Message *reply_msg, bool allowed)
{
    QtWebEngineHostMsg_RequestStorageAccessSync::WriteReplyParams(reply_msg, allowed);
    Send(reply_msg);
}

void BrowserMessageFilterQt::OnRequestStorageAccessAsync(int render_frame_id,
                                                         int request_id,
                                                         const GURL& origin_url,
                                                         const GURL& top_origin_url,
                                                         int storage_type)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    auto callback = base::BindOnce(
            &BrowserMessageFilterQt::OnRequestStorageAccessAsyncResponse,
            base::WrapRefCounted(this), render_frame_id, request_id);
    OnRequestStorageAccess(render_frame_id,
                           origin_url,
                           top_origin_url,
                           storage_type,
                           std::move(callback));
}

void BrowserMessageFilterQt::OnRequestStorageAccessAsyncResponse(int render_frame_id,
                                                                 int request_id,
                                                                 bool allowed)
{
    Send(new QtWebEngineMsg_RequestStorageAccessAsyncResponse(render_frame_id, request_id, allowed));
}

void BrowserMessageFilterQt::OnRequestStorageAccess(int /*render_frame_id*/,
                                                    const GURL &origin_url,
                                                    const GURL &top_origin_url,
                                                    int /*storage_type*/,
                                                    base::OnceCallback<void(bool)> callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    bool allowed = m_profileData->canGetCookies(toQt(top_origin_url), toQt(origin_url));

    std::move(callback).Run(allowed);
}

} // namespace QtWebEngineCore
