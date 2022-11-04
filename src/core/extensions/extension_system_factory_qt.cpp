// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extension_system_factory_qt.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "extensions/browser/event_router_factory.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extension_registry_factory.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/browser/process_manager_factory.h"
#include "extensions/browser/renderer_startup_helper.h"

namespace extensions {

// static
ExtensionSystem *ExtensionSystemFactoryQt::GetForBrowserContext(content::BrowserContext *context)
{
    return static_cast<ExtensionSystem *>(GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
ExtensionSystemFactoryQt *ExtensionSystemFactoryQt::GetInstance()
{
    return base::Singleton<ExtensionSystemFactoryQt>::get();
}

ExtensionSystemFactoryQt::ExtensionSystemFactoryQt()
        : ExtensionSystemProvider("ExtensionSystem", BrowserContextDependencyManager::GetInstance())
{
    DCHECK(ExtensionsBrowserClient::Get()) << "ExtensionSystemFactory must be initialized after BrowserProcess";
    DependsOn(ExtensionPrefsFactory::GetInstance());
    DependsOn(ExtensionRegistryFactory::GetInstance());
}

ExtensionSystemFactoryQt::~ExtensionSystemFactoryQt()
{
}

KeyedService *ExtensionSystemFactoryQt::BuildServiceInstanceFor(content::BrowserContext *context) const
{
    return new ExtensionSystemQt(context);
}

content::BrowserContext *ExtensionSystemFactoryQt::GetBrowserContextToUse(content::BrowserContext *context) const
{
    // Separate instance in incognito.
    return context;
}

bool ExtensionSystemFactoryQt::ServiceIsCreatedWithBrowserContext() const
{
    return true;
}

} // namespace extensions
