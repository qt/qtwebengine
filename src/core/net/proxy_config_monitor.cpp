// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// originally based on chrome/browser/net/proxy_config_monitor.cc
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "profile_qt.h"
#include "proxy_config_monitor.h"
#include "proxy_config_service_qt.h"

#include "content/public/browser/browser_task_traits.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "net/proxy_resolution/proxy_config_with_annotation.h"
#include "services/network/public/mojom/network_context.mojom.h"

#include <utility>

using content::BrowserThread;

ProxyConfigMonitor::ProxyConfigMonitor(PrefService *prefs)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    proxy_config_service_.reset(new ProxyConfigServiceQt(prefs, content::GetUIThreadTaskRunner({})));

    proxy_config_service_->AddObserver(this);
}

ProxyConfigMonitor::~ProxyConfigMonitor()
{
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI)
           || !BrowserThread::IsThreadInitialized(BrowserThread::UI));
    proxy_config_service_->RemoveObserver(this);
}

void ProxyConfigMonitor::AddToNetworkContextParams(
        network::mojom::NetworkContextParams *network_context_params)
{
    mojo::PendingRemote<network::mojom::ProxyConfigClient> proxy_config_client;
    network_context_params->proxy_config_client_receiver =
        proxy_config_client.InitWithNewPipeAndPassReceiver();
    proxy_config_client_set_.Add(std::move(proxy_config_client));

    poller_receiver_set_.Add(this,
                             network_context_params->proxy_config_poller_client.InitWithNewPipeAndPassReceiver());

    net::ProxyConfigWithAnnotation proxy_config;
    net::ProxyConfigService::ConfigAvailability availability =
            proxy_config_service_->GetLatestProxyConfig(&proxy_config);
    if (availability != net::ProxyConfigService::CONFIG_PENDING)
        network_context_params->initial_proxy_config = proxy_config;
}

void ProxyConfigMonitor::OnProxyConfigChanged(
        const net::ProxyConfigWithAnnotation &config,
        net::ProxyConfigService::ConfigAvailability availability)
{
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI)
           || !BrowserThread::IsThreadInitialized(BrowserThread::UI));
    for (const auto &proxy_config_client : proxy_config_client_set_) {
        switch (availability) {
        case net::ProxyConfigService::CONFIG_VALID:
            proxy_config_client->OnProxyConfigUpdated(config);
            break;
        case net::ProxyConfigService::CONFIG_UNSET:
            proxy_config_client->OnProxyConfigUpdated(net::ProxyConfigWithAnnotation::CreateDirect());
            break;
        case net::ProxyConfigService::CONFIG_PENDING:
            NOTREACHED();
            break;
        }
    }
}

void ProxyConfigMonitor::OnLazyProxyConfigPoll()
{
    proxy_config_service_->OnLazyPoll();
}
