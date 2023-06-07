// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SSL_HOST_STATE_DELEGATE_QT_H
#define SSL_HOST_STATE_DELEGATE_QT_H

#include "content/public/browser/ssl_host_state_delegate.h"

#include <map>
#include <string>

namespace QtWebEngineCore {

class CertPolicy
{
public:
    CertPolicy();
    ~CertPolicy();
    bool Check(const net::X509Certificate &cert, int error) const;
    void Allow(const net::X509Certificate &cert, int error);
    bool HasAllowException() const { return m_allowed.size() > 0; }

private:
    std::map<net::SHA256HashValue, int> m_allowed;
};

class SSLHostStateDelegateQt : public content::SSLHostStateDelegate
{

public:
    SSLHostStateDelegateQt();
    ~SSLHostStateDelegateQt();

    // content::SSLHostStateDelegate implementation:
    void AllowCert(const std::string &, const net::X509Certificate &cert, int error, content::StoragePartition *storage_partition) override;
    void Clear(base::RepeatingCallback<bool(const std::string&)> host_filter) override;
    CertJudgment QueryPolicy(const std::string &host, const net::X509Certificate &cert, int error, content::StoragePartition *web_contents) override;
    void HostRanInsecureContent(const std::string &host, int child_id, InsecureContentType content_type) override;
    bool DidHostRunInsecureContent(const std::string &host, int child_id, InsecureContentType content_type) override;
    void AllowHttpForHost(const std::string &host, content::StoragePartition *web_contents) override;
    bool IsHttpAllowedForHost(const std::string &host, content::StoragePartition *web_contents) override;
    void RevokeUserAllowExceptions(const std::string &host) override;
    bool HasAllowException(const std::string &host, content::StoragePartition *web_contents) override;

private:
    std::map<std::string, CertPolicy> m_certPolicyforHost;
};

} // namespace QtWebEngineCore

#endif // SSL_HOST_STATE_DELEGATE_QT_H
