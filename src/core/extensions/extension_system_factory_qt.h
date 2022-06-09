// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSION_SYSTEM_FACTORY_QT_H_
#define EXTENSION_SYSTEM_FACTORY_QT_H_

#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "extensions/browser/extension_system_provider.h"
#include "extension_system_qt.h"

namespace extensions {
class ExtensionSystem;

// BrowserContextKeyedServiceFactory for ExtensionSystemImpl.
// TODO(yoz): Rename to ExtensionSystemImplFactory.
class ExtensionSystemFactoryQt : public ExtensionSystemProvider
{
public:
    // ExtensionSystem provider implementation:
    ExtensionSystem *GetForBrowserContext(content::BrowserContext *context) override;

    static ExtensionSystemFactoryQt *GetInstance();

private:
    friend struct base::DefaultSingletonTraits<ExtensionSystemFactoryQt>;

    ExtensionSystemFactoryQt();
    ~ExtensionSystemFactoryQt() override;

    // BrowserContextKeyedServiceFactory implementation:
    KeyedService *BuildServiceInstanceFor(content::BrowserContext *context) const override;
    content::BrowserContext *GetBrowserContextToUse(content::BrowserContext *context) const override;
    bool ServiceIsCreatedWithBrowserContext() const override;
};

} // namespace extensions

#endif // EXTENSION_SYSTEM_FACTORY_QT_H_
