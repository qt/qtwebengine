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

#ifndef PLUGIN_RESPONSE_INTERCEPTOR_URL_LOADER_THROTTLE_H_
#define PLUGIN_RESPONSE_INTERCEPTOR_URL_LOADER_THROTTLE_H_

#include "base/macros.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"

namespace content {
class BrowserContext;
class ResourceContext;
}

namespace QtWebEngineCore {

class PluginResponseInterceptorURLLoaderThrottle : public blink::URLLoaderThrottle
{
public:
    PluginResponseInterceptorURLLoaderThrottle(content::ResourceContext *resource_context,
                                               int resource_type, int frame_tree_node_id);
    PluginResponseInterceptorURLLoaderThrottle(content::BrowserContext *browser_context,
                                               int resource_type, int frame_tree_node_id);
    ~PluginResponseInterceptorURLLoaderThrottle() override = default;

private:
    // content::URLLoaderThrottle overrides;
    void WillProcessResponse(const GURL &response_url, network::mojom::URLResponseHead *response_head, bool *defer) override;

    content::ResourceContext *m_resource_context = nullptr;
    content::BrowserContext *m_browser_context = nullptr;
    const int m_resource_type;
    const int m_frame_tree_node_id;

    DISALLOW_COPY_AND_ASSIGN(PluginResponseInterceptorURLLoaderThrottle);
};

} // namespace QtWebEngineCore

#endif // PLUGIN_RESPONSE_INTERCEPTOR_URL_LOADER_THROTTLE_H_
