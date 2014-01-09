/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "protocol_handler_registry_factory_qt.h"

#include "base/memory/singleton.h"
#include "protocol_handler_registry_qt.h"
#include "components/browser_context_keyed_service/browser_context_dependency_manager.h"

// static
ProtocolHandlerRegistryFactoryQt *ProtocolHandlerRegistryFactoryQt::GetInstance()
{
    return Singleton<ProtocolHandlerRegistryFactoryQt>::get();
}

// static
ProtocolHandlerRegistryQt *ProtocolHandlerRegistryFactoryQt::GetForProfile(
        content::BrowserContext *context)
{
    return static_cast<ProtocolHandlerRegistryQt *>(GetInstance()->GetServiceForBrowserContext(
            context, true));
}

ProtocolHandlerRegistryFactoryQt::ProtocolHandlerRegistryFactoryQt()
        : BrowserContextKeyedServiceFactory("ProtocolHandlerRegistryQt",
                BrowserContextDependencyManager::GetInstance())
{
}

ProtocolHandlerRegistryFactoryQt::~ProtocolHandlerRegistryFactoryQt()
{
}

// Will be created when initializing profile_io_data, so we might
// as well have the framework create this along with other
// PKSs to preserve orderly civic conduct :)
bool ProtocolHandlerRegistryFactoryQt::ServiceIsCreatedWithBrowserContext() const
{
    return true;
}

// Allows the produced registry to be used in incognito mode.
content::BrowserContext *ProtocolHandlerRegistryFactoryQt::GetBrowserContextToUse(
        content::BrowserContext *context) const
{
    return context;
}

// Do not create this service for tests. MANY tests will fail
// due to the threading requirements of this service. ALSO,
// not creating this increases test isolation (which is GOOD!)
bool ProtocolHandlerRegistryFactoryQt::ServiceIsNULLWhileTesting() const
{
    return true;
}

BrowserContextKeyedService *ProtocolHandlerRegistryFactoryQt::BuildServiceInstanceFor(
        content::BrowserContext *context) const
{
    ProtocolHandlerRegistryQt *registry = new ProtocolHandlerRegistryQt(context);

    return registry;
}
