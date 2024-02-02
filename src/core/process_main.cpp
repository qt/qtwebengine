// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtwebenginecoreglobal_p.h"
#include "content_main_delegate_qt.h"
#include "content/public/app/content_main.h"
#if BUILDFLAG(IS_WIN)
#include "sandbox/win/src/sandbox_types.h"
#include "content/public/app/sandbox_helper_win.h"
#elif BUILDFLAG(IS_MAC)
#include "sandbox/mac/seatbelt_exec.h"
#endif

namespace QtWebEngineCore {

/*! \internal */
int processMain(int argc, const char **argv)
{
    ContentMainDelegateQt delegate;
    content::ContentMainParams params(&delegate);

#if BUILDFLAG(IS_WIN)
    HINSTANCE instance_handle = NULL;
    params.sandbox_info = QtWebEngineSandbox::staticSandboxInterfaceInfo();
    sandbox::SandboxInterfaceInfo sandbox_info = {nullptr};
    if (!params.sandbox_info) {
        content::InitializeSandboxInfo(&sandbox_info);
        params.sandbox_info = &sandbox_info;
    }
    params.instance = instance_handle;
#else
    params.argc = argc;
    params.argv = argv;
#endif // IS_WIN
#if BUILDFLAG(IS_MAC)
  sandbox::SeatbeltExecServer::CreateFromArgumentsResult seatbelt =
          sandbox::SeatbeltExecServer::CreateFromArguments(argv[0], argc, const_cast<char**>(argv));
  if (seatbelt.sandbox_required) {
    CHECK(seatbelt.server->InitializeSandbox());
  }
#endif  // IS_MAC
    return content::ContentMain(std::move(params));
}

} // namespace QtWebEngineCore
