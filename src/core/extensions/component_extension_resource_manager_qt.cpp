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

// based on chrome/browser/extensions/chrome_component_extension_resource_manager.cc:
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "component_extension_resource_manager_qt.h"

#include "base/logging.h"
#include "base/path_service.h"
#include "base/stl_util.h"
#include "base/values.h"

#include "chrome/grit/component_extension_resources_map.h"

namespace extensions {

ComponentExtensionResourceManagerQt::ComponentExtensionResourceManagerQt()
{
    AddComponentResourceEntries(kComponentExtensionResources,
                                kComponentExtensionResourcesSize);
}

ComponentExtensionResourceManagerQt::~ComponentExtensionResourceManagerQt() {}

bool ComponentExtensionResourceManagerQt::IsComponentExtensionResource(const base::FilePath &extension_path,
                                                                       const base::FilePath &resource_path,
                                                                       int *resource_id) const
{
    base::FilePath directory_path = extension_path;
    base::FilePath resources_dir;
    base::FilePath relative_path;
    if (!base::PathService::Get(base::DIR_QT_LIBRARY_DATA, &resources_dir)
            || !resources_dir.AppendRelativePath(directory_path, &relative_path)) {
        return false;
    }

    relative_path = relative_path.Append(resource_path);
    relative_path = relative_path.NormalizePathSeparators();

    auto entry = path_to_resource_id_.find(relative_path);
    if (entry != path_to_resource_id_.end()) {
        *resource_id = entry->second;
        return true;
    }

    return false;
}

const ui::TemplateReplacements *ComponentExtensionResourceManagerQt::GetTemplateReplacementsForExtension(const std::string &) const
{
    return nullptr;
}

void ComponentExtensionResourceManagerQt::AddComponentResourceEntries(const GritResourceMap *entries, size_t size)
{
    base::FilePath gen_folder_path = base::FilePath().AppendASCII("@out_folder@/gen/chrome/browser/resources/");
    gen_folder_path = gen_folder_path.NormalizePathSeparators();

    for (size_t i = 0; i < size; ++i) {
        base::FilePath resource_path = base::FilePath().AppendASCII(entries[i].name);
        resource_path = resource_path.NormalizePathSeparators();


        if (!gen_folder_path.IsParent(resource_path)) {
            DCHECK(!base::Contains(path_to_resource_id_, resource_path));
            path_to_resource_id_[resource_path] = entries[i].value;
        } else {
            // If the resource is a generated file, strip the generated folder's path,
            // so that it can be served from a normal URL (as if it were not
            // generated).
            base::FilePath effective_path =
            base::FilePath().AppendASCII(resource_path.AsUTF8Unsafe().substr(
                    gen_folder_path.value().length()));
            DCHECK(!base::Contains(path_to_resource_id_, effective_path));
            path_to_resource_id_[effective_path] = entries[i].value;
        }
    }
}

} // namespace extensions
