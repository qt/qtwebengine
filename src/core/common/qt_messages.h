// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// Multiply-included file, no traditional include guard.

#include "content/public/common/common_param_traits.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_start.h"
#include "url/gurl.h"

#define IPC_MESSAGE_START QtMsgStart

//-----------------------------------------------------------------------------
// These are messages sent from the browser to the renderer process.

// Tells the renderer whether or not a storage access has been allowed.
IPC_MESSAGE_ROUTED2(QtWebEngineMsg_RequestStorageAccessAsyncResponse,
                    int  /* request_id */,
                    bool /* allowed */)

//-----------------------------------------------------------------------------
// These are messages sent from the renderer to the browser process.

IPC_SYNC_MESSAGE_CONTROL4_1(QtWebEngineHostMsg_AllowStorageAccess,
                            int /* render_frame_id */,
                            GURL /* origin_url */,
                            GURL /* top origin url */,
                            int /* storage_type */,
                            bool /* allowed */)

IPC_SYNC_MESSAGE_CONTROL4_1(QtWebEngineHostMsg_RequestStorageAccessSync,
                            int /* render_frame_id */,
                            GURL /* origin_url */,
                            GURL /* top origin url */,
                            int /* storage_type */,
                            bool /* allowed */)

// Sent by the renderer process to check whether access to storage is
// granted by content settings.
IPC_MESSAGE_CONTROL5(QtWebEngineHostMsg_RequestStorageAccessAsync,
                     int /* render_frame_id */,
                     int /* request_id */,
                     GURL /* origin_url */,
                     GURL /* top origin url */,
                     int /* storage_type */)

