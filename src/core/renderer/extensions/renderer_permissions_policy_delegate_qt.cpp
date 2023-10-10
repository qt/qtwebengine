// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "renderer_permissions_policy_delegate_qt.h"

#include "extensions/common/constants.h"
#include "extensions/common/extensions_client.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/switches.h"
#include "extensions/renderer/dispatcher.h"

namespace QtWebEngineCore {

RendererPermissionsPolicyDelegateQt::RendererPermissionsPolicyDelegateQt(extensions::Dispatcher *)
{
    extensions::PermissionsData::SetPolicyDelegate(this);
}

RendererPermissionsPolicyDelegateQt::~RendererPermissionsPolicyDelegateQt()
{
    extensions::PermissionsData::SetPolicyDelegate(nullptr);
}

bool RendererPermissionsPolicyDelegateQt::IsRestrictedUrl(const GURL &, std::string *)
{
    return false;
}

} // namespace QtWebEngineCore
