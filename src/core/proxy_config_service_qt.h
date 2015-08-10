/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PROXY_CONFIG_SERVICE_QT_H
#define PROXY_CONFIG_SERVICE_QT_H

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"

#include "net/proxy/proxy_config.h"
#include "net/proxy/proxy_config_service.h"

#include <QNetworkProxy>

class ProxyConfigServiceQt
        : public net::ProxyConfigService,
        public net::ProxyConfigService::Observer {
public:

    static net::ProxyServer fromQNetworkProxy(const QNetworkProxy &);

    explicit ProxyConfigServiceQt(net::ProxyConfigService *baseService);
    ~ProxyConfigServiceQt() override;

    // ProxyConfigService implementation:
    void AddObserver(net::ProxyConfigService::Observer *observer) override;
    void RemoveObserver(net::ProxyConfigService::Observer *observer) override;
    ConfigAvailability GetLatestProxyConfig(net::ProxyConfig *config) override;
    void OnLazyPoll() override;

private:
    // ProxyConfigService::Observer implementation:
    void OnProxyConfigChanged(const net::ProxyConfig& config,
                              ConfigAvailability availability) override;

    // Makes sure that the observer registration with the base service is set up.
    void RegisterObserver();

    scoped_ptr<net::ProxyConfigService> m_baseService;
    base::ObserverList<net::ProxyConfigService::Observer, true> m_observers;

    // Keep the last QNetworkProxy::applicationProxy state around.
    QNetworkProxy m_qtApplicationProxy;
    net::ProxyConfig m_qtProxyConfig;

    // Indicates whether the base service registration is done.
    bool m_registeredObserver;

    DISALLOW_COPY_AND_ASSIGN(ProxyConfigServiceQt);
};

#endif // PROXY_CONFIG_SERVICE_QT_H
