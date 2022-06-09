// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/custom_handlers/protocol_handler_registry_factory.h:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PROTOCOL_HANDLER_REGISTRY_FACTORY_H_
#define PROTOCOL_HANDLER_REGISTRY_FACTORY_H_

#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace custom_handlers {
class ProtocolHandlerRegistry;
}

namespace base {
template <typename T> struct DefaultSingletonTraits;
}

namespace QtWebEngineCore {

// Singleton that owns all ProtocolHandlerRegistrys and associates them with
// Profiles. Listens for the Profile's destruction notification and cleans up
// the associated ProtocolHandlerRegistry.
class ProtocolHandlerRegistryFactory : public BrowserContextKeyedServiceFactory {
public:
    // Returns the singleton instance of the ProtocolHandlerRegistryFactory.
    static ProtocolHandlerRegistryFactory *GetInstance();

    // Returns the ProtocolHandlerRegistry that provides intent registration for
    // |context|. Ownership stays with this factory object.
    static custom_handlers::ProtocolHandlerRegistry *GetForBrowserContext(content::BrowserContext *context);

    ProtocolHandlerRegistryFactory(const ProtocolHandlerRegistryFactory &) = delete;
    ProtocolHandlerRegistryFactory &operator=(const ProtocolHandlerRegistryFactory &) = delete;

protected:
    // BrowserContextKeyedServiceFactory implementation.
    bool ServiceIsCreatedWithBrowserContext() const override;
    content::BrowserContext *GetBrowserContextToUse(content::BrowserContext *context) const override;
    bool ServiceIsNULLWhileTesting() const override;

private:
    friend struct base::DefaultSingletonTraits<ProtocolHandlerRegistryFactory>;

    ProtocolHandlerRegistryFactory();
    ~ProtocolHandlerRegistryFactory() override;

    // BrowserContextKeyedServiceFactory implementation.
    KeyedService *BuildServiceInstanceFor(content::BrowserContext *profile) const override;
};

} // namespace QtWebEngineCore

#endif  // PROTOCOL_HANDLER_REGISTRY_FACTORY_H_
