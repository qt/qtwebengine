/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// based on chrome/browser/renderer_host/pepper/pepper_flash_clipboard_message_filter.h
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PEPPER_FLASH_CLIPBOARD_MESSAGE_FILTER_QT_H
#define PEPPER_FLASH_CLIPBOARD_MESSAGE_FILTER_QT_H

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "ppapi/host/resource_message_filter.h"
#include "ppapi/shared_impl/flash_clipboard_format_registry.h"

namespace ppapi {
namespace host {
struct HostMessageContext;
}
}

namespace ui {
class ScopedClipboardWriter;
}

namespace QtWebEngineCore {

// Resource message filter for accessing the clipboard in Pepper. Pepper
// supports reading/writing custom formats from the clipboard. Currently, all
// custom formats that are read/written from the clipboard through pepper are
// stored in a single real clipboard format (in the same way the "web custom"
// clipboard formats are). This is done so that we don't have to have use real
// clipboard types for each custom clipboard format which may be a limited
// resource on a particular platform.
class PepperFlashClipboardMessageFilter : public ppapi::host::ResourceMessageFilter {
public:
    PepperFlashClipboardMessageFilter();

protected:
    // ppapi::host::ResourceMessageFilter overrides.
    scoped_refptr<base::TaskRunner> OverrideTaskRunnerForMessage(const IPC::Message& msg) override;
    int32_t OnResourceMessageReceived(const IPC::Message& msg, ppapi::host::HostMessageContext* context) override;

private:
    ~PepperFlashClipboardMessageFilter() override;

    int32_t OnMsgRegisterCustomFormat(ppapi::host::HostMessageContext* host_context, const std::string& format_name);
    int32_t OnMsgIsFormatAvailable(ppapi::host::HostMessageContext* host_context,
                                   uint32_t clipboard_type,
                                   uint32_t format);
    int32_t OnMsgReadData(ppapi::host::HostMessageContext* host_context,
                          uint32_t clipoard_type,
                          uint32_t format);
    int32_t OnMsgWriteData(ppapi::host::HostMessageContext* host_context,
                           uint32_t clipboard_type,
                           const std::vector<uint32_t>& formats,
                           const std::vector<std::string>& data);
    int32_t OnMsgGetSequenceNumber(ppapi::host::HostMessageContext* host_context,
                                   uint32_t clipboard_type);

    int32_t WriteClipboardDataItem(uint32_t format,
                                   const std::string& data,
                                   ui::ScopedClipboardWriter* scw);

    ppapi::FlashClipboardFormatRegistry m_custom_formats;

    DISALLOW_COPY_AND_ASSIGN(PepperFlashClipboardMessageFilter);
};

}  // namespace QtWebEngineCore

#endif  // PEPPER_FLASH_CLIPBOARD_MESSAGE_FILTER_QT_H
