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

#include "extensions_api_provider_qt.h"

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
    return api::GeneratedSchemas::IsGenerated(name);
}

base::StringPiece ExtensionsAPIProviderQt::GetAPISchema(const std::string &name)
{
    return api::GeneratedSchemas::Get(name);
}

void ExtensionsAPIProviderQt::RegisterPermissions(PermissionsInfo* permissions_info)
{
}

}
