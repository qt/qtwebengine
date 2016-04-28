/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

// This is based on chrome/browser/renderer_host/pepper/chrome_browser_pepper_host_factory.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pepper_host_factory_qt.h"

#include "build/build_config.h"
#include "content/public/browser/browser_ppapi_host.h"
#include "ppapi/host/message_filter_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/host/resource_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

#include "pepper_flash_browser_host_qt.h"
#include "pepper_isolated_file_system_message_filter.h"

using ppapi::host::MessageFilterHost;
using ppapi::host::ResourceHost;
using ppapi::host::ResourceMessageFilter;

namespace QtWebEngineCore {

PepperHostFactoryQt::PepperHostFactoryQt(content::BrowserPpapiHost* host)
    : host_(host)
{
}

PepperHostFactoryQt::~PepperHostFactoryQt() {}

scoped_ptr<ppapi::host::ResourceHost> PepperHostFactoryQt::CreateResourceHost(ppapi::host::PpapiHost* host,
        PP_Resource resource,
        PP_Instance instance,
        const IPC::Message& message)
{
    DCHECK(host == host_->GetPpapiHost());


    if (!host_->IsValidInstance(instance))
        return scoped_ptr<ppapi::host::ResourceHost>();

    if (host_->GetPpapiHost()->permissions().HasPermission(ppapi::PERMISSION_FLASH)
            && message.type() == PpapiHostMsg_Flash_Create::ID)
        return scoped_ptr<ppapi::host::ResourceHost>(
                    new PepperFlashBrowserHostQt(host_,
                                                 instance,
                                                 resource));

    // Permissions for the following interfaces will be checked at the
    // time of the corresponding instance's methods calls (because
    // permission check can be performed only on the UI
    // thread). Currently these interfaces are available only for
    // whitelisted apps which may not have access to the other private
    // interfaces.
    if (message.type() == PpapiHostMsg_IsolatedFileSystem_Create::ID) {
        PepperIsolatedFileSystemMessageFilter* isolated_fs_filter = PepperIsolatedFileSystemMessageFilter::Create(instance, host_);
        if (!isolated_fs_filter)
            return scoped_ptr<ResourceHost>();
        return scoped_ptr<ResourceHost>(new MessageFilterHost(host, instance, resource, isolated_fs_filter));
    }

    return scoped_ptr<ppapi::host::ResourceHost>();
}

}  // namespace QtWebEngineCore
