// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PROXYING_RESTRICTED_COOKIE_MANAGER_QT_H
#define PROXYING_RESTRICTED_COOKIE_MANAGER_QT_H

#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "services/network/public/mojom/restricted_cookie_manager.mojom.h"
#include "url/gurl.h"

namespace QtWebEngineCore {

class ProfileIODataQt;

class ProxyingRestrictedCookieManagerQt : public network::mojom::RestrictedCookieManager
{
public:
    // Expects to be called on the UI thread.
    static void CreateAndBind(ProfileIODataQt *profileIoData,
                              mojo::PendingRemote<network::mojom::RestrictedCookieManager> underlying_rcm,
                              mojo::PendingReceiver<network::mojom::RestrictedCookieManager> receiver);

    ~ProxyingRestrictedCookieManagerQt() override;

    // network::mojom::RestrictedCookieManager interface:
    void GetAllForUrl(const GURL &url,
                      const net::SiteForCookies &site_for_cookies,
                      const url::Origin &top_frame_origin,
                      bool has_storage_access,
                      network::mojom::CookieManagerGetOptionsPtr options,
                      GetAllForUrlCallback callback) override;

    void SetCanonicalCookie(const net::CanonicalCookie& cookie,
                            const GURL &url,
                            const net::SiteForCookies &site_for_cookies,
                            const url::Origin &top_frame_origin,
                            bool has_storage_access,
                            net::CookieInclusionStatus status,
                            SetCanonicalCookieCallback callback) override;
    void AddChangeListener(const GURL &url,
                           const net::SiteForCookies &site_for_cookies,
                           const url::Origin &top_frame_origin,
                           bool has_storage_access,
                           mojo::PendingRemote<network::mojom::CookieChangeListener> listener,
                           AddChangeListenerCallback callback) override;
    void SetCookieFromString(const GURL &url,
                             const net::SiteForCookies &site_for_cookies,
                             const url::Origin &top_frame_origin,
                             bool has_storage_access,
                             const std::string &cookie,
                             SetCookieFromStringCallback callback) override;
    void GetCookiesString(const GURL &url,
                          const net::SiteForCookies &site_for_cookies,
                          const url::Origin &top_frame_origin,
                          bool has_storage_access,
                          GetCookiesStringCallback callback) override;
    void CookiesEnabledFor(const GURL &url,
                           const net::SiteForCookies &site_for_cookies,
                           const url::Origin &top_frame_origin,
                           bool has_storage_access,
                           CookiesEnabledForCallback callback) override;

    // Internal:
    bool allowCookies(const GURL &url, const net::SiteForCookies &site_for_cookies) const;

private:
    ProxyingRestrictedCookieManagerQt(base::WeakPtr<ProfileIODataQt> profileIoData,
                                      mojo::PendingRemote<network::mojom::RestrictedCookieManager> underlying_rcm);

    static void CreateAndBindOnIoThread(ProfileIODataQt *profileIoData,
                                        mojo::PendingRemote<network::mojom::RestrictedCookieManager> underlying_rcm,
                                        mojo::PendingReceiver<network::mojom::RestrictedCookieManager> receiver);

    base::WeakPtr<ProfileIODataQt> m_profileIoData;

    mojo::Remote<network::mojom::RestrictedCookieManager> underlying_restricted_cookie_manager_;

    base::WeakPtrFactory<ProxyingRestrictedCookieManagerQt> weak_factory_;
};

} // namespace QtWebEngineCore

#endif  // PROXYING_RESTRICTED_COOKIE_MANAGER_QT_H
