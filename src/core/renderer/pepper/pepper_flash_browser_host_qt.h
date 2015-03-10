/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef PEPPER_FLASH_BROWSER_HOST_QT_H
#define PEPPER_FLASH_BROWSER_HOST_QT_H

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/host/resource_host.h"

namespace base {
class Time;
}

namespace content {
class BrowserPpapiHost;
class ResourceContext;
}

class GURL;

namespace QtWebEngineCore {

class PepperFlashBrowserHostQt : public ppapi::host::ResourceHost {
public:
    PepperFlashBrowserHostQt(content::BrowserPpapiHost* host,
                             PP_Instance instance,
                             PP_Resource resource);
    ~PepperFlashBrowserHostQt() override;

    // ppapi::host::ResourceHost override.
    int32_t OnResourceMessageReceived(
            const IPC::Message& msg,
            ppapi::host::HostMessageContext* context) override;

private:
    int32_t OnUpdateActivity(ppapi::host::HostMessageContext* host_context);
    int32_t OnGetLocalTimeZoneOffset(
            ppapi::host::HostMessageContext* host_context,
            const base::Time& t);
    int32_t OnGetLocalDataRestrictions(ppapi::host::HostMessageContext* context);

    void GetLocalDataRestrictions(ppapi::host::ReplyMessageContext reply_context,
                                  const GURL& document_url,
                                  const GURL& plugin_url);

    content::BrowserPpapiHost* host_;
    int render_process_id_;
    base::WeakPtrFactory<PepperFlashBrowserHostQt> weak_factory_;

    DISALLOW_COPY_AND_ASSIGN(PepperFlashBrowserHostQt);
};

}  // namespace QtWebEngineCore

#endif  // PEPPER_FLASH_BROWSER_HOST_QT_H
