// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "touch_selection_controller_client_qt.h"
#include "touch_selection_menu_controller.h"

namespace QtWebEngineCore {

TouchSelectionMenuController::TouchSelectionMenuController(TouchSelectionControllerClientQt *touchSelectionControllerClient)
    : m_touchSelectionControllerClient(touchSelectionControllerClient)
{
}

TouchSelectionMenuController::~TouchSelectionMenuController()
{
}

int TouchSelectionMenuController::buttonCount()
{
    // Context menu should be always available
    return qPopulationCount(static_cast<quint8>(availableActions())) + 1;
}

bool TouchSelectionMenuController::isCommandEnabled(TouchSelectionMenuController::TouchSelectionCommandFlag command)
{
    return m_touchSelectionControllerClient->IsCommandIdEnabled(static_cast<int>(command));
}

void TouchSelectionMenuController::cut()
{
    m_touchSelectionControllerClient->ExecuteCommand(static_cast<int>(Cut), 0);
}

void TouchSelectionMenuController::copy()
{
    m_touchSelectionControllerClient->ExecuteCommand(static_cast<int>(Copy), 0);
}

void TouchSelectionMenuController::paste()
{
    m_touchSelectionControllerClient->ExecuteCommand(static_cast<int>(Paste), 0);
}

void TouchSelectionMenuController::runContextMenu()
{
    return m_touchSelectionControllerClient->RunContextMenu();
}

TouchSelectionMenuController::TouchSelectionCommandFlags TouchSelectionMenuController::availableActions()
{
    TouchSelectionCommandFlags availableActions;

    if (m_touchSelectionControllerClient->IsCommandIdEnabled(Cut)) {
        availableActions |= TouchSelectionMenuController::Cut;
    }
    if (m_touchSelectionControllerClient->IsCommandIdEnabled(Copy)) {
        availableActions |= TouchSelectionMenuController::Copy;
    }
    if (m_touchSelectionControllerClient->IsCommandIdEnabled(Paste)) {
        availableActions |= TouchSelectionMenuController::Paste;
    }

    return availableActions;
}

} // namespace QtWebEngineCore
