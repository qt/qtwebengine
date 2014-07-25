/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
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
#include <QtCore/qcompilerdetection.h> // Needed for Q_DECL_OVERRIDE

class NetworkDelegateQt : public net::NetworkDelegate {
public:
    NetworkDelegateQt() {}
    virtual ~NetworkDelegateQt() {}

private:
    // net::NetworkDelegate implementation
    virtual int OnBeforeURLRequest(net::URLRequest* request, const net::CompletionCallback& callback, GURL* new_url) Q_DECL_OVERRIDE;
    virtual int OnBeforeSendHeaders(net::URLRequest* request, const net::CompletionCallback& callback, net::HttpRequestHeaders* headers) Q_DECL_OVERRIDE
    {
        return net::OK;
    }

    virtual void OnSendHeaders(net::URLRequest* request, const net::HttpRequestHeaders& headers) Q_DECL_OVERRIDE {}
    virtual int OnHeadersReceived(net::URLRequest* request, const net::CompletionCallback& callback,
        const net::HttpResponseHeaders* original_response_headers,
        scoped_refptr<net::HttpResponseHeaders>* override_response_headers) Q_DECL_OVERRIDE { return net::OK; }

    virtual void OnBeforeRedirect(net::URLRequest* request, const GURL& new_location) Q_DECL_OVERRIDE { }
    virtual void OnResponseStarted(net::URLRequest* request) Q_DECL_OVERRIDE { }
    virtual void OnRawBytesRead(const net::URLRequest& request, int bytes_read) Q_DECL_OVERRIDE { }
    virtual void OnCompleted(net::URLRequest* request, bool started) Q_DECL_OVERRIDE { }
    virtual void OnURLRequestDestroyed(net::URLRequest* request) Q_DECL_OVERRIDE;

    virtual void OnPACScriptError(int line_number, const base::string16& error) Q_DECL_OVERRIDE { }
    virtual AuthRequiredResponse OnAuthRequired(net::URLRequest* request, const net::AuthChallengeInfo& auth_info,
        const AuthCallback& callback, net::AuthCredentials* credentials) Q_DECL_OVERRIDE { return AUTH_REQUIRED_RESPONSE_NO_ACTION; }

    virtual bool OnCanGetCookies(const net::URLRequest& request, const net::CookieList& cookie_list) Q_DECL_OVERRIDE { return true; }
    virtual bool OnCanSetCookie(const net::URLRequest& request, const std::string& cookie_line, net::CookieOptions* options) Q_DECL_OVERRIDE { return true; }
    virtual bool OnCanAccessFile(const net::URLRequest& request, const base::FilePath& path) const Q_DECL_OVERRIDE { return true; }
    virtual bool OnCanThrottleRequest(const net::URLRequest& request) const Q_DECL_OVERRIDE { return false; }
    virtual int OnBeforeSocketStreamConnect(net::SocketStream* stream, const net::CompletionCallback& callback) Q_DECL_OVERRIDE { return net::OK; }
    virtual void OnRequestWaitStateChange(const net::URLRequest& request, RequestWaitState state) Q_DECL_OVERRIDE { }

    static void NotifyNavigationRequestedOnUIThread(qintptr requestId,
                                                    int navigationType,
                                                    const GURL *url,
                                                    GURL *new_url,
                                                    const net::CompletionCallback &callback,
                                                    int renderProcessId,
                                                    int renderViewId);

    static void CompleteURLRequestOnIOThread(qintptr requestId,
                                             int navigationRequestAction,
                                             QUrl url,
                                             GURL *new_url,
                                             const net::CompletionCallback &callback);

    static bool IsValidRequest(qintptr id) { return m_activeRequests.contains(id); }

    static QSet<qintptr> m_activeRequests;
};

#endif // NETWORK_DELEGATE_QT_H
