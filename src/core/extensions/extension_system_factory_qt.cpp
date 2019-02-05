/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extension_system_factory_qt.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "extensions/browser/declarative_user_script_manager_factory.h"
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
