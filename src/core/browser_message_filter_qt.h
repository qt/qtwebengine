// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BROWSER_MESSAGE_FILTER_QT_H
#define BROWSER_MESSAGE_FILTER_QT_H

#include "base/functional/callback.h"
#include "content/public/browser/browser_message_filter.h"

class GURL;
class Profile;

namespace QtWebEngineCore {

class ProfileIODataQt;

class BrowserMessageFilterQt : public content::BrowserMessageFilter
{
public:
    BrowserMessageFilterQt(int render_process_id, Profile *profile);

private:
    bool OnMessageReceived(const IPC::Message& message) override;


    void OnAllowStorageAccess(int render_frame_id,
                              const GURL &origin_url,
                              const GURL &top_origin_url,
                              int storage_type,
                              bool *allowed);

    void OnRequestStorageAccessSync(int render_frame_id,
                                    const GURL &origin_url,
                                    const GURL &top_origin_url,
                                    int storage_type,
                                    IPC::Message *message);
    void OnRequestStorageAccessAsync(int render_frame_id,
                                     int request_id,
                                     const GURL &origin_url,
                                     const GURL &top_origin_url,
                                     int storage_type);
    void OnRequestStorageAccessSyncResponse(IPC::Message *reply_msg,
                                            bool allowed);
    void OnRequestStorageAccessAsyncResponse(int render_frame_id,
                                             int request_id,
                                             bool allowed);
    void OnRequestStorageAccess(int render_frame_id,
                                const GURL &origin_url,
                                const GURL &top_origin_url,
                                int storage_type,
                                base::OnceCallback<void(bool)> callback);

    ProfileIODataQt *m_profileData;
};

} // namespace QtWebEngineCore

#endif // BROWSER_MESSAGE_FILTER_QT_H
