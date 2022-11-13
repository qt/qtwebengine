// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENT_EXTENSION_RESOURCE_MANAGER_QT_H_
#define COMPONENT_EXTENSION_RESOURCE_MANAGER_QT_H_

#include <map>

#include "base/files/file_path.h"
#include "extensions/browser/component_extension_resource_manager.h"
#include "ui/base/webui/resource_path.h"

namespace extensions {

class ComponentExtensionResourceManagerQt : public ComponentExtensionResourceManager
{
public:
    ComponentExtensionResourceManagerQt();
    ~ComponentExtensionResourceManagerQt() override;

    // Overridden from ComponentExtensionResourceManager:
    bool IsComponentExtensionResource(const base::FilePath &extension_path,
                                      const base::FilePath &resource_path,
                                      int *resource_id) const override;
    const ui::TemplateReplacements *GetTemplateReplacementsForExtension(const std::string &extension_id) const override;

private:
    void AddComponentResourceEntries(const webui::ResourcePath *entries, size_t size);

    // A map from a resource path to the resource ID.  Used by
    // IsComponentExtensionResource.
    std::map<base::FilePath, int> path_to_resource_id_;

    // A map from an extension ID to its i18n template replacements.
    std::map<std::string, ui::TemplateReplacements> template_replacements_;
};

} // namespace extensions

#endif // COMPONENT_EXTENSION_RESOURCE_MANAGER_QT_H_
