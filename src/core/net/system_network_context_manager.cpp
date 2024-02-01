// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/net/system_network_context_manager.cc:
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/system_network_context_manager.h"

#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/strings/string_split.h"
#include "chrome/browser/net/chrome_mojo_proxy_resolver_factory.h"
#include "chrome/common/chrome_switches.h"
#include "components/certificate_transparency/ct_known_logs.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "content/public/browser/network_service_instance.h"
#include "content/public/common/content_switches.h"
#include "crypto/sha2.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "net/base/port_util.h"
#include "net/net_buildflags.h"
#include "services/cert_verifier/public/mojom/cert_verifier_service_factory.mojom.h"
#include "services/network/network_service.h"
#include "services/network/public/cpp/cross_thread_pending_shared_url_loader_factory.h"
#include "services/network/public/cpp/features.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/mojom/cert_verifier_service.mojom.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "services/proxy_resolver/public/mojom/proxy_resolver.mojom.h"
#include "api/qwebengineglobalsettings.h"
#include "api/qwebengineglobalsettings_p.h"

#if BUILDFLAG(IS_WIN)
#include "chrome/browser/net/chrome_mojo_proxy_resolver_win.h"
#include "components/os_crypt/os_crypt.h"
#include "content/public/common/network_service_util.h"
#endif

namespace {

network::mojom::HttpAuthStaticParamsPtr CreateHttpAuthStaticParams()
{
    network::mojom::HttpAuthStaticParamsPtr auth_static_params =
        network::mojom::HttpAuthStaticParams::New();

    return auth_static_params;
}

network::mojom::HttpAuthDynamicParamsPtr CreateHttpAuthDynamicParams()
{
    network::mojom::HttpAuthDynamicParamsPtr auth_dynamic_params = network::mojom::HttpAuthDynamicParams::New();

    auth_dynamic_params->allowed_schemes = { "basic", "digest", "ntlm", "negotiate" };

    auto *command_line = base::CommandLine::ForCurrentProcess();
    auth_dynamic_params->server_allowlist = command_line->GetSwitchValueASCII(switches::kAuthServerAllowlist);
//    auth_dynamic_params->delegate_allowlist = command_line->GetSwitchValueASCII(switches::kAuthNegotiateDelegateWhitelist);
//    auth_dynamic_params->enable_negotiate_port = command_line->HasSwitch(switches::kEnableAuthNegotiatePort);

    return auth_dynamic_params;
}

} // namespace

namespace QtWebEngineCore {

// The global instance of the SystemNetworkContextmanager.
SystemNetworkContextManager *g_system_network_context_manager = nullptr;

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
                    std::move(receiver), request_id, options, url_request,
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

#if BUILDFLAG(IS_WIN)
    if (content::IsOutOfProcessNetworkService())
        network_service->SetEncryptionKey(OSCrypt::GetRawEncryptionKey());
#endif

    // Configure the Certificate Transparency logs.
    std::vector<std::pair<std::string, base::Time>> disqualified_logs =
        certificate_transparency::GetDisqualifiedLogs();
    std::vector<network::mojom::CTLogInfoPtr> log_list_mojo;
    for (const auto &ct_log : certificate_transparency::GetKnownLogs()) {
        network::mojom::CTLogInfoPtr log_info = network::mojom::CTLogInfo::New();
        log_info->public_key = std::string(ct_log.log_key, ct_log.log_key_length);
        log_info->id = crypto::SHA256HashString(log_info->public_key);
        log_info->name = ct_log.log_name;
        log_info->current_operator = ct_log.current_operator;

        auto it = std::lower_bound(
            std::begin(disqualified_logs), std::end(disqualified_logs), log_info->id,
            [](const auto& disqualified_log, const std::string& log_id) {
                return disqualified_log.first < log_id;
            });
        if (it != std::end(disqualified_logs) && it->first == log_info->id)
            log_info->disqualified_at = it->second;

        for (size_t i = 0; i < ct_log.previous_operators_length; i++) {
            const auto& op = ct_log.previous_operators[i];
            network::mojom::PreviousOperatorEntryPtr previous_operator =
                network::mojom::PreviousOperatorEntry::New();
            previous_operator->name = op.name;
            previous_operator->end_time = op.end_time;
            log_info->previous_operators.push_back(std::move(previous_operator));
        }

        log_list_mojo.push_back(std::move(log_info));
    }
    network_service->UpdateCtLogList(
            std::move(log_list_mojo),
            certificate_transparency::GetLogListTimestamp(),
            base::DoNothing());

    // The system NetworkContext is created first
    network_service_network_context_.reset();
    network_service->CreateNetworkContext(
        network_service_network_context_.BindNewPipeAndPassReceiver(),
        CreateNetworkContextParams());

    // Handle --explicitly-allowed-ports
    if (base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kExplicitlyAllowedPorts)) {
        std::vector<uint16_t> explicitly_allowed_network_ports;
        std::string switch_value =
            base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(switches::kExplicitlyAllowedPorts);
        const auto split = base::SplitStringPiece(switch_value, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
        for (const auto &piece : split) {
            int port;
            if (!base::StringToInt(piece, &port))
                continue;
            if (!net::IsPortValid(port))
                continue;
            explicitly_allowed_network_ports.push_back(static_cast<uint16_t>(port));
        }

        network_service->SetExplicitlyAllowedPorts(explicitly_allowed_network_ports);
    }

    // The network service is a singleton that can be reinstantiated for different reasons,
    // e.g., when the network service crashes. Therefore, we configure the stub host
    // resolver of the network service here, each time it is instantiated, with our global
    // DNS-Over-HTTPS settings. This ensures that the global settings don't get lost
    // on reinstantiation and are in effect upon initial instantiation.
    QWebEngineGlobalSettingsPrivate::instance()->configureStubHostResolver();
}

void SystemNetworkContextManager::AddSSLConfigToNetworkContextParams(network::mojom::NetworkContextParams *network_context_params)
{
    network_context_params->initial_ssl_config = network::mojom::SSLConfig::New();
    network_context_params->initial_ssl_config->symantec_enforcement_disabled = true;
}

void SystemNetworkContextManager::ConfigureDefaultNetworkContextParams(network::mojom::NetworkContextParams *network_context_params,
                                                                       cert_verifier::mojom::CertVerifierCreationParams *cert_verifier_creation_params)
{
    network_context_params->enable_brotli = true;

    // Disable referrers by default. Any consumer that enables referrers should
    // respect prefs::kEnableReferrers from the appropriate pref store.
    network_context_params->enable_referrers = false;

    const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();

    if (!command_line.HasSwitch(switches::kWinHttpProxyResolver)) {
        if (command_line.HasSwitch(switches::kSingleProcess)) {
            LOG(ERROR) << "Cannot use V8 Proxy resolver in single process mode.";
        } else {
            network_context_params->proxy_resolver_factory =
                    ChromeMojoProxyResolverFactory::CreateWithSelfOwnedReceiver();
        }
    }
#if BUILDFLAG(IS_WIN)
    if (command_line.HasSwitch(switches::kUseSystemProxyResolver)) {
        network_context_params->windows_system_proxy_resolver =
                ChromeMojoProxyResolverWin::CreateWithSelfOwnedReceiver();
    }
#endif
    // Use the SystemNetworkContextManager to populate and update SSL
    // configuration. The SystemNetworkContextManager is owned by the
    // BrowserProcess itself, so will only be destroyed on shutdown, at which
    // point, all NetworkContexts will be destroyed as well.
    AddSSLConfigToNetworkContextParams(network_context_params);
}

network::mojom::NetworkContextParamsPtr SystemNetworkContextManager::CreateNetworkContextParams()
{
    // TODO(mmenke): Set up parameters here (in memory cookie store, etc).
    network::mojom::NetworkContextParamsPtr network_context_params = network::mojom::NetworkContextParams::New();
    cert_verifier::mojom::CertVerifierCreationParamsPtr
            cert_verifier_creation_params = cert_verifier::mojom::CertVerifierCreationParams::New();
    ConfigureDefaultNetworkContextParams(network_context_params.get(), cert_verifier_creation_params.get());

    network_context_params->enable_referrers = false;

    network_context_params->http_cache_enabled = false;

    proxy_config_monitor_.AddToNetworkContextParams(network_context_params.get());

    network_context_params->cert_verifier_params =
         content::GetCertVerifierParams(std::move(cert_verifier_creation_params));
    return network_context_params;
}

} // namespace QtWebEngineCore
