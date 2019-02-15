/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "command_line_pref_store_qt.h"

#include "chrome/common/chrome_switches.h"
#include "components/proxy_config/proxy_config_dictionary.h"
#include "components/proxy_config/proxy_config_pref_names.h"
#include "content/public/common/content_switches.h"
#include <QDebug>

CommandLinePrefStoreQt::CommandLinePrefStoreQt(const base::CommandLine *commandLine)
    : CommandLinePrefStore(commandLine)
{

    if (commandLine->HasSwitch(switches::kNoProxyServer)) {
        SetValue(proxy_config::prefs::kProxy,
                 std::make_unique<base::Value>(ProxyConfigDictionary::CreateDirect()),
                 WriteablePrefStore::DEFAULT_PREF_WRITE_FLAGS);
    } else if (commandLine->HasSwitch(switches::kProxyPacUrl)) {
        std::string pac_script_url =
                commandLine->GetSwitchValueASCII(switches::kProxyPacUrl);
        SetValue(proxy_config::prefs::kProxy,
                 std::make_unique<base::Value>(ProxyConfigDictionary::CreatePacScript(
                         pac_script_url, false)),
                 WriteablePrefStore::DEFAULT_PREF_WRITE_FLAGS);
    } else if (commandLine->HasSwitch(switches::kProxyAutoDetect)) {
        SetValue(proxy_config::prefs::kProxy,
                 std::make_unique<base::Value>(
                         ProxyConfigDictionary::CreateAutoDetect()),
                 WriteablePrefStore::DEFAULT_PREF_WRITE_FLAGS);
    } else if (commandLine->HasSwitch(switches::kProxyServer)) {
        std::string proxy_server =
                commandLine->GetSwitchValueASCII(switches::kProxyServer);
        std::string bypass_list =
                commandLine->GetSwitchValueASCII(switches::kProxyBypassList);
        SetValue(
                proxy_config::prefs::kProxy,
                std::make_unique<base::Value>(ProxyConfigDictionary::CreateFixedServers(
                        proxy_server, bypass_list)),
                WriteablePrefStore::DEFAULT_PREF_WRITE_FLAGS);
    }

    if (commandLine->HasSwitch(switches::kNoProxyServer) && (commandLine->HasSwitch(switches::kProxyAutoDetect) || commandLine->HasSwitch(switches::kProxyServer) || commandLine->HasSwitch(switches::kProxyPacUrl) || commandLine->HasSwitch(switches::kProxyBypassList))) {
        qWarning("Additional command-line proxy switches specified when --%s was also specified",
                 qPrintable(switches::kNoProxyServer));
    }
}

CommandLinePrefStoreQt::~CommandLinePrefStoreQt() = default;
