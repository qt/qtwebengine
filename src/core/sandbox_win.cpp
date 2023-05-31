// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtwebenginecoreglobal_p.h"
#include "sandbox/win/src/process_mitigations.h"
#include "sandbox/win/src/sandbox_factory.h"

#ifndef NDEBUG
#include "base/command_line.h"
#include "base/logging.h"
#endif

namespace QtWebEngineSandbox {
// A duplicate of the function by same name in sandbox_helper_win.cc
static void InitializeSandboxInfo(sandbox::SandboxInterfaceInfo *info)
{
    info->broker_services = sandbox::SandboxFactory::GetBrokerServices();
    if (!info->broker_services) {
        info->target_services = sandbox::SandboxFactory::GetTargetServices();
    } else {
        // Ensure the proper mitigations are enforced for the browser process.
        info->broker_services->RatchetDownSecurityMitigations(
            sandbox::MITIGATION_DEP | sandbox::MITIGATION_DEP_NO_ATL_THUNK |
            sandbox::MITIGATION_HARDEN_TOKEN_IL_POLICY);
        // Note: these mitigations are "post-startup".  Some mitigations that need
        // to be enabled sooner (e.g. MITIGATION_EXTENSION_POINT_DISABLE) are done
        // so in Chrome_ELF.
    }
}

// Initializes the staticlib copy of //base and //sandbox used for Windows sandboxing
void initializeStaticCopy(int argc, const char **argv)
{
#ifndef NDEBUG
    // Initialize //base for debugging
    base::CommandLine::Init(argc, argv);
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    logging::InitLogging(settings);
#endif
    sandbox::SandboxInterfaceInfo *info = new sandbox::SandboxInterfaceInfo();
    memset(info, 0, sizeof(sandbox::SandboxInterfaceInfo));
    InitializeSandboxInfo(info);
    QtWebEngineSandbox::staticSandboxInterfaceInfo(info);
}
} // namespace QtWebEngineSandbox
