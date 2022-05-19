// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// This is based on chrome/renderer/pepper/chrome_renderer_pepper_host_factory.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "pepper_renderer_host_factory_qt.h"
#include "qtwebenginecoreglobal_p.h"

#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/host/resource_host.h"
#include "ppapi/proxy/ppapi_messages.h"


namespace QtWebEngineCore {

PepperRendererHostFactoryQt::PepperRendererHostFactoryQt(content::RendererPpapiHost* host)
    : host_(host)
{
}

PepperRendererHostFactoryQt::~PepperRendererHostFactoryQt()
{
}

std::unique_ptr<ppapi::host::ResourceHost> PepperRendererHostFactoryQt::CreateResourceHost(
        ppapi::host::PpapiHost* host,
        PP_Resource resource,
        PP_Instance instance,
        const IPC::Message& message)
{
    DCHECK_EQ(host_->GetPpapiHost(), host);

    if (!host_->IsValidInstance(instance))
        return nullptr;

    // Create a default ResourceHost for this message type to suppress
    // "Failed to create PPAPI resource host" console error message.
    switch (message.type()) {
    case PpapiHostMsg_UMA_Create::ID:
        return std::make_unique<ppapi::host::ResourceHost>(host_->GetPpapiHost(), instance, resource);
    }

    return nullptr;
}

} // QtWebEngineCore
