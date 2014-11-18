// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included file, no traditional include guard.

#include "content/public/common/common_param_traits.h"
#include "ipc/ipc_message_macros.h"

#include "user_script_data.h"

IPC_STRUCT_TRAITS_BEGIN(UserScriptData)
    IPC_STRUCT_TRAITS_MEMBER(source)
    IPC_STRUCT_TRAITS_MEMBER(url)
    IPC_STRUCT_TRAITS_MEMBER(injectionPoint)
    IPC_STRUCT_TRAITS_MEMBER(injectForSubframes)
    IPC_STRUCT_TRAITS_MEMBER(worldId)
    IPC_STRUCT_TRAITS_MEMBER(scriptId)
IPC_STRUCT_TRAITS_END()


#define IPC_MESSAGE_START QtMsgStart

//-----------------------------------------------------------------------------
// RenderView messages
// These are messages sent from the browser to the renderer process.

IPC_MESSAGE_ROUTED1(QtRenderViewObserver_FetchDocumentMarkup,
                    uint64 /* requestId */)

IPC_MESSAGE_ROUTED1(QtRenderViewObserver_FetchDocumentInnerText,
                    uint64 /* requestId */)

IPC_MESSAGE_ROUTED1(WebChannelIPCTransport_Message, std::vector<char> /*binaryJSON*/)

// User scripts messages
IPC_MESSAGE_ROUTED1(RenderViewObserverHelper_AddScript,
                    UserScriptData /* script */)
IPC_MESSAGE_ROUTED1(RenderViewObserverHelper_RemoveScript,
                    UserScriptData /* script */)
IPC_MESSAGE_ROUTED0(RenderViewObserverHelper_ClearScripts)

IPC_MESSAGE_CONTROL1(UserScriptController_AddScript, UserScriptData /* scriptContents */)
IPC_MESSAGE_CONTROL1(UserScriptController_RemoveScript, UserScriptData /* scriptContents */)
IPC_MESSAGE_CONTROL0(UserScriptController_ClearScripts)

//-----------------------------------------------------------------------------
// WebContents messages
// These are messages sent from the renderer back to the browser process.

IPC_MESSAGE_ROUTED2(QtRenderViewObserverHost_DidFetchDocumentMarkup,
                    uint64 /* requestId */,
                    base::string16 /* markup */)

IPC_MESSAGE_ROUTED2(QtRenderViewObserverHost_DidFetchDocumentInnerText,
                    uint64 /* requestId */,
                    base::string16 /* innerText */)

IPC_MESSAGE_ROUTED0(QtRenderViewObserverHost_DidFirstVisuallyNonEmptyLayout)

IPC_MESSAGE_ROUTED1(WebChannelIPCTransportHost_SendMessage, std::vector<char> /*binaryJSON*/)
