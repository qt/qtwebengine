// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PROXY_CONFIG_SERVICE_QT_H
#define PROXY_CONFIG_SERVICE_QT_H

#include "base/observer_list.h"
#include "base/task/sequenced_task_runner.h"
#include "net/proxy_resolution/proxy_config.h"
#include "net/proxy_resolution/proxy_config_service.h"
#include "net/proxy_resolution/proxy_config_with_annotation.h"
#include "components/proxy_config/proxy_prefs.h"

#include <QNetworkProxy>

class PrefService;

class ProxyConfigServiceQt
    : public net::ProxyConfigService
    , public net::ProxyConfigService::Observer
{
public:
    static net::ProxyServer fromQNetworkProxy(const QNetworkProxy &);

    explicit ProxyConfigServiceQt(PrefService *prefService,
                                  const scoped_refptr<base::SequencedTaskRunner> &taskRunner);
    ~ProxyConfigServiceQt() override;

    // ProxyConfigService implementation:
    void AddObserver(net::ProxyConfigService::Observer *observer) override;
    void RemoveObserver(net::ProxyConfigService::Observer *observer) override;
    ConfigAvailability GetLatestProxyConfig(net::ProxyConfigWithAnnotation *config) override;
    void OnLazyPoll() override;

private:
    // ProxyConfigService::Observer implementation:
    void OnProxyConfigChanged(const net::ProxyConfigWithAnnotation &config,
                              ConfigAvailability availability) override;

    // Retrieve new proxy settings and notify observers.
    void Update();

    // Makes sure that the observer registration with the base service is set up.
    void RegisterObserver();

    std::unique_ptr<net::ProxyConfigService> m_baseService;
    base::ObserverList<net::ProxyConfigService::Observer, true>::Unchecked m_observers;

    // Keep the last state around.
    bool m_usesSystemConfiguration;
    QNetworkProxy m_qtApplicationProxy;
    net::ProxyConfig m_qtProxyConfig;

    // Indicates whether the base service registration is done.
    bool m_registeredObserver;

    // Configuration as defined by prefs.
    net::ProxyConfigWithAnnotation m_prefConfig;
    ProxyPrefs::ConfigState m_prefState;

    SEQUENCE_CHECKER(m_sequenceChecker);
};

#endif // PROXY_CONFIG_SERVICE_QT_H
