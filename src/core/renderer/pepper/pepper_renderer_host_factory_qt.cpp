// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// This is based on chrome/renderer/pepper/chrome_renderer_pepper_host_factory.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "pepper_renderer_host_factory_qt.h"
#include "qtwebenginecoreglobal_p.h"

#include "base/memory/ptr_util.h"
#include "chrome/renderer/pepper/pepper_uma_host.h"
#if QT_CONFIG(webengine_printing_and_pdf)
#include "chrome/renderer/pepper/pepper_flash_font_file_host.h"
#include "components/pdf/renderer/pepper_pdf_host.h"
#endif // QT_CONFIG(webengine_printing_and_pdf)
#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/host/resource_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/ppapi_message_utils.h"
#include "ppapi/shared_impl/ppapi_permissions.h"


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

    // TODO(raymes): PDF also needs access to the FlashFontFileHost currently.
    // We should either rename PPB_FlashFont_File to PPB_FontFile_Private or get
    // rid of its use in PDF if possible.
#if QT_CONFIG(webengine_printing_and_pdf)
    if (host_->GetPpapiHost()->permissions().HasPermission(ppapi::PERMISSION_FLASH)
        || host_->GetPpapiHost()->permissions().HasPermission(ppapi::PERMISSION_PDF)) {
        switch (message.type()) {
        case PpapiHostMsg_FlashFontFile_Create::ID: {
            ppapi::proxy::SerializedFontDescription description;
            PP_PrivateFontCharset charset;
            if (ppapi::UnpackMessage<PpapiHostMsg_FlashFontFile_Create>(message, &description, &charset))
                return base::WrapUnique(new PepperFlashFontFileHost(host_, instance, resource, description, charset));
            break;
        }
        }
    }

    if (host_->GetPpapiHost()->permissions().HasPermission(ppapi::PERMISSION_PDF)) {
        switch (message.type()) {
        case PpapiHostMsg_PDF_Create::ID:
            return std::make_unique<pdf::PepperPDFHost>(host_, instance, resource);
        }
    }
#endif // QT_CONFIG(webengine_printing_and_pdf)

    // Create a default ResourceHost for this message type to suppress
    // "Failed to create PPAPI resource host" console error message.
    switch (message.type()) {
    case PpapiHostMsg_UMA_Create::ID:
        return std::make_unique<ppapi::host::ResourceHost>(host_->GetPpapiHost(), instance, resource);
    }

    return nullptr;
}

} // QtWebEngineCore
