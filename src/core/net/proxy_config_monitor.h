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

// originally based on chrome/browser/net/proxy_config_monitor.h
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PROXY_CONFIG_MONITOR_H
#define PROXY_CONFIG_MONITOR_H

#include <memory>
#include <string>

#include "base/macros.h"
#include "build/buildflag.h"
#include "extensions/buildflags/buildflags.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "net/proxy_resolution/proxy_config_service.h"
#include "services/network/public/mojom/network_context.mojom-forward.h"
#include "services/network/public/mojom/network_service.mojom-forward.h"
#include "services/network/public/mojom/proxy_config.mojom-forward.h"
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

    DISALLOW_COPY_AND_ASSIGN(ProxyConfigMonitor);
};

#endif // !PROXY_CONFIG_MONITOR_H
