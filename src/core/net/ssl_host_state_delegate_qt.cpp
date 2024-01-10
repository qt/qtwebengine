// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "ssl_host_state_delegate_qt.h"

#include "base/functional/callback.h"

namespace QtWebEngineCore {

// Mirrors implementation in aw_ssl_host_state_delegate.cc

CertPolicy::CertPolicy()
{
}

CertPolicy::~CertPolicy()
{
}

// For an allowance, we consider a given |cert| to be a match to a saved
// allowed cert if the |error| is an exact match to or subset of the errors
// in the saved CertStatus.
bool CertPolicy::Check(const net::X509Certificate &cert, int error) const
{
    net::SHA256HashValue fingerprint = cert.CalculateChainFingerprint256();
    auto allowed_iter = m_allowed.find(fingerprint);
    if ((allowed_iter != m_allowed.end()) && (allowed_iter->second & error) && ((allowed_iter->second & error) == error))
        return true;
    return false;
}

void CertPolicy::Allow(const net::X509Certificate &cert, int error)
{
    net::SHA256HashValue fingerprint = cert.CalculateChainFingerprint256();
    m_allowed[fingerprint] |= error;
}

SSLHostStateDelegateQt::SSLHostStateDelegateQt() {}

SSLHostStateDelegateQt::~SSLHostStateDelegateQt() {}

void SSLHostStateDelegateQt::AllowCert(const std::string &host, const net::X509Certificate &cert, int error, content::StoragePartition *)
{
    m_certPolicyforHost[host].Allow(cert, error);
}

// Clear all allow preferences.
void SSLHostStateDelegateQt::Clear(base::RepeatingCallback<bool(const std::string&)> host_filter)
{
    if (host_filter.is_null()) {
        m_certPolicyforHost.clear();
        return;
    }

    for (auto it = m_certPolicyforHost.begin(); it != m_certPolicyforHost.end();) {
        auto next_it = std::next(it);

        if (host_filter.Run(it->first))
            m_certPolicyforHost.erase(it);

        it = next_it;
    }
}

// Queries whether |cert| is allowed for |host| and |error|. Returns true in
// |expired_previous_decision| if a previous user decision expired immediately
// prior to this query, otherwise false.
content::SSLHostStateDelegate::CertJudgment SSLHostStateDelegateQt::QueryPolicy(const std::string &host,
                                                                                const net::X509Certificate &cert,
                                                                                int error, content::StoragePartition *)
{
    return m_certPolicyforHost[host].Check(cert, error) ? SSLHostStateDelegate::ALLOWED : SSLHostStateDelegate::DENIED;
}

// Records that a host has run insecure content.
void SSLHostStateDelegateQt::HostRanInsecureContent(const std::string &host, int pid, InsecureContentType content_type)
{
}

// Returns whether the specified host ran insecure content.
bool SSLHostStateDelegateQt::DidHostRunInsecureContent(const std::string &host, int pid, InsecureContentType content_type)
{
    return false;
}

void SSLHostStateDelegateQt::AllowHttpForHost(const std::string &host, content::StoragePartition *web_contents)
{
    // Intentional no-op see aw_ssl_host_state_delegate
}

bool SSLHostStateDelegateQt::IsHttpAllowedForHost(const std::string &host, content::StoragePartition *web_contents)
{
    return false;
}

// Revokes all SSL certificate error allow exceptions made by the user for
// |host|.
void SSLHostStateDelegateQt::RevokeUserAllowExceptions(const std::string &host)
{
    m_certPolicyforHost.erase(host);
}

// Returns whether the user has allowed a certificate error exception for
// |host|. This does not mean that *all* certificate errors are allowed, just
// that there exists an exception. To see if a particular certificate and
// error combination exception is allowed, use QueryPolicy().
bool SSLHostStateDelegateQt::HasAllowException(const std::string &host, content::StoragePartition *)
{
    auto policy_iterator = m_certPolicyforHost.find(host);
    return policy_iterator != m_certPolicyforHost.end() &&
           policy_iterator->second.HasAllowException();
}


} // namespace QtWebEngineCore
