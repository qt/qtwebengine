// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CLIENT_HINTS_H_
#define CLIENT_HINTS_H_

// based on components/client_hints/browser/client_hints.h:
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/raw_ptr.h"
#include "base/no_destructor.h"
#include "base/sequence_checker.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/client_hints_controller_delegate.h"
#include "third_party/blink/public/common/user_agent/user_agent_metadata.h"

namespace QtWebEngineCore {

class ClientHints;

class ClientHintsFactory : public BrowserContextKeyedServiceFactory
{
public:
    ClientHintsFactory(const ClientHintsFactory &) = delete;
    ClientHintsFactory &operator=(const ClientHintsFactory &) = delete;

    static ClientHints *GetForBrowserContext(content::BrowserContext *browser_context);
    static ClientHintsFactory *GetInstance();

private:
    friend class base::NoDestructor<ClientHintsFactory>;

    ClientHintsFactory();
    ~ClientHintsFactory() override;

    // BrowserContextKeyedServiceFactory methods:
    KeyedService *BuildServiceInstanceFor(content::BrowserContext *profile) const override;
    content::BrowserContext *GetBrowserContextToUse(content::BrowserContext *context) const override;
};

class ClientHints : public KeyedService, public content::ClientHintsControllerDelegate
{
public:
    ClientHints(content::BrowserContext *context);

    ClientHints(const ClientHints &) = delete;
    ClientHints &operator=(const ClientHints &) = delete;

    ~ClientHints() override;

    // content::ClientHintsControllerDelegate:
    network::NetworkQualityTracker *GetNetworkQualityTracker() override;

    void GetAllowedClientHintsFromSource(const url::Origin &origin, blink::EnabledClientHints *client_hints) override;

    bool IsJavaScriptAllowed(const GURL &url, content::RenderFrameHost *parent_rfh) override;

    bool AreThirdPartyCookiesBlocked(const GURL &url, content::RenderFrameHost *rfh) override;

    blink::UserAgentMetadata GetUserAgentMetadata() override;

    void PersistClientHints(const url::Origin &primary_origin,
                            content::RenderFrameHost *parent_rfh,
                            const std::vector<network::mojom::WebClientHintsType> &client_hints) override;

    void SetAdditionalClientHints(const std::vector<network::mojom::WebClientHintsType> &) override;

    void ClearAdditionalClientHints() override;

    void SetMostRecentMainFrameViewportSize(const gfx::Size&) override;
    gfx::Size GetMostRecentMainFrameViewportSize() override;

private:
    SEQUENCE_CHECKER(sequence_checker_);

    raw_ptr<content::BrowserContext> context_ = nullptr;

    // Stores enabled Client Hint types for an origin.
    std::map<url::Origin, blink::EnabledClientHints> accept_ch_cache_
        GUARDED_BY_CONTEXT(sequence_checker_);

    // Additional Client Hint types for Client Hints Reliability. If additional
    // hints are set, they would be included by subsequent calls to
    // GetAllowedClientHintsFromSource.
    std::vector<network::mojom::WebClientHintsType> additional_hints_
        GUARDED_BY_CONTEXT(sequence_checker_);

    std::unique_ptr<network::NetworkQualityTracker> network_quality_tracker_
        GUARDED_BY_CONTEXT(sequence_checker_);

    // This stores the viewport size of the most recent visible main frame tree
    // node. This value is only used when the viewport size cannot be directly
    // queried such as for prefetch requests and for tab restores.
    gfx::Size viewport_size_;
};

} // namespace QtWebEngineCore

#endif
