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

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENT_EXTENSION_RESOURCE_MANAGER_QT_H_
#define COMPONENT_EXTENSION_RESOURCE_MANAGER_QT_H_

#include <map>

#include "base/files/file_path.h"
#include "extensions/browser/component_extension_resource_manager.h"

struct GritResourceMap;

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
    void AddComponentResourceEntries(const GritResourceMap *entries, size_t size);

    // A map from a resource path to the resource ID.  Used by
    // IsComponentExtensionResource.
    std::map<base::FilePath, int> path_to_resource_id_;

    DISALLOW_COPY_AND_ASSIGN(ComponentExtensionResourceManagerQt);
};

} // namespace extensions

#endif // COMPONENT_EXTENSION_RESOURCE_MANAGER_QT_H_
