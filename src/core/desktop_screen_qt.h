// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DESKTOP_SCREEN_QT_H
#define DESKTOP_SCREEN_QT_H

#include "ui/display/screen_base.h"

#include <qmetaobject.h>

namespace QtWebEngineCore {

class DesktopScreenQt : public display::ScreenBase
{
public:
    DesktopScreenQt();
    ~DesktopScreenQt() override;

    display::Display GetDisplayNearestWindow(gfx::NativeWindow /*window*/) const override;
#if BUILDFLAG(IS_CHROMEOS_LACROS) || BUILDFLAG(IS_LINUX)
    std::unique_ptr<ScreenSaverSuspender> SuspendScreenSaver() override;
#endif
    bool IsScreenSaverActive() const override;

private:
    void initializeScreens();
    bool updateAllScreens();
    QMetaObject::Connection m_connections[3];
};

} // namespace QtWebEngineCore

#endif // DESKTOP_SCREEN_QT_H
