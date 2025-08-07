// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "extension_action_manager.h"

#include "extensions/common/extension.h"

using namespace extensions;

namespace QtWebEngineCore {
void ExtensionActionManager::removeExtensionAction(const std::string &id)
{
    m_actions.erase(id);
}

ExtensionAction *ExtensionActionManager::getExtensionAction(const Extension *extension)
{
    if (!extension)
        return nullptr;
    // Todo: adapt and use "extensions/browser/extension_action_manager.h" instead
    // which is used by some of the js extension apis.
    auto iter = m_actions.find(extension->id());
    if (iter != m_actions.end())
        return iter->second.get();

    const ActionInfo *action_info = ActionInfo::GetExtensionActionInfo(extension);
    if (!action_info)
        return nullptr;

    auto action = std::make_unique<ExtensionAction>(*extension, *action_info);
    ExtensionAction *raw_action = action.get();
    m_actions[extension->id()] = std::move(action);
    return raw_action;
}
} // namespace QtWebEngineCore
