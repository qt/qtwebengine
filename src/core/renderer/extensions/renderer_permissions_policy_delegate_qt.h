// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef RENDERERPERMISSIONSPOLICYDELEGATEQT_H
#define RENDERERPERMISSIONSPOLICYDELEGATEQT_H

#include "extensions/common/permissions/permissions_data.h"

namespace extensions {
class Dispatcher;
}

namespace QtWebEngineCore {

class RendererPermissionsPolicyDelegateQt : public extensions::PermissionsData::PolicyDelegate
{
public:
    explicit RendererPermissionsPolicyDelegateQt(extensions::Dispatcher *dispatcher);
    ~RendererPermissionsPolicyDelegateQt() override;

    bool IsRestrictedUrl(const GURL &, std::string *) override;
};

} // namespace QtWebEngineCore

#endif // RENDERERPERMISSIONSPOLICYDELEGATEQT_H
