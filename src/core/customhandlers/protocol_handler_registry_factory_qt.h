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

#ifndef PROTOCOL_HANDLER_REGISTRY_FACTORY_QT_H_
#define PROTOCOL_HANDLER_REGISTRY_FACTORY_QT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "components/browser_context_keyed_service/browser_context_keyed_service_factory.h"

class ProtocolHandlerRegistryQt;

template <typename T> struct DefaultSingletonTraits;

// Singleton that owns all ProtocolHandlerRegistrys and associates them with
// Profiles. Listens for the Profile's destruction notification and cleans up
// the associated ProtocolHandlerRegistryQt.
class ProtocolHandlerRegistryFactoryQt: public BrowserContextKeyedServiceFactory {

public:
    // Returns the singleton instance of the ProtocolHandlerRegistryFactoryQt.
    static ProtocolHandlerRegistryFactoryQt *GetInstance();

    // Returns the ProtocolHandlerRegistryQt that provides intent registration for
    // |profile|. Ownership stays with this factory object.
    static ProtocolHandlerRegistryQt *GetForProfile(content::BrowserContext *context);

protected:
    // BrowserContextKeyedServiceFactory implementation.
    virtual bool ServiceIsCreatedWithBrowserContext() const OVERRIDE;

    virtual content::BrowserContext *GetBrowserContextToUse(content::BrowserContext *context) const OVERRIDE;
    virtual bool ServiceIsNULLWhileTesting() const OVERRIDE;

private:
    friend struct DefaultSingletonTraits<ProtocolHandlerRegistryFactoryQt>;

    ProtocolHandlerRegistryFactoryQt();
    virtual ~ProtocolHandlerRegistryFactoryQt();

    // BrowserContextKeyedServiceFactory implementation.
    virtual BrowserContextKeyedService *BuildServiceInstanceFor(
            content::BrowserContext *context) const OVERRIDE;

    DISALLOW_COPY_AND_ASSIGN (ProtocolHandlerRegistryFactoryQt);
};

#endif  // PROTOCOL_HANDLER_REGISTRY_FACTORY_QT_H_
