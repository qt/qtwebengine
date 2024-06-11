// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// originally based on chrome/browser/net/proxy_config_monitor.h
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PROXY_CONFIG_MONITOR_H
#define PROXY_CONFIG_MONITOR_H

#include <memory>

#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "net/proxy_resolution/proxy_config_service.h"
#include "services/network/public/mojom/network_context.mojom-forward.h"
#include "services/network/public/mojom/proxy_config_with_annotation.mojom.h"

namespace net {
class ProxyConfigWithAnnotation;
}

class PrefService;
class ProxyConfigServiceQt;

// Tracks the ProxyConfig to use, and passes any updates to a NetworkContext's
// ProxyConfigClient. This also responds to errors related to proxy settings
// from the NetworkContext, and forwards them any listening Chrome extensions
// associated with the profile.
class ProxyConfigMonitor : public net::ProxyConfigService::Observer,
                           public network::mojom::ProxyConfigPollerClient
{
public:
    explicit ProxyConfigMonitor(PrefService *prefs = nullptr);

    ~ProxyConfigMonitor() override;

    // Populates proxy-related fields of |network_context_params|. Updated
    // ProxyConfigs will be sent to a NetworkContext created with those params
    // whenever the configuration changes. Can be called more than once to inform
    // multiple NetworkContexts of proxy changes.
    void AddToNetworkContextParams(network::mojom::NetworkContextParams *network_context_params);

private:
    // net::ProxyConfigService::Observer implementation:
    void OnProxyConfigChanged(const net::ProxyConfigWithAnnotation &config,
                              net::ProxyConfigService::ConfigAvailability availability) override;

    // network::mojom::ProxyConfigPollerClient implementation:
    void OnLazyProxyConfigPoll() override;

    std::unique_ptr<ProxyConfigServiceQt> proxy_config_service_;

    mojo::ReceiverSet<network::mojom::ProxyConfigPollerClient> poller_receiver_set_;
    mojo::RemoteSet<network::mojom::ProxyConfigClient> proxy_config_client_set_;
};

#endif // PROXY_CONFIG_MONITOR_H
