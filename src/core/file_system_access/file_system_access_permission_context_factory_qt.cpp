// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "file_system_access_permission_context_factory_qt.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"

#include <QtGlobal>

namespace QtWebEngineCore {

// static
FileSystemAccessPermissionContextQt *
FileSystemAccessPermissionContextFactoryQt::GetForProfile(content::BrowserContext *profile)
{
    return static_cast<FileSystemAccessPermissionContextQt *>(
            GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
FileSystemAccessPermissionContextQt *
FileSystemAccessPermissionContextFactoryQt::GetForProfileIfExists(content::BrowserContext *profile)
{
    return static_cast<FileSystemAccessPermissionContextQt *>(
            GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
FileSystemAccessPermissionContextFactoryQt *
FileSystemAccessPermissionContextFactoryQt::GetInstance()
{
    return base::Singleton<FileSystemAccessPermissionContextFactoryQt>::get();
}

FileSystemAccessPermissionContextFactoryQt::FileSystemAccessPermissionContextFactoryQt()
    : BrowserContextKeyedServiceFactory("FileSystemAccessPermissionContext",
                                        BrowserContextDependencyManager::GetInstance())
{
}

FileSystemAccessPermissionContextFactoryQt::~FileSystemAccessPermissionContextFactoryQt() = default;

content::BrowserContext *FileSystemAccessPermissionContextFactoryQt::GetBrowserContextToUse(
        content::BrowserContext *context) const
{
    return context;
}

KeyedService *FileSystemAccessPermissionContextFactoryQt::BuildServiceInstanceFor(
        content::BrowserContext *context) const
{
    return new FileSystemAccessPermissionContextQt(context);
}

void FileSystemAccessPermissionContextFactoryQt::BrowserContextShutdown(
        content::BrowserContext *context)
{
    Q_UNUSED(context);
}

} // namespace QtWebEngineCore
