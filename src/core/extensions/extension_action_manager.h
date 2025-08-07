// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef EXTENSION_ACTION_MANAGER_H_
#define EXTENSION_ACTION_MANAGER_H_

#include <map>
#include <memory>
#include <string>

#include "extensions/browser/extension_action.h"

namespace extensions {
class Extension;
}
namespace QtWebEngineCore {
class ExtensionActionManager
{
public:
    ExtensionActionManager() = default;
    void removeExtensionAction(const std::string &id);
    extensions::ExtensionAction *getExtensionAction(const extensions::Extension *extension);

private:
    using ExtensionActionMap = std::map<std::string, std::unique_ptr<extensions::ExtensionAction>>;
    ExtensionActionMap m_actions{};
};
}

#endif // EXTENSION_ACTION_MANAGER_H_
