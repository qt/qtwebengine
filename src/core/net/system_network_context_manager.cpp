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

// based on chrome/browser/net/system_network_context_manager.cc:
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/system_network_context_manager.h"

#include <set>
#include <unordered_map>
#include <utility>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/sequence_checker.h"
#include "base/strings/string_split.h"
#include "base/task/post_task.h"
#include "base/values.h"
#include "build/build_config.h"
#include "chrome/browser/net/chrome_mojo_proxy_resolver_factory.h"
#include "chrome/common/chrome_switches.h"
#include "components/certificate_transparency/ct_known_logs.h"
#include "components/network_session_configurator/common/network_features.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cors_exempt_headers.h"
#include "content/public/browser/network_service_instance.h"
#include "content/public/common/content_features.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/service_names.mojom.h"
#include "content/public/common/user_agent.h"
#include "mojo/public/cpp/bindings/associated_interface_ptr.h"
#include "net/dns/public/util.h"
#include "net/net_buildflags.h"
#include "net/third_party/uri_template/uri_template.h"
#include "services/network/network_service.h"
#include "services/network/public/cpp/cross_thread_pending_shared_url_loader_factory.h"
#include "services/network/public/cpp/features.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/mojom/host_resolver.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "services/proxy_resolver/public/mojom/proxy_resolver.mojom.h"
#include "url/gurl.h"

namespace {

// The global instance of the SystemNetworkContextmanager.
SystemNetworkContextManager *g_system_network_context_manager = nullptr;

network::mojom::HttpAuthStaticParamsPtr CreateHttpAuthStaticParams()
{
    network::mojom::HttpAuthStaticParamsPtr auth_static_params = network::mojom::HttpAuthStaticParams::New();

    auth_static_params->supported_schemes = { "basic", "digest", "ntlm", "negotiate" };

    return auth_static_params;
}

network::mojom::HttpAuthDynamicParamsPtr CreateHttpAuthDynamicParams()
{
    network::mojom::HttpAuthDynamicParamsPtr auth_dynamic_params = network::mojom::HttpAuthDynamicParams::New();

    auto *command_line = base::CommandLine::ForCurrentProcess();
    auth_dynamic_params->server_allowlist = command_line->GetSwitchValueASCII(switches::kAuthServerWhitelist);
//    auth_dynamic_params->delegate_allowlist = command_line->GetSwitchValueASCII(switches::kAuthNegotiateDelegateWhitelist);
//    auth_dynamic_params->enable_negotiate_port = command_line->HasSwitch(switches::kEnableAuthNegotiatePort);

    return auth_dynamic_params;
}

} // namespace

// SharedURLLoaderFactory backed by a SystemNetworkContextManager and its
// network context. Transparently handles crashes.
class SystemNetworkContextManager::URLLoaderFactoryForSystem : public network::SharedURLLoaderFactory
{
public:
    explicit URLLoaderFactoryForSystem(SystemNetworkContextManager *manager) : manager_(manager)
    {
        DETACH_FROM_SEQUENCE(sequence_checker_);
    }

    // mojom::URLLoaderFactory implementation:

    void CreateLoaderAndStart(mojo::PendingReceiver<network::mojom::URLLoader> receiver,
                              int32_t routing_id,
                              int32_t request_id,
                              uint32_t options,
                              const network::ResourceRequest &url_request,
                              mojo::PendingRemote<network::mojom::URLLoaderClient> client,
                              const net::MutableNetworkTrafficAnnotationTag &traffic_annotation) override
    {
        DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
        if (!manager_)
            return;
        manager_->GetURLLoaderFactory()->CreateLoaderAndStart(
                    std::move(receiver), routing_id, request_id, options, url_request,
                    std::move(client), traffic_annotation);
    }

    void Clone(mojo::PendingReceiver<network::mojom::URLLoaderFactory> receiver) override
    {
        if (!manager_)
            return;
        manager_->GetURLLoaderFactory()->Clone(std::move(receiver));
    }

    // SharedURLLoaderFactory implementation:
    std::unique_ptr<network::PendingSharedURLLoaderFactory> Clone() override
    {
        DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
        return std::make_unique<network::CrossThreadPendingSharedURLLoaderFactory>(this);
    }

    void Shutdown() { manager_ = nullptr; }

private:
    friend class base::RefCounted<URLLoaderFactoryForSystem>;
    ~URLLoaderFactoryForSystem() override {}

    SEQUENCE_CHECKER(sequence_checker_);
    SystemNetworkContextManager *manager_;

    DISALLOW_COPY_AND_ASSIGN(URLLoaderFactoryForSystem);
};

network::mojom::NetworkContext *SystemNetworkContextManager::GetContext()
{
    if (!network_service_network_context_ ||
        !network_service_network_context_.is_connected()) {
        // This should call into OnNetworkServiceCreated(), which will re-create
        // the network service, if needed. There's a chance that it won't be
        // invoked, if the NetworkContext has encountered an error but the
        // NetworkService has not yet noticed its pipe was closed. In that case,
        // trying to create a new NetworkContext would fail, anyways, and hopefully
        // a new NetworkContext will be created on the next GetContext() call.
        content::GetNetworkService();
        DCHECK(network_service_network_context_);
    }
    return network_service_network_context_.get();
}

network::mojom::URLLoaderFactory *SystemNetworkContextManager::GetURLLoaderFactory()
{
    // Create the URLLoaderFactory as needed.
    if (url_loader_factory_ && url_loader_factory_.is_connected()) {
        return url_loader_factory_.get();
    }

    network::mojom::URLLoaderFactoryParamsPtr params = network::mojom::URLLoaderFactoryParams::New();
    params->process_id = network::mojom::kBrowserProcessId;
    params->is_corb_enabled = false;
    GetContext()->CreateURLLoaderFactory(url_loader_factory_.BindNewPipeAndPassReceiver(), std::move(params));
    return url_loader_factory_.get();
}

scoped_refptr<network::SharedURLLoaderFactory> SystemNetworkContextManager::GetSharedURLLoaderFactory()
{
    return shared_url_loader_factory_;
}

// static
SystemNetworkContextManager *SystemNetworkContextManager::CreateInstance()
{
    DCHECK(!g_system_network_context_manager);
    g_system_network_context_manager = new SystemNetworkContextManager();
    return g_system_network_context_manager;
}

// static
SystemNetworkContextManager *SystemNetworkContextManager::GetInstance()
{
    return g_system_network_context_manager;
}

// static
void SystemNetworkContextManager::DeleteInstance()
{
    DCHECK(g_system_network_context_manager);
    delete g_system_network_context_manager;
}

SystemNetworkContextManager::SystemNetworkContextManager()
{
    shared_url_loader_factory_ = new URLLoaderFactoryForSystem(this);
}

SystemNetworkContextManager::~SystemNetworkContextManager()
{
    shared_url_loader_factory_->Shutdown();
}

void SystemNetworkContextManager::OnNetworkServiceCreated(network::mojom::NetworkService *network_service)
{
    bool is_quic_force_enabled = base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kEnableQuic);
    // Disable QUIC globally
    if (!is_quic_force_enabled)
        network_service->DisableQuic();

    network_service->SetUpHttpAuth(CreateHttpAuthStaticParams());
    network_service->ConfigureHttpAuthPrefs(CreateHttpAuthDynamicParams());

    // The system NetworkContext must be created first, since it sets
    // |primary_network_context| to true.
    network_service_network_context_.reset();
    network_service->CreateNetworkContext(
        network_service_network_context_.BindNewPipeAndPassReceiver(),
        CreateNetworkContextParams());

    // Configure the stub resolver. This must be done after the system
    // NetworkContext is created, but before anything has the chance to use it.
    //    bool stub_resolver_enabled;
    //    base::Optional<std::vector<network::mojom::DnsOverHttpsServerPtr>> dns_over_https_servers;
    //    GetStubResolverConfig(local_state_, &stub_resolver_enabled, &dns_over_https_servers);
    //    content::GetNetworkService()->ConfigureStubHostResolver(stub_resolver_enabled, std::move(dns_over_https_servers));
}

void SystemNetworkContextManager::AddSSLConfigToNetworkContextParams(network::mojom::NetworkContextParams *network_context_params)
{
    network_context_params->initial_ssl_config = network::mojom::SSLConfig::New();
    network_context_params->initial_ssl_config->rev_checking_enabled = true;
    network_context_params->initial_ssl_config->symantec_enforcement_disabled = true;
}

network::mojom::NetworkContextParamsPtr SystemNetworkContextManager::CreateDefaultNetworkContextParams()
{
    network::mojom::NetworkContextParamsPtr network_context_params = network::mojom::NetworkContextParams::New();
    content::UpdateCorsExemptHeader(network_context_params.get());

    network_context_params->enable_brotli = true;

    //    network_context_params->user_agent = GetUserAgent();

    // Disable referrers by default. Any consumer that enables referrers should
    // respect prefs::kEnableReferrers from the appropriate pref store.
    network_context_params->enable_referrers = false;

    //  const base::CommandLine& command_line =
    //      *base::CommandLine::ForCurrentProcess();

    //  // TODO(eroman): Figure out why this doesn't work in single-process mode,
    //  // or if it does work, now.
    //  // Should be possible now that a private isolate is used.
    //  // http://crbug.com/474654
    //  if (!command_line.HasSwitch(switches::kWinHttpProxyResolver)) {
    //    if (command_line.HasSwitch(switches::kSingleProcess)) {
    //      LOG(ERROR) << "Cannot use V8 Proxy resolver in single process mode.";
    //    } else {
    network_context_params->proxy_resolver_factory = ChromeMojoProxyResolverFactory::CreateWithSelfOwnedReceiver();
    //    }
    //  }

    //    network_context_params->pac_quick_check_enabled = local_state_->GetBoolean(prefs::kQuickCheckEnabled);

    // Use the SystemNetworkContextManager to populate and update SSL
    // configuration. The SystemNetworkContextManager is owned by the
    // BrowserProcess itself, so will only be destroyed on shutdown, at which
    // point, all NetworkContexts will be destroyed as well.
    AddSSLConfigToNetworkContextParams(network_context_params.get());

    // CT is only enabled on Desktop platforms for now.
    network_context_params->enforce_chrome_ct_policy = true;
    for (const auto &ct_log : certificate_transparency::GetKnownLogs()) {
        // TODO(rsleevi): https://crbug.com/702062 - Remove this duplication.
        network::mojom::CTLogInfoPtr log_info = network::mojom::CTLogInfo::New();
        log_info->public_key = std::string(ct_log.log_key, ct_log.log_key_length);
        log_info->name = ct_log.log_name;
        network_context_params->ct_logs.push_back(std::move(log_info));
    }

    return network_context_params;
}

network::mojom::NetworkContextParamsPtr SystemNetworkContextManager::CreateNetworkContextParams()
{
    // TODO(mmenke): Set up parameters here (in memory cookie store, etc).
    network::mojom::NetworkContextParamsPtr network_context_params = CreateDefaultNetworkContextParams();

    network_context_params->context_name = std::string("system");

    network_context_params->enable_referrers = false;

    network_context_params->http_cache_enabled = false;

    // These are needed for PAC scripts that use FTP URLs.
#if !BUILDFLAG(DISABLE_FTP_SUPPORT)
    network_context_params->enable_ftp_url_support = true;
#endif

    network_context_params->primary_network_context = false;

    proxy_config_monitor_.AddToNetworkContextParams(network_context_params.get());

    return network_context_params;
}
