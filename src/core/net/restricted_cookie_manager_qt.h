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

#ifndef RESTRICTED_COOKIE_MANAGER_QT_H
#define RESTRICTED_COOKIE_MANAGER_QT_H

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "services/network/restricted_cookie_manager.h"
#include "url/gurl.h"

namespace QtWebEngineCore {

class ProfileIODataQt;

class RestrictedCookieManagerQt : public network::RestrictedCookieManager
{
public:
    RestrictedCookieManagerQt(base::WeakPtr<ProfileIODataQt> profileIoData,
                              network::mojom::RestrictedCookieManagerRole role,
                              net::CookieStore *cookie_store,
                              network::CookieSettings *cookie_settings,
                              const url::Origin &origin,
                              bool is_service_worker,
                              int32_t process_id,
                              int32_t frame_id);
    ~RestrictedCookieManagerQt() override;

    // network::mojom::RestrictedCookieManager interface:
    void GetAllForUrl(const GURL &url,
                      const GURL &site_for_cookies,
                      network::mojom::CookieManagerGetOptionsPtr options,
                      GetAllForUrlCallback callback) override;
    void SetCanonicalCookie(const net::CanonicalCookie& cookie,
                            const GURL &url,
                            const GURL &site_for_cookies,
                            SetCanonicalCookieCallback callback) override;
    void AddChangeListener(const GURL &url,
                           const GURL &site_for_cookies,
                           network::mojom::CookieChangeListenerPtr listener,
                           AddChangeListenerCallback callback) override;

    void GetCookiesString(const GURL &url,
                          const GURL &site_for_cookies,
                          GetCookiesStringCallback callback) override;

    void CookiesEnabledFor(const GURL &url,
                           const GURL &site_for_cookies,
                           CookiesEnabledForCallback callback) override;

    // Internal:
    bool allowCookies(const GURL &url, const GURL &site_for_cookies) const;

private:
    base::WeakPtr<ProfileIODataQt> m_profileIoData;

    base::WeakPtrFactory<RestrictedCookieManagerQt> weak_factory_;

    DISALLOW_COPY_AND_ASSIGN(RestrictedCookieManagerQt);
};

} // namespace QtWebEngineCore

#endif // RESTRICTED_COOKIE_MANAGER_QT_H
