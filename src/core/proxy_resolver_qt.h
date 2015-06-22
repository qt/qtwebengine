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

#ifndef PROXY_RESOLVER_QT_H
#define PROXY_RESOLVER_QT_H

#include "net/proxy/proxy_resolver.h"
#include "net/proxy/proxy_resolver_factory.h"
#include <qglobal.h>


class ProxyResolverQt : public net::ProxyResolver {
public:
    static bool useProxyResolverQt() { return qEnvironmentVariableIsSet("QTWEBENGINE_USE_QT_PROXYRESOLVER"); }

    ProxyResolverQt();

    int GetProxyForURL(const GURL &url, net::ProxyInfo *results, const net::CompletionCallback &
                       , RequestHandle*, const net::BoundNetLog&) override;

    void CancelRequest(RequestHandle) override;

    net::LoadState GetLoadState(RequestHandle) const override;

    void CancelSetPacScript() override;

    int SetPacScript(const scoped_refptr<net::ProxyResolverScriptData>& /*script_data*/, const net::CompletionCallback& /*callback*/) override;
};

class ProxyResolverFactoryQt : public net::LegacyProxyResolverFactory {
public:
    ProxyResolverFactoryQt(bool expects_pac_bytes) : net::LegacyProxyResolverFactory(expects_pac_bytes)
    {
    }
    scoped_ptr<net::ProxyResolver> CreateProxyResolver() override
    {
        return scoped_ptr<net::ProxyResolver>(new ProxyResolverQt());
    }
};

#endif // PROXY_RESOLVER_QT_H
