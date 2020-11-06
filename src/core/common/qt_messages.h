// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

// Multiply-included file, no traditional include guard.

#include "base/optional.h"
#include "content/public/common/common_param_traits.h"
#include "content/public/common/webplugininfo.h"
#include "ipc/ipc_message_macros.h"
#include "ppapi/buildflags/buildflags.h"

#define IPC_MESSAGE_START QtMsgStart

//-----------------------------------------------------------------------------
// RenderView messages
// These are messages sent from the browser to the renderer process.

// Tells the renderer whether or not a storage access has been allowed.
IPC_MESSAGE_ROUTED2(QtWebEngineMsg_RequestStorageAccessAsyncResponse,
                    int  /* request_id */,
                    bool /* allowed */)

//-----------------------------------------------------------------------------
// WebContents messages
// These are messages sent from the renderer back to the browser process.

IPC_MESSAGE_ROUTED0(RenderViewObserverHostQt_DidFirstVisuallyNonEmptyLayout)

//-----------------------------------------------------------------------------
// Misc messages
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

