// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


//================ Based on ChromeProxyConfigService =======================
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "proxy_config_service_qt.h"

#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "net/base/proxy_server.h"

#include <QNetworkProxy>

net::ProxyServer ProxyConfigServiceQt::fromQNetworkProxy(const QNetworkProxy &qtProxy)
{
    std::string host = qtProxy.hostName().toStdString();
    uint16_t port = qtProxy.port();
    switch (qtProxy.type()) {
    case QNetworkProxy::Socks5Proxy:
        return net::ProxyServer::FromSchemeHostAndPort(net::ProxyServer::SCHEME_SOCKS5, host, port);
    case QNetworkProxy::HttpProxy:
    case QNetworkProxy::HttpCachingProxy:
    case QNetworkProxy::FtpCachingProxy:
        return net::ProxyServer::FromSchemeHostAndPort(net::ProxyServer::SCHEME_HTTP, host, port);
    case QNetworkProxy::NoProxy:
    case QNetworkProxy::DefaultProxy:
        return net::ProxyServer(net::ProxyServer::SCHEME_DIRECT, net::HostPortPair());
    default:
        return net::ProxyServer(net::ProxyServer::SCHEME_INVALID, net::HostPortPair());
    }
}

ProxyConfigServiceQt::ProxyConfigServiceQt(PrefService *prefService,
                                           const scoped_refptr<base::SequencedTaskRunner> &taskRunner)
    : m_baseService(net::ProxyConfigService::CreateSystemProxyConfigService(taskRunner))
    , m_usesSystemConfiguration(false)
    , m_registeredObserver(false)
    , m_prefState(prefService
                  ? PrefProxyConfigTrackerImpl::ReadPrefConfig(prefService, &m_prefConfig)
                  : ProxyPrefs::CONFIG_UNSET)
{
    DETACH_FROM_SEQUENCE(m_sequenceChecker);
}

ProxyConfigServiceQt::~ProxyConfigServiceQt()
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(m_sequenceChecker);
    if (m_registeredObserver && m_baseService.get())
        m_baseService->RemoveObserver(this);
}

void ProxyConfigServiceQt::AddObserver(net::ProxyConfigService::Observer *observer)
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(m_sequenceChecker);
    m_observers.AddObserver(observer);
}

void ProxyConfigServiceQt::RemoveObserver(net::ProxyConfigService::Observer *observer)
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(m_sequenceChecker);
    m_observers.RemoveObserver(observer);
}

net::ProxyConfigService::ConfigAvailability ProxyConfigServiceQt::GetLatestProxyConfig(net::ProxyConfigWithAnnotation *config)
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(m_sequenceChecker);
    m_usesSystemConfiguration = QNetworkProxyFactory::usesSystemConfiguration();
    if (m_usesSystemConfiguration) {
        // Use Chromium's base service to retrieve system settings
        net::ProxyConfigWithAnnotation systemConfig;
        ConfigAvailability systemAvailability = net::ProxyConfigService::CONFIG_UNSET;
        if (m_baseService.get())
            systemAvailability = m_baseService->GetLatestProxyConfig(&systemConfig);
        ProxyPrefs::ConfigState configState;
        systemAvailability = PrefProxyConfigTrackerImpl::GetEffectiveProxyConfig(
            m_prefState, m_prefConfig, systemAvailability, systemConfig,
            false, &configState, config);
        RegisterObserver();
        return systemAvailability;
    }

    // Use QNetworkProxy::applicationProxy settings
    const QNetworkProxy &qtProxy = QNetworkProxy::applicationProxy();
    if (qtProxy == m_qtApplicationProxy && !m_qtProxyConfig.proxy_rules().empty()) {
        // no changes
        *config = net::ProxyConfigWithAnnotation(m_qtProxyConfig, config->traffic_annotation());
        return CONFIG_VALID;
    }

    m_qtApplicationProxy = qtProxy;
    m_qtProxyConfig = net::ProxyConfig();

    net::ProxyConfig::ProxyRules qtRules;
    net::ProxyServer server = fromQNetworkProxy(qtProxy);
    switch (qtProxy.type()) {
    case QNetworkProxy::HttpProxy:
    case QNetworkProxy::Socks5Proxy:
        qtRules.type = net::ProxyConfig::ProxyRules::Type::PROXY_LIST;
        qtRules.single_proxies.SetSingleProxyServer(server);
        break;
    case QNetworkProxy::HttpCachingProxy:
        qtRules.type = net::ProxyConfig::ProxyRules::Type::PROXY_LIST_PER_SCHEME;
        qtRules.proxies_for_http.SetSingleProxyServer(server);
        break;
    case QNetworkProxy::FtpCachingProxy:
        qtRules.type = net::ProxyConfig::ProxyRules::Type::PROXY_LIST_PER_SCHEME;
        qtRules.proxies_for_ftp.SetSingleProxyServer(server);
        break;
    default:
        qtRules.type = net::ProxyConfig::ProxyRules::Type::EMPTY;
    }

    qtRules.bypass_rules.PrependRuleToBypassSimpleHostnames(); // don't use proxy for connections to localhost
    m_qtProxyConfig.proxy_rules() = qtRules;
    *config = net::ProxyConfigWithAnnotation(m_qtProxyConfig, config->traffic_annotation());
    return CONFIG_VALID;
}

void ProxyConfigServiceQt::OnLazyPoll()
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(m_sequenceChecker);

    // We need to update if
    // - setUseSystemConfiguration() was called in between
    // - user changed application proxy
    if (m_usesSystemConfiguration != QNetworkProxyFactory::usesSystemConfiguration()
        || (!m_usesSystemConfiguration && m_qtApplicationProxy != QNetworkProxy::applicationProxy())) {
        Update();
    } else if (m_usesSystemConfiguration) {
        if (m_baseService.get())
            m_baseService->OnLazyPoll();
    }
}

// Called when the base service changed
void ProxyConfigServiceQt::OnProxyConfigChanged(const net::ProxyConfigWithAnnotation &config, ConfigAvailability availability)
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(m_sequenceChecker);
    Q_UNUSED(config);

    if (!m_usesSystemConfiguration)
        return;

    Update();
}

// Update our observers
void ProxyConfigServiceQt::Update()
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(m_sequenceChecker);
    net::ProxyConfigWithAnnotation actual_config;
    ConfigAvailability availability = GetLatestProxyConfig(&actual_config);
    if (availability == CONFIG_PENDING)
        return;
    for (net::ProxyConfigService::Observer &observer: m_observers)
        observer.OnProxyConfigChanged(actual_config, availability);
}

// Register ourselves as observer of the base service.
// This has to be done on the IO thread, and therefore cannot be done
// in the constructor.
void ProxyConfigServiceQt::RegisterObserver()
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(m_sequenceChecker);
    if (!m_registeredObserver && m_baseService.get()) {
        m_baseService->AddObserver(this);
        m_registeredObserver = true;
    }
}
