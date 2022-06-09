// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PLUGIN_RESPONSE_INTERCEPTOR_URL_LOADER_THROTTLE_H_
#define PLUGIN_RESPONSE_INTERCEPTOR_URL_LOADER_THROTTLE_H_

#include "base/memory/weak_ptr.h"
#include "services/network/public/mojom/fetch_api.mojom-shared.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"

namespace content {
class BrowserContext;
}

namespace QtWebEngineCore {

class PluginResponseInterceptorURLLoaderThrottle : public blink::URLLoaderThrottle
{
public:
    PluginResponseInterceptorURLLoaderThrottle(network::mojom::RequestDestination request_destination,
                                               int frame_tree_node_id);
    ~PluginResponseInterceptorURLLoaderThrottle() override = default;

private:
    // content::URLLoaderThrottle overrides;
    void WillProcessResponse(const GURL &response_url, network::mojom::URLResponseHead *response_head, bool *defer) override;

    // Resumes loading for an intercepted response. This would give the extension
    // layer chance to initialize its browser side state.
    void ResumeLoad();

    const network::mojom::RequestDestination m_request_destination;
    const int m_frame_tree_node_id;

    base::WeakPtrFactory<PluginResponseInterceptorURLLoaderThrottle>
        weak_factory_{this};
};

} // namespace QtWebEngineCore

#endif // PLUGIN_RESPONSE_INTERCEPTOR_URL_LOADER_THROTTLE_H_
