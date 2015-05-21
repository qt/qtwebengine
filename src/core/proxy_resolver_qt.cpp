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

#include "proxy_resolver_qt.h"

#include "proxy_config_service_qt.h"
#include "type_conversion.h"

#include "net/base/load_states.h"
#include "net/base/net_errors.h"
#include "net/proxy/proxy_info.h"

#include <QNetworkProxyFactory>
#include <QNetworkProxyQuery>

ProxyResolverQt::ProxyResolverQt()
    : net::ProxyResolver(/*expects_pac_bytes = */ false)
{
}

int ProxyResolverQt::GetProxyForURL(const GURL &url, net::ProxyInfo *results, const net::CompletionCallback &, net::ProxyResolver::RequestHandle *, const net::BoundNetLog &)
{
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(QtWebEngineCore::toQt(url)));
    if (proxies.empty())
        return net::ERR_FAILED;
    if (proxies.size() == 1) {
        const QNetworkProxy &qtProxy = proxies.first();
        if (qtProxy.type() == QNetworkProxy::NoProxy)
            results->UseDirect();
        else
            results->UseProxyServer(ProxyConfigServiceQt::fromQNetworkProxy(qtProxy));
        return net::OK;
    }
    net::ProxyList proxyList;
    Q_FOREACH (const QNetworkProxy &qtProxy, proxies)
        proxyList.AddProxyServer(ProxyConfigServiceQt::fromQNetworkProxy(qtProxy));
    results->UseProxyList(proxyList);
    return net::OK;
}

void ProxyResolverQt::CancelRequest(net::ProxyResolver::RequestHandle)
{
    // This is a synchronous ProxyResolver; no possibility for async requests.
    NOTREACHED();
}

net::LoadState ProxyResolverQt::GetLoadState(net::ProxyResolver::RequestHandle) const
{
    NOTREACHED();
    return net::LOAD_STATE_IDLE;
}

void ProxyResolverQt::CancelSetPacScript()
{
    NOTREACHED();
}

int ProxyResolverQt::SetPacScript(const scoped_refptr<net::ProxyResolverScriptData> &, const net::CompletionCallback &) {
    return net::OK;
}
