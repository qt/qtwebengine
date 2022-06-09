// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef FILE_SYSTEM_ACCESS_PERMISSION_CONTEXT_FACTORY_QT_H
#define FILE_SYSTEM_ACCESS_PERMISSION_CONTEXT_FACTORY_QT_H

#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

#include "file_system_access_permission_context_qt.h"

namespace QtWebEngineCore {

class FileSystemAccessPermissionContextFactoryQt : public BrowserContextKeyedServiceFactory
{
public:
    static FileSystemAccessPermissionContextQt *GetForProfile(content::BrowserContext *profile);
    static FileSystemAccessPermissionContextQt *
    GetForProfileIfExists(content::BrowserContext *profile);
    static FileSystemAccessPermissionContextFactoryQt *GetInstance();

private:
    friend struct base::DefaultSingletonTraits<FileSystemAccessPermissionContextFactoryQt>;

    FileSystemAccessPermissionContextFactoryQt();
    ~FileSystemAccessPermissionContextFactoryQt() override;

    // BrowserContextKeyedServiceFactory
    content::BrowserContext *
    GetBrowserContextToUse(content::BrowserContext *context) const override;
    KeyedService *BuildServiceInstanceFor(content::BrowserContext *profile) const override;
    void BrowserContextShutdown(content::BrowserContext *context) override;
};

} // namespace QtWebEngineCore

#endif // FILE_SYSTEM_ACCESS_PERMISSION_CONTEXT_FACTORY_QT_H
