// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "extensions_api_provider_qt.h"

#include "chrome/common/extensions/permissions/chrome_api_permissions.h"
#include "chrome/common/extensions/api/generated_schemas.h"
#include "chrome/grit/common_resources.h"

#include "extensions/common/api/api_features.h"
#include "extensions/common/api/behavior_features.h"
#include "extensions/common/api/generated_schemas.h"
#include "extensions/common/api/manifest_features.h"
#include "extensions/common/api/permission_features.h"
#include "extensions/common/common_manifest_handlers.h"
#include "extensions/common/features/feature_provider.h"
#include "extensions/common/features/json_feature_provider_source.h"
#include "extensions/common/permissions/permissions_info.h"
#include "extensions/grit/extensions_resources.h"
#include "qtwebengine/common/extensions/api/generated_schemas.h"

#include "qt_api_features.h"
//#include "qt_behavior_features.h"
#include "qt_permission_features.h"
//#include "qt_manifest_features.h"


namespace extensions {

ExtensionsAPIProviderQt::ExtensionsAPIProviderQt()
{
}

void ExtensionsAPIProviderQt::RegisterManifestHandlers()
{
}

void ExtensionsAPIProviderQt::AddAPIFeatures(FeatureProvider *provider)
{
    AddQtAPIFeatures(provider);
}

void ExtensionsAPIProviderQt::AddAPIJSONSources(JSONFeatureProviderSource *json_source)
{
    json_source->LoadJSON(IDR_CHROME_EXTENSION_API_FEATURES);
}

void ExtensionsAPIProviderQt::AddPermissionFeatures(FeatureProvider *provider)
{
    AddQtPermissionFeatures(provider);
}

bool ExtensionsAPIProviderQt::IsAPISchemaGenerated(const std::string &name)
{
    return api::GeneratedSchemas::IsGenerated(name) ||
            api::ChromeGeneratedSchemas::IsGenerated(name) ||
            api::QtWebEngineGeneratedSchemas::IsGenerated(name);
}

base::StringPiece ExtensionsAPIProviderQt::GetAPISchema(const std::string &name)
{
    if (!api::GeneratedSchemas::Get(name).empty())
        return api::GeneratedSchemas::Get(name);

    if (!api::ChromeGeneratedSchemas::Get(name).empty())
        return api::ChromeGeneratedSchemas::Get(name);

    if (!api::QtWebEngineGeneratedSchemas::Get(name).empty())
        return api::QtWebEngineGeneratedSchemas::Get(name);

    return "";
}

void ExtensionsAPIProviderQt::RegisterPermissions(PermissionsInfo* permissions_info)
{
    permissions_info->RegisterPermissions(
        chrome_api_permissions::GetPermissionInfos(),
        chrome_api_permissions::GetPermissionAliases());
}

}
