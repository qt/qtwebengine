/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef RESOURCE_DISPATCHER_HOST_DELEGATE_QT_H
#define RESOURCE_DISPATCHER_HOST_DELEGATE_QT_H

#include "content/public/browser/resource_dispatcher_host_delegate.h"
#include "extensions/buildflags/buildflags.h"

#include "web_contents_adapter_client.h"

namespace QtWebEngineCore {

class ResourceDispatcherHostDelegateQt : public content::ResourceDispatcherHostDelegate {
public:
    // If the stream will be rendered in a BrowserPlugin, |payload| will contain
    // the data that should be given to the old ResourceHandler to forward to the
    // renderer process.
    bool ShouldInterceptResourceAsStream(net::URLRequest *request,
                                         const std::string &mime_type,
                                         GURL *origin,
                                         std::string *payload) override;

    // Informs the delegate that a Stream was created. The Stream can be read from
    // the blob URL of the Stream, but can only be read once.
    void OnStreamCreated(net::URLRequest *request,
                         std::unique_ptr<content::StreamInfo> stream) override;
private:
#if BUILDFLAG(ENABLE_EXTENSIONS)
    struct StreamTargetInfo {
        std::string extension_id;
        std::string view_id;
    };
    std::map<net::URLRequest *, StreamTargetInfo> stream_target_info_;
#endif

};

} // namespace QtWebEngineCore

#endif // RESOURCE_DISPATCHER_HOST_DELEGATE_QT_H
