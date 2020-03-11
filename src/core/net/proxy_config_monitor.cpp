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

// originally based on chrome/browser/net/proxy_config_monitor.cc
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "profile_qt.h"
#include "proxy_config_monitor.h"
#include "proxy_config_service_qt.h"

#include "base/strings/utf_string_conversions.h"
#include "base/task/post_task.h"
#include "build/build_config.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/associated_interface_ptr.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "net/proxy_resolution/proxy_resolution_service.h"
#include "services/network/public/mojom/network_context.mojom.h"

#include <utility>

using content::BrowserThread;

ProxyConfigMonitor::ProxyConfigMonitor(PrefService *prefs)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    proxy_config_service_.reset(
            new ProxyConfigServiceQt(
                    prefs, base::CreateSingleThreadTaskRunner({ BrowserThread::UI })));

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
