/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

// based on chrome/browser/custom_handlers/protocol_handler_registry_factory.cc
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "protocol_handler_registry_factory.h"

#include <memory>

#include "base/memory/singleton.h"
#include "components/custom_handlers/protocol_handler_registry.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"

#include "protocol_handler_registry_delegate_qt.h"

namespace QtWebEngineCore {

// static
ProtocolHandlerRegistryFactory *ProtocolHandlerRegistryFactory::GetInstance()
{
    return base::Singleton<ProtocolHandlerRegistryFactory>::get();
}

// static
custom_handlers::ProtocolHandlerRegistry *ProtocolHandlerRegistryFactory::GetForBrowserContext(content::BrowserContext *context)
{
    return static_cast<custom_handlers::ProtocolHandlerRegistry *>(GetInstance()->GetServiceForBrowserContext(context, true));
}

ProtocolHandlerRegistryFactory::ProtocolHandlerRegistryFactory()
        : BrowserContextKeyedServiceFactory("ProtocolHandlerRegistry", BrowserContextDependencyManager::GetInstance()) {}

ProtocolHandlerRegistryFactory::~ProtocolHandlerRegistryFactory()
{
}

// Will be created when initializing profile_io_data, so we might
// as well have the framework create this along with other
// PKSs to preserve orderly civic conduct :)
bool ProtocolHandlerRegistryFactory::ServiceIsCreatedWithBrowserContext() const
{
    return true;
}

// Allows the produced registry to be used in incognito mode.
content::BrowserContext *ProtocolHandlerRegistryFactory::GetBrowserContextToUse(content::BrowserContext *context) const
{
    return context;
//   return chrome::GetBrowserContextRedirectedInIncognito(context);
}

// Do not create this service for tests. MANY tests will fail
// due to the threading requirements of this service. ALSO,
// not creating this increases test isolation (which is GOOD!)
bool ProtocolHandlerRegistryFactory::ServiceIsNULLWhileTesting() const
{
    return true;
}

KeyedService *ProtocolHandlerRegistryFactory::BuildServiceInstanceFor(content::BrowserContext *context) const
{
    custom_handlers::ProtocolHandlerRegistry *registry =
        new custom_handlers::ProtocolHandlerRegistry(context,
                                                     std::make_unique<ProtocolHandlerRegistryDelegateQt>());

    // Must be called as a part of the creation process.
    registry->InitProtocolSettings();

    return registry;
}

} // namespace QtWebEngineCore
