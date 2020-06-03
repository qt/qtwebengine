/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef PROXYING_RESTRICTED_COOKIE_MANAGER_QT_H
#define PROXYING_RESTRICTED_COOKIE_MANAGER_QT_H

#include "base/macros.h"
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
                              bool is_service_worker,
                              int process_id,
                              int frame_id,
                              mojo::PendingReceiver<network::mojom::RestrictedCookieManager> receiver);

    ~ProxyingRestrictedCookieManagerQt() override;

    // network::mojom::RestrictedCookieManager interface:
    void GetAllForUrl(const GURL &url,
                      const net::SiteForCookies &site_for_cookies,
                      const url::Origin &top_frame_origin,
                      network::mojom::CookieManagerGetOptionsPtr options,
                      GetAllForUrlCallback callback) override;
    void SetCanonicalCookie(const net::CanonicalCookie& cookie,
                            const GURL &url,
                            const net::SiteForCookies &site_for_cookies,
                            const url::Origin &top_frame_origin,
                            SetCanonicalCookieCallback callback) override;
    void AddChangeListener(const GURL &url,
                           const net::SiteForCookies &site_for_cookies,
                           const url::Origin &top_frame_origin,
                           mojo::PendingRemote<network::mojom::CookieChangeListener> listener,
                           AddChangeListenerCallback callback) override;
    void SetCookieFromString(const GURL &url,
                             const net::SiteForCookies &site_for_cookies,
                             const url::Origin &top_frame_origin,
                             const std::string &cookie,
                             SetCookieFromStringCallback callback) override;
    void GetCookiesString(const GURL &url,
                          const net::SiteForCookies &site_for_cookies,
                          const url::Origin &top_frame_origin,
                          GetCookiesStringCallback callback) override;
    void CookiesEnabledFor(const GURL &url,
                           const net::SiteForCookies &site_for_cookies,
                           const url::Origin &top_frame_origin,
                           CookiesEnabledForCallback callback) override;

    // Internal:
    bool allowCookies(const GURL &url, const net::SiteForCookies &site_for_cookies) const;

private:
    ProxyingRestrictedCookieManagerQt(base::WeakPtr<ProfileIODataQt> profileIoData,
                                      mojo::PendingRemote<network::mojom::RestrictedCookieManager> underlying_rcm,
                                      bool is_service_worker,
                                      int32_t process_id,
                                      int32_t frame_id);

    static void CreateAndBindOnIoThread(ProfileIODataQt *profileIoData,
                                        mojo::PendingRemote<network::mojom::RestrictedCookieManager> underlying_rcm,
                                        bool is_service_worker,
                                        int process_id,
                                        int frame_id,
                                        mojo::PendingReceiver<network::mojom::RestrictedCookieManager> receiver);

    base::WeakPtr<ProfileIODataQt> m_profileIoData;

    mojo::Remote<network::mojom::RestrictedCookieManager> underlying_restricted_cookie_manager_;
    bool is_service_worker_;
    int process_id_;
    int frame_id_;

    base::WeakPtrFactory<ProxyingRestrictedCookieManagerQt> weak_factory_;

    DISALLOW_COPY_AND_ASSIGN(ProxyingRestrictedCookieManagerQt);
};

} // namespace QtWebEngineCore

#endif  // PROXYING_RESTRICTED_COOKIE_MANAGER_QT_H
