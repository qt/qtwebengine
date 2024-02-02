// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "client_hints.h"

#include "web_contents_delegate_qt.h"
#include "web_engine_settings.h"

#include "components/embedder_support/user_agent_utils.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/network_service_instance.h"
#include "extensions/buildflags/buildflags.h"
#include "services/network/public/cpp/is_potentially_trustworthy.h"
#include "services/network/public/cpp/network_quality_tracker.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "components/guest_view/browser/guest_view_base.h"
#endif

namespace QtWebEngineCore {

// based on weblayer/browser/client_hints_factory.cc:
// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// static
ClientHints *ClientHintsFactory::GetForBrowserContext(content::BrowserContext *browser_context)
{
    return static_cast<ClientHints*>(GetInstance()->GetServiceForBrowserContext(browser_context, true));
}

// static
ClientHintsFactory *ClientHintsFactory::GetInstance()
{
    static base::NoDestructor<ClientHintsFactory> factory;
    return factory.get();
}

ClientHintsFactory::ClientHintsFactory()
        : BrowserContextKeyedServiceFactory("ClientHints", BrowserContextDependencyManager::GetInstance())
{
}

ClientHintsFactory::~ClientHintsFactory() = default;

KeyedService *ClientHintsFactory::BuildServiceInstanceFor(content::BrowserContext *context) const
{
    return new ClientHints(context);
}

content::BrowserContext *ClientHintsFactory::GetBrowserContextToUse(content::BrowserContext *context) const
{
    return context;
}

// based on components/client_hints/browser/in_memory_client_hints_controller_delegate.cc:
// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

ClientHints::ClientHints(content::BrowserContext *context)
{
}

ClientHints::~ClientHints() = default;

// Enabled Client Hints are only cached and not persisted in this
// implementation.
void ClientHints::PersistClientHints(const url::Origin &primary_origin,
                                     content::RenderFrameHost *parent_rfh,
                                     const std::vector<network::mojom::WebClientHintsType> &client_hints)
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    const GURL primary_url = primary_origin.GetURL();
    DCHECK(primary_url.is_valid());
    if (!network::IsUrlPotentiallyTrustworthy(primary_url))
        return;

    // Client hints should only be enabled when JavaScript is enabled.
    if (!IsJavaScriptAllowed(primary_url, parent_rfh))
        return;

    blink::EnabledClientHints enabled_hints;
    for (auto hint : client_hints) {
        enabled_hints.SetIsEnabled(hint, true);
    }
    accept_ch_cache_[primary_origin] = enabled_hints;
}

// Looks up enabled Client Hints for the URL origin, and adds additional Client
// Hints if set.
void ClientHints::GetAllowedClientHintsFromSource(const url::Origin &origin,
                                                  blink::EnabledClientHints *client_hints)
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    DCHECK(client_hints);
    // Can not assert this, as we get here for unregistered custom schemes:
    if (!network::IsOriginPotentiallyTrustworthy(origin))
        return;

    const auto &it = accept_ch_cache_.find(origin);
    if (it != accept_ch_cache_.end()) {
        *client_hints = it->second;
    }

    for (auto hint : additional_hints_)
        client_hints->SetIsEnabled(hint, true);
}

void ClientHints::SetAdditionalClientHints(const std::vector<network::mojom::WebClientHintsType> &hints)
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    additional_hints_ = hints;
}

void ClientHints::ClearAdditionalClientHints()
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    additional_hints_.clear();
}

network::NetworkQualityTracker *ClientHints::GetNetworkQualityTracker()
{
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    if (!network_quality_tracker_) {
        network_quality_tracker_ =
                std::make_unique<network::NetworkQualityTracker>(
                    base::BindRepeating(&content::GetNetworkService));
    }
    return network_quality_tracker_.get();
}

bool ClientHints::IsJavaScriptAllowed(const GURL &url, content::RenderFrameHost *parent_rfh)
{
    content::WebContents *webContents = content::WebContents::FromRenderFrameHost(parent_rfh);

#if BUILDFLAG(ENABLE_EXTENSIONS)
    if (webContents && guest_view::GuestViewBase::IsGuest(webContents))
        webContents = guest_view::GuestViewBase::GetTopLevelWebContents(webContents);
#endif

    if (webContents) {
        WebContentsDelegateQt* delegate =
                static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());
        if (delegate) {
            WebEngineSettings *settings = delegate->webEngineSettings();
            if (settings)
                return settings->testAttribute(QWebEngineSettings::JavascriptEnabled);
        }
    }
    return true;
}

bool ClientHints::AreThirdPartyCookiesBlocked(const GURL &url, content::RenderFrameHost *rfh)
{
    return false; // we probably can not report anything more specific
}

blink::UserAgentMetadata ClientHints::GetUserAgentMetadata()
{
    return embedder_support::GetUserAgentMetadata();
}

void ClientHints::SetMostRecentMainFrameViewportSize(
    const gfx::Size& viewport_size) {
  viewport_size_ = viewport_size;
}

gfx::Size
ClientHints::GetMostRecentMainFrameViewportSize() {
  return viewport_size_;
}
} // namespace
