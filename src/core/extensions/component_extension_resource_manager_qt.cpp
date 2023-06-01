// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/extensions/chrome_component_extension_resource_manager.cc:
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "component_extension_resource_manager_qt.h"

#include "base/check.h"
#include "base/containers/contains.h"
#include "base/path_service.h"
#include "base/stl_util.h"
#include "base/values.h"
#include "chrome/grit/component_extension_resources_map.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/common/constants.h"
#include "pdf/buildflags.h"
#include "ppapi/buildflags/buildflags.h"

#if BUILDFLAG(ENABLE_PLUGINS)
#include "chrome/grit/pdf_resources_map.h"
#endif

#if BUILDFLAG(ENABLE_PDF)
#include "qtwebengine/browser/pdf/pdf_extension_util.h"
#endif  // BUILDFLAG(ENABLE_PDF)

namespace extensions {

ComponentExtensionResourceManagerQt::ComponentExtensionResourceManagerQt()
{
    AddComponentResourceEntries(kComponentExtensionResources,
                                kComponentExtensionResourcesSize);
#if BUILDFLAG(ENABLE_PLUGINS)
    AddComponentResourceEntries(kPdfResources, kPdfResourcesSize);
#endif
#if BUILDFLAG(ENABLE_PDF)
    base::Value::Dict dict;
    pdf_extension_util::AddStrings(pdf_extension_util::PdfViewerContext::kPdfViewer, &dict);
    pdf_extension_util::AddAdditionalData(/*enable_annotations=*/true, &dict);

    ui::TemplateReplacements pdf_viewer_replacements;
    ui::TemplateReplacementsFromDictionaryValue(dict, &pdf_viewer_replacements);
    template_replacements_[extension_misc::kPdfExtensionId] = std::move(pdf_viewer_replacements);
#endif
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

const ui::TemplateReplacements *ComponentExtensionResourceManagerQt::GetTemplateReplacementsForExtension(const std::string &extension_id) const
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    auto it = template_replacements_.find(extension_id);
    return it != template_replacements_.end() ? &it->second : nullptr;
}

void ComponentExtensionResourceManagerQt::AddComponentResourceEntries(const webui::ResourcePath *entries, size_t size)
{
    base::FilePath gen_folder_path = base::FilePath().AppendASCII("@out_folder@/gen/chrome/browser/resources/");
    gen_folder_path = gen_folder_path.NormalizePathSeparators();

    for (size_t i = 0; i < size; ++i) {
        base::FilePath resource_path = base::FilePath().AppendASCII(entries[i].path);
        resource_path = resource_path.NormalizePathSeparators();

        if (!gen_folder_path.IsParent(resource_path)) {
            DCHECK(!base::Contains(path_to_resource_id_, resource_path));
            path_to_resource_id_[resource_path] = entries[i].id;
        } else {
            // If the resource is a generated file, strip the generated folder's path,
            // so that it can be served from a normal URL (as if it were not
            // generated).
            base::FilePath effective_path =
            base::FilePath().AppendASCII(resource_path.AsUTF8Unsafe().substr(
                    gen_folder_path.value().length()));
            DCHECK(!base::Contains(path_to_resource_id_, effective_path));
            path_to_resource_id_[effective_path] = entries[i].id;
        }
    }
}

} // namespace extensions
