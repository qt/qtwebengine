/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#include "qtwebenginecoreglobal_p.h"
#include "sandbox/win/src/process_mitigations.h"
#include "sandbox/win/src/sandbox_factory.h"

#ifndef NDEBUG
#include "base/command_line.h"
#include "base/logging.h"
#endif

namespace QtWebEngineSandbox {
// A duplicate of the function by same name in startup_helper_win.cc
static void InitializeSandboxInfo(sandbox::SandboxInterfaceInfo *info)
{
    info->broker_services = sandbox::SandboxFactory::GetBrokerServices();
    if (!info->broker_services) {
        info->target_services = sandbox::SandboxFactory::GetTargetServices();
    } else {
        // Ensure the proper mitigations are enforced for the browser process.
        sandbox::ApplyProcessMitigationsToCurrentProcess(
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
