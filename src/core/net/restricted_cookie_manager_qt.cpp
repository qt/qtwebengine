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

// originally based on android_webview/browser/network_service/aw_proxying_restricted_cookie_manager.cc:
// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "restricted_cookie_manager_qt.h"

#include "api/qwebenginecookiestore.h"
#include "api/qwebenginecookiestore_p.h"
#include "profile_adapter.h"
#include "profile_qt.h"
#include "type_conversion.h"

#include "base/memory/ptr_util.h"
#include "base/task/post_task.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace QtWebEngineCore {

class RestrictedCookieManagerListenerQt : public network::mojom::CookieChangeListener
{
public:
    RestrictedCookieManagerListenerQt(const GURL &url,
                                      const GURL &site_for_cookies,
                                      base::WeakPtr<RestrictedCookieManagerQt> restricted_cookie_manager,
                                      network::mojom::CookieChangeListenerPtr client_listener)
            : url_(url)
            , site_for_cookies_(site_for_cookies)
            , restricted_cookie_manager_(restricted_cookie_manager)
            , client_listener_(std::move(client_listener))
    {}

    void OnCookieChange(const net::CanonicalCookie &cookie, network::mojom::CookieChangeCause cause) override
    {
        if (restricted_cookie_manager_ && restricted_cookie_manager_->allowCookies(url_, site_for_cookies_))
            client_listener_->OnCookieChange(cookie, cause);
    }

private:
    const GURL url_;
    const GURL site_for_cookies_;
    base::WeakPtr<RestrictedCookieManagerQt> restricted_cookie_manager_;
    network::mojom::CookieChangeListenerPtr client_listener_;
};

RestrictedCookieManagerQt::RestrictedCookieManagerQt(base::WeakPtr<ProfileIODataQt> profileIoData,
                                                     network::mojom::RestrictedCookieManagerRole role,
                                                     net::CookieStore *cookie_store,
                                                     network::CookieSettings *cookie_settings,
                                                     const url::Origin &origin,
                                                     bool is_service_worker,
                                                     int32_t process_id,
                                                     int32_t frame_id)
        : network::RestrictedCookieManager(role, cookie_store, cookie_settings, origin,
                                           nullptr, is_service_worker, process_id, frame_id)
        , m_profileIoData(profileIoData)
        , weak_factory_(this)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
}

RestrictedCookieManagerQt::~RestrictedCookieManagerQt()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
}

void RestrictedCookieManagerQt::GetAllForUrl(const GURL &url,
                                             const GURL &site_for_cookies,
                                             network::mojom::CookieManagerGetOptionsPtr options,
                                             GetAllForUrlCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    if (allowCookies(url, site_for_cookies)) {
        network::RestrictedCookieManager::GetAllForUrl(url, site_for_cookies, std::move(options), std::move(callback));
    } else {
        std::move(callback).Run(std::vector<net::CanonicalCookie>());
    }
}

void RestrictedCookieManagerQt::SetCanonicalCookie(const net::CanonicalCookie &cookie,
                                                   const GURL &url,
                                                   const GURL &site_for_cookies,
                                                   SetCanonicalCookieCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    if (allowCookies(url, site_for_cookies)) {
        network::RestrictedCookieManager::SetCanonicalCookie(cookie, url, site_for_cookies, std::move(callback));
    } else {
        std::move(callback).Run(false);
    }
}

void RestrictedCookieManagerQt::AddChangeListener(const GURL &url,
                                                  const GURL &site_for_cookies,
                                                  network::mojom::CookieChangeListenerPtr listener,
                                                  AddChangeListenerCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    network::mojom::CookieChangeListenerPtr proxy_listener_ptr;
    auto proxy_listener =
          std::make_unique<RestrictedCookieManagerListenerQt>(
                url, site_for_cookies, weak_factory_.GetWeakPtr(),
                std::move(listener));

    mojo::MakeStrongBinding(std::move(proxy_listener),
                            mojo::MakeRequest(&proxy_listener_ptr));

    network::RestrictedCookieManager::AddChangeListener(
                url, site_for_cookies, std::move(proxy_listener_ptr),
                std::move(callback));
}

void RestrictedCookieManagerQt::GetCookiesString(const GURL &url,
                                                 const GURL &site_for_cookies,
                                                 GetCookiesStringCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    if (allowCookies(url, site_for_cookies)) {
        network::RestrictedCookieManager::GetCookiesString(url, site_for_cookies, std::move(callback));
    } else {
        std::move(callback).Run("");
    }
}

void RestrictedCookieManagerQt::CookiesEnabledFor(const GURL &url,
                                                  const GURL &site_for_cookies,
                                                  CookiesEnabledForCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    std::move(callback).Run(allowCookies(url, site_for_cookies));
}

bool RestrictedCookieManagerQt::allowCookies(const GURL &url, const GURL &site_for_cookies) const
{
    if (!m_profileIoData)
        return false;
    return m_profileIoData->canGetCookies(toQt(site_for_cookies), toQt(url));
}

} // namespace QtWebEngineCore
