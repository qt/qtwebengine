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

#include "proxying_restricted_cookie_manager_qt.h"

#include "api/qwebenginecookiestore.h"
#include "api/qwebenginecookiestore_p.h"
#include "profile_adapter.h"
#include "profile_qt.h"
#include "type_conversion.h"

#include "base/memory/ptr_util.h"
#include "base/task/post_task.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"

namespace QtWebEngineCore {

// static
void ProxyingRestrictedCookieManagerQt::CreateAndBind(ProfileIODataQt *profileIoData,
                                                      mojo::PendingRemote<network::mojom::RestrictedCookieManager> underlying_rcm,
                                                      bool is_service_worker,
                                                      int process_id,
                                                      int frame_id,
                                                      mojo::PendingReceiver<network::mojom::RestrictedCookieManager> receiver)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    base::PostTask(FROM_HERE, {content::BrowserThread::IO},
                   base::BindOnce(&ProxyingRestrictedCookieManagerQt::CreateAndBindOnIoThread,
                                  profileIoData,
                                  std::move(underlying_rcm),
                                  is_service_worker, process_id, frame_id,
                                  std::move(receiver)));
}


// static
void ProxyingRestrictedCookieManagerQt::CreateAndBindOnIoThread(ProfileIODataQt *profileIoData,
                                                                mojo::PendingRemote<network::mojom::RestrictedCookieManager> underlying_rcm,
                                                                bool is_service_worker,
                                                                int process_id,
                                                                int frame_id,
                                                                mojo::PendingReceiver<network::mojom::RestrictedCookieManager> receiver)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    auto wrapper = base::WrapUnique(new ProxyingRestrictedCookieManagerQt(
                                        profileIoData->getWeakPtrOnIOThread(),
                                        std::move(underlying_rcm),
                                        is_service_worker, process_id, frame_id));
    mojo::MakeSelfOwnedReceiver(std::move(wrapper), std::move(receiver));
}

ProxyingRestrictedCookieManagerQt::ProxyingRestrictedCookieManagerQt(
        base::WeakPtr<ProfileIODataQt> profileIoData,
        mojo::PendingRemote<network::mojom::RestrictedCookieManager> underlyingRestrictedCookieManager,
        bool is_service_worker,
        int32_t process_id,
        int32_t frame_id)
        : m_profileIoData(std::move(profileIoData))
        , underlying_restricted_cookie_manager_(std::move(underlyingRestrictedCookieManager))
        , is_service_worker_(is_service_worker)
        , process_id_(process_id)
        , frame_id_(frame_id)
        , weak_factory_(this)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
}

ProxyingRestrictedCookieManagerQt::~ProxyingRestrictedCookieManagerQt()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
}

void ProxyingRestrictedCookieManagerQt::GetAllForUrl(const GURL &url,
                                                     const net::SiteForCookies &site_for_cookies,
                                                     const url::Origin &top_frame_origin,
                                                     network::mojom::CookieManagerGetOptionsPtr options,
                                                     GetAllForUrlCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    if (allowCookies(url, site_for_cookies)) {
        underlying_restricted_cookie_manager_->GetAllForUrl(url, site_for_cookies, top_frame_origin, std::move(options), std::move(callback));
    } else {
        std::move(callback).Run(std::vector<net::CanonicalCookie>());
    }
}

void ProxyingRestrictedCookieManagerQt::SetCanonicalCookie(const net::CanonicalCookie &cookie,
                                                           const GURL &url,
                                                           const net::SiteForCookies &site_for_cookies,
                                                           const url::Origin &top_frame_origin,
                                                           SetCanonicalCookieCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    if (allowCookies(url, site_for_cookies)) {
        underlying_restricted_cookie_manager_->SetCanonicalCookie(cookie, url, site_for_cookies, top_frame_origin, std::move(callback));
    } else {
        std::move(callback).Run(false);
    }
}

void ProxyingRestrictedCookieManagerQt::AddChangeListener(const GURL &url,
                                                          const net::SiteForCookies &site_for_cookies,
                                                          const url::Origin &top_frame_origin,
                                                          mojo::PendingRemote<network::mojom::CookieChangeListener> listener,
                                                          AddChangeListenerCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    underlying_restricted_cookie_manager_->AddChangeListener(url, site_for_cookies, top_frame_origin, std::move(listener), std::move(callback));
}

void ProxyingRestrictedCookieManagerQt::SetCookieFromString(const GURL &url,
                                                            const net::SiteForCookies &site_for_cookies,
                                                            const url::Origin &top_frame_origin,
                                                            const std::string &cookie,
                                                            SetCookieFromStringCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    if (allowCookies(url, site_for_cookies)) {
        underlying_restricted_cookie_manager_->SetCookieFromString(url, site_for_cookies, top_frame_origin, cookie, std::move(callback));
    } else {
        std::move(callback).Run();
    }
}

void ProxyingRestrictedCookieManagerQt::GetCookiesString(const GURL &url,
                                                         const net::SiteForCookies &site_for_cookies,
                                                         const url::Origin &top_frame_origin,
                                                         GetCookiesStringCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    if (allowCookies(url, site_for_cookies)) {
        underlying_restricted_cookie_manager_->GetCookiesString(url, site_for_cookies, top_frame_origin, std::move(callback));
    } else {
        std::move(callback).Run("");
    }
}

void ProxyingRestrictedCookieManagerQt::CookiesEnabledFor(const GURL &url,
                                                          const net::SiteForCookies &site_for_cookies,
                                                          const url::Origin & /*top_frame_origin*/,
                                                          CookiesEnabledForCallback callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    std::move(callback).Run(allowCookies(url, site_for_cookies));
}

bool ProxyingRestrictedCookieManagerQt::allowCookies(const GURL &url, const net::SiteForCookies &site_for_cookies) const
{
    if (!m_profileIoData)
        return false;
    return m_profileIoData->canGetCookies(toQt(site_for_cookies.RepresentativeUrl()), toQt(url));
}

}  // namespace QtWebEngineCore
