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

#ifndef PEPPER_FLASH_RENDERER_HOST_QT_H
#define PEPPER_FLASH_RENDERER_HOST_QT_H

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/host/resource_host.h"

struct PP_Rect;

namespace ppapi {
struct URLRequestInfoData;
}

namespace ppapi {
namespace proxy {
struct PPBFlash_DrawGlyphs_Params;
}
}

namespace content {
class RendererPpapiHost;
}

namespace QtWebEngineCore {

class PepperFlashRendererHostQt : public ppapi::host::ResourceHost {
public:
    PepperFlashRendererHostQt(content::RendererPpapiHost* host,
                              PP_Instance instance,
                              PP_Resource resource);
    ~PepperFlashRendererHostQt() override;

    // ppapi::host::ResourceHost override.
    int32_t OnResourceMessageReceived(
            const IPC::Message& msg,
            ppapi::host::HostMessageContext* context) override;

private:
    int32_t OnGetProxyForURL(ppapi::host::HostMessageContext* host_context,
                             const std::string& url);
    int32_t OnSetInstanceAlwaysOnTop(
            ppapi::host::HostMessageContext* host_context,
            bool on_top);
    int32_t OnDrawGlyphs(ppapi::host::HostMessageContext* host_context,
                         ppapi::proxy::PPBFlash_DrawGlyphs_Params params);
    int32_t OnNavigate(ppapi::host::HostMessageContext* host_context,
                       const ppapi::URLRequestInfoData& data,
                       const std::string& target,
                       bool from_user_action);
    int32_t OnIsRectTopmost(ppapi::host::HostMessageContext* host_context,
                            const PP_Rect& rect);
    int32_t OnInvokePrinting(ppapi::host::HostMessageContext* host_context);

    // A stack of ReplyMessageContexts to track Navigate() calls which have not
    // yet been replied to.
    std::vector<ppapi::host::ReplyMessageContext> navigate_replies_;

    content::RendererPpapiHost* host_;
    base::WeakPtrFactory<PepperFlashRendererHostQt> weak_factory_;

    DISALLOW_COPY_AND_ASSIGN(PepperFlashRendererHostQt);
};

} //QtWebEngineCore
#endif  // PEPPER_FLASH_RENDERER_HOST_QT_H
