// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef EXTENSIONS_API_PROVIDER_QT_H
#define EXTENSIONS_API_PROVIDER_QT_H

#include "extensions/common/extensions_api_provider.h"

namespace extensions {

class ExtensionsAPIProviderQt : public ExtensionsAPIProvider
{
public:
    ExtensionsAPIProviderQt();

    void RegisterManifestHandlers() override;
    void AddAPIFeatures(FeatureProvider *provider) override;
    void AddAPIJSONSources(JSONFeatureProviderSource* json_source) override;
    void AddPermissionFeatures(FeatureProvider* provider) override;

    bool IsAPISchemaGenerated(const std::string& name) override;
    base::StringPiece GetAPISchema(const std::string& name) override;

    // Adds feature definitions to the given |provider| of the specified type.
    void AddManifestFeatures(FeatureProvider* provider) override { }
    void AddBehaviorFeatures(FeatureProvider* provider) override { }

    // Registers permissions for any associated API features.
    void RegisterPermissions(PermissionsInfo* permissions_info) override;
};

}

#endif // EXTENSIONS_API_PROVIDER_QT_H
