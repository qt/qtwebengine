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

// based on chrome/browser/net/system_network_context_manager.h:
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYSTEM_NETWORK_CONTEXT_MANAGER_H_
#define SYSTEM_NETWORK_CONTEXT_MANAGER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/optional.h"
#include "services/network/public/mojom/host_resolver.mojom-forward.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "services/network/public/mojom/network_service.mojom-forward.h"
#include "services/network/public/mojom/ssl_config.mojom-forward.h"
#include "services/network/public/mojom/url_loader_factory.mojom-forward.h"

#include "net/proxy_config_monitor.h"

namespace network {
namespace mojom {
class URLLoaderFactory;
}
class SharedURLLoaderFactory;
} // namespace network

namespace net_log {
class NetExportFileWriter;
}

// Responsible for creating and managing access to the system NetworkContext.
// Lives on the UI thread. The NetworkContext this owns is intended for requests
// not associated with a profile. It stores no data on disk, and has no HTTP
// cache, but it does have ephemeral cookie and channel ID stores. It also does
// not have access to HTTP proxy auth information the user has entered or that
// comes from extensions, and similarly, has no extension-provided per-profile
// proxy configuration information.
//
// This class is also responsible for configuring global NetworkService state.
//
// The "system" NetworkContext will either share a URLRequestContext with
// IOThread's SystemURLRequestContext and be part of IOThread's NetworkService
// (If the network service is disabled) or be an independent NetworkContext
// using the actual network service.
//
// This class is intended to eventually replace IOThread. Handling the two cases
// differently allows this to be used in production without breaking anything or
// requiring two separate paths, while IOThread consumers slowly transition over
// to being compatible with the network service.
class SystemNetworkContextManager
{
public:
    ~SystemNetworkContextManager();

    // Creates the global instance of SystemNetworkContextManager. If an
    // instance already exists, this will cause a DCHECK failure.
    static SystemNetworkContextManager *CreateInstance();

    // Gets the global SystemNetworkContextManager instance.
    static SystemNetworkContextManager *GetInstance();

    // Destroys the global SystemNetworkContextManager instance.
    static void DeleteInstance();

    // Returns the System NetworkContext. May only be called after SetUp(). Does
    // any initialization of the NetworkService that may be needed when first
    // called.
    network::mojom::NetworkContext *GetContext();

    // Returns a URLLoaderFactory owned by the SystemNetworkContextManager that is
    // backed by the SystemNetworkContext. Allows sharing of the URLLoaderFactory.
    // Prefer this to creating a new one.  Call Clone() on the value returned by
    // this method to get a URLLoaderFactory that can be used on other threads.
    network::mojom::URLLoaderFactory *GetURLLoaderFactory();

    // Returns a SharedURLLoaderFactory owned by the SystemNetworkContextManager
    // that is backed by the SystemNetworkContext.
    scoped_refptr<network::SharedURLLoaderFactory> GetSharedURLLoaderFactory();

    // Called when content creates a NetworkService. Creates the
    // SystemNetworkContext, if the network service is enabled.
    void OnNetworkServiceCreated(network::mojom::NetworkService *network_service);

    // Populates |initial_ssl_config| and |ssl_config_client_request| members of
    // |network_context_params|. As long as the SystemNetworkContextManager
    // exists, any NetworkContext created with the params will continue to get
    // SSL configuration updates.
    void AddSSLConfigToNetworkContextParams(network::mojom::NetworkContextParams *network_context_params);

    // Returns default set of parameters for configuring the network service.
    network::mojom::NetworkContextParamsPtr CreateDefaultNetworkContextParams();

private:
    class URLLoaderFactoryForSystem;

    explicit SystemNetworkContextManager();

    // Creates parameters for the NetworkContext. May only be called once, since
    // it initializes some class members.
    network::mojom::NetworkContextParamsPtr CreateNetworkContextParams();

    //  ProxyConfigMonitor proxy_config_monitor_;

    // NetworkContext using the network service, if the network service is
    // enabled. nullptr, otherwise.
    mojo::Remote<network::mojom::NetworkContext> network_service_network_context_;

    // URLLoaderFactory backed by the NetworkContext returned by GetContext(), so
    // consumers don't all need to create their own factory.
    scoped_refptr<URLLoaderFactoryForSystem> shared_url_loader_factory_;
    mojo::Remote<network::mojom::URLLoaderFactory> url_loader_factory_;

    ProxyConfigMonitor proxy_config_monitor_;

    DISALLOW_COPY_AND_ASSIGN(SystemNetworkContextManager);
};

#endif // SYSTEM_NETWORK_CONTEXT_MANAGER_H_
