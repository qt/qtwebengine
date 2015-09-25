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

#ifndef NETWORK_DELEGATE_QT_H
#define NETWORK_DELEGATE_QT_H

#include "net/base/network_delegate.h"
#include "net/base/net_errors.h"

#include <QUrl>
#include <QSet>

namespace QtWebEngineCore {

class URLRequestContextGetterQt;

class NetworkDelegateQt : public net::NetworkDelegate {
    QSet<net::URLRequest *> m_activeRequests;
    URLRequestContextGetterQt *m_requestContextGetter;
public:
    NetworkDelegateQt(URLRequestContextGetterQt *requestContext);

    struct RequestParams {
        QUrl url;
        bool isMainFrameRequest;
        int navigationType;
        int renderProcessId;
        int renderFrameId;
    };

    void NotifyNavigationRequestedOnUIThread(net::URLRequest *request,
                                             RequestParams params,
                                             const net::CompletionCallback &callback);

    void CompleteURLRequestOnIOThread(net::URLRequest *request,
                                      int navigationRequestAction,
                                      const net::CompletionCallback &callback);

    // net::NetworkDelegate implementation
    virtual int OnBeforeURLRequest(net::URLRequest* request, const net::CompletionCallback& callback, GURL* newUrl) override;
    virtual void OnURLRequestDestroyed(net::URLRequest* request) override;
    virtual bool OnCanSetCookie(const net::URLRequest&, const std::string&, net::CookieOptions*) override;
    virtual void OnResolveProxy(const GURL&, int, const net::ProxyService&, net::ProxyInfo*) override;
    virtual void OnProxyFallback(const net::ProxyServer&, int) override;
    virtual int OnBeforeSendHeaders(net::URLRequest*, const net::CompletionCallback&, net::HttpRequestHeaders*) override;
    virtual void OnBeforeSendProxyHeaders(net::URLRequest*, const net::ProxyInfo&, net::HttpRequestHeaders*) override;
    virtual void OnSendHeaders(net::URLRequest*, const net::HttpRequestHeaders&) override;
    virtual int OnHeadersReceived(net::URLRequest*, const net::CompletionCallback&, const net::HttpResponseHeaders*, scoped_refptr<net::HttpResponseHeaders>*, GURL*) override;
    virtual void OnBeforeRedirect(net::URLRequest*, const GURL&) override;
    virtual void OnResponseStarted(net::URLRequest*) override;
    virtual void OnRawBytesRead(const net::URLRequest&, int) override;
    virtual void OnCompleted(net::URLRequest*, bool) override;
    virtual void OnPACScriptError(int, const base::string16&) override;
    virtual net::NetworkDelegate::AuthRequiredResponse OnAuthRequired(net::URLRequest*, const net::AuthChallengeInfo&, const AuthCallback&, net::AuthCredentials*) override;
    virtual bool OnCanGetCookies(const net::URLRequest&, const net::CookieList&) override;
    virtual bool OnCanAccessFile(const net::URLRequest& request, const base::FilePath& path) const override;
    virtual bool OnCanEnablePrivacyMode(const GURL&, const GURL&) const override;
    virtual bool OnFirstPartyOnlyCookieExperimentEnabled() const override;
    virtual bool OnCancelURLRequestWithPolicyViolatingReferrerHeader(const net::URLRequest&, const GURL&, const GURL&) const override;
};

} // namespace QtWebEngineCore

#endif // NETWORK_DELEGATE_QT_H
