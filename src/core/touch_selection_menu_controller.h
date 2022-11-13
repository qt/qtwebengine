// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef TOUCH_SELECTION_MENU_CONTROLLER_H
#define TOUCH_SELECTION_MENU_CONTROLLER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QtCore/QObject>

namespace QtWebEngineCore {

class TouchSelectionControllerClientQt;

class Q_WEBENGINECORE_PRIVATE_EXPORT TouchSelectionMenuController : public QObject {
    Q_OBJECT
public:
    enum TouchSelectionCommandFlag {
        Cut = 0x1,
        Copy = 0x2,
        Paste = 0x4
    };
    Q_DECLARE_FLAGS(TouchSelectionCommandFlags, TouchSelectionCommandFlag)
    Q_FLAG(TouchSelectionCommandFlag)

    TouchSelectionMenuController(TouchSelectionControllerClientQt *touchSelectionControllerClient);
    ~TouchSelectionMenuController();
    int buttonCount();
    bool isCommandEnabled(TouchSelectionCommandFlag);
    TouchSelectionCommandFlags availableActions();

public Q_SLOTS:
    void cut();
    void copy();
    void paste();
    void runContextMenu();

private:
    TouchSelectionControllerClientQt *m_touchSelectionControllerClient;
};

} // namespace QtWebEngineCore

#endif // TOUCH_SELECTION_CONTROLLER_CLIENT_QT_H
