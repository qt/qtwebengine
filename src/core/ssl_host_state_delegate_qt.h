/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef SSL_HOST_STATE_DELEGATE_QT_H
#define SSL_HOST_STATE_DELEGATE_QT_H

#include "content/public/browser/ssl_host_state_delegate.h"
#include "browser_context_adapter.h"

namespace QtWebEngineCore {

class CertPolicy {
public:
    CertPolicy();
    ~CertPolicy();
    bool Check(const net::X509Certificate& cert, net::CertStatus error) const;
    void Allow(const net::X509Certificate& cert, net::CertStatus error);
    bool HasAllowException() const { return m_allowed.size() > 0; }

private:
    std::map<net::SHA256HashValue, net::CertStatus, net::SHA256HashValueLessThan> m_allowed;
};

class SSLHostStateDelegateQt : public content::SSLHostStateDelegate {

public:
    SSLHostStateDelegateQt();
    ~SSLHostStateDelegateQt();

    // content::SSLHostStateDelegate implementation:
    virtual void AllowCert(const std::string &, const net::X509Certificate &cert, net::CertStatus error) override;
    virtual void Clear(const base::Callback<bool(const std::string&)>& host_filter) override;
    virtual CertJudgment QueryPolicy(const std::string &host, const net::X509Certificate &cert,
                                     net::CertStatus error,bool *expired_previous_decision) override;
    virtual void HostRanInsecureContent(const std::string& host, int child_id, InsecureContentType content_type) override;
    virtual bool DidHostRunInsecureContent(const std::string& host, int child_id, InsecureContentType content_type) const override;
    virtual void RevokeUserAllowExceptions(const std::string &host) override;
    virtual bool HasAllowException(const std::string &host) const override;

private:
    std::map<std::string, CertPolicy> m_certPolicyforHost;
};

} // namespace QtWebEngineCore

#endif // SSL_HOST_STATE_DELEGATE_QT_H
