// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "desktop_screen_qt.h"

#include "ui/display/display.h"

#include "type_conversion.h"

#include <QGuiApplication>
#include <QScreen>

#if defined(USE_OZONE)
#include "ui/ozone/buildflags.h"
#if BUILDFLAG(OZONE_PLATFORM_X11)
#define USE_XSCREENSAVER
#include "ui/base/x/x11_screensaver.h"
#include "ui/base/x/x11_util.h"
#endif
#endif

#include <cmath>

namespace QtWebEngineCore {

static display::Display::Rotation toDisplayRotation(Qt::ScreenOrientation orientation)
{
    switch (orientation) {
    case Qt::PrimaryOrientation:
    case Qt::LandscapeOrientation:
        return display::Display::ROTATE_0;
    case Qt::PortraitOrientation:
        return display::Display::ROTATE_90;
    case Qt::InvertedLandscapeOrientation:
        return display::Display::ROTATE_180;
    case Qt::InvertedPortraitOrientation:
        return display::Display::ROTATE_270;
    }
}

display::Display toDisplayDisplay(int id, const QScreen *screen)
{
    auto display = display::Display(id, toGfx(screen->geometry()));
    display.set_work_area(toGfx(screen->availableGeometry()));
    display.set_is_monochrome(screen->depth() == 1);
    display.set_color_depth(screen->depth());
    display.set_depth_per_component(8); // FIXME: find the real value
    display.set_display_frequency(std::ceil(screen->refreshRate()));
    display.set_rotation(toDisplayRotation(screen->orientation()));

    // FIXME: support lower scale factor
    float pixelRatio = screen->devicePixelRatio();
    if (pixelRatio < 1) {
        qWarning("Unsupported scale factor (%f) detected on Display%d", pixelRatio, id);
        display.set_device_scale_factor(qGuiApp->devicePixelRatio());
    } else {
        display.set_device_scale_factor(pixelRatio);
    }

    if (screen->nativeOrientation() != Qt::PrimaryOrientation)
        display.set_panel_rotation(toDisplayRotation(screen->nativeOrientation()));
    return display;
}

DesktopScreenQt::DesktopScreenQt()
{
    initializeScreens();
}

DesktopScreenQt::~DesktopScreenQt()
{
    for (auto conn : std::as_const(m_connections))
        QObject::disconnect(conn);
}

void DesktopScreenQt::initializeScreens()
{
    if (updateAllScreens()) {
        m_connections[0] =
            QObject::connect(qApp, &QGuiApplication::primaryScreenChanged, [this] (QScreen *screen) {
                ProcessDisplayChanged(toDisplayDisplay(0, screen), true /* is_primary */);
            });
        // no guarantees how these will affect ids:
        m_connections[1] =
            QObject::connect(qApp, &QGuiApplication::screenAdded, [this] (QScreen *) {
                updateAllScreens();
            });
        m_connections[2] =
            QObject::connect(qApp, &QGuiApplication::screenRemoved, [this] (QScreen *) {
                updateAllScreens();
            });
    } else {
        // Running headless
        ProcessDisplayChanged(display::Display::GetDefaultDisplay(), true /* is_primary */);
        m_connections[0] =
            QObject::connect(qApp, &QGuiApplication::screenAdded, [this] (QScreen *) {
                display_list().RemoveDisplay(display::kDefaultDisplayId);
                QObject::disconnect(m_connections[0]);
                initializeScreens();
            });
    }
}

bool DesktopScreenQt::updateAllScreens()
{
    Q_ASSERT(qApp->primaryScreen() == qApp->screens().first());
    const auto screens = qApp->screens();
    const int oldLen = GetNumDisplays();
    for (int i = screens.length(); i < oldLen; ++i)
        display_list().RemoveDisplay(i);
    for (int i = 0; i < screens.length(); ++i)
        ProcessDisplayChanged(toDisplayDisplay(i, screens.at(i)), i == 0 /* is_primary */);

    return screens.length() > 0;
}

display::Display DesktopScreenQt::GetDisplayNearestWindow(gfx::NativeWindow /*window*/) const
{
    return GetPrimaryDisplay();
}

#if defined(USE_XSCREENSAVER)
class XScreenSuspender : public display::Screen::ScreenSaverSuspender
{
public:
    XScreenSuspender()
    {
        ui::SuspendX11ScreenSaver(true);
    }
    ~XScreenSuspender() override
    {
        ui::SuspendX11ScreenSaver(false);
    }
};
#endif
#if BUILDFLAG(IS_CHROMEOS_LACROS) || BUILDFLAG(IS_LINUX)
std::unique_ptr<display::Screen::ScreenSaverSuspender> DesktopScreenQt::SuspendScreenSaver()
{
#if defined(USE_XSCREENSAVER)
    return std::make_unique<XScreenSuspender>();
#else
    return nullptr;
#endif
}
#endif

bool DesktopScreenQt::IsScreenSaverActive() const
{
#if defined(USE_XSCREENSAVER)
    return ui::IsXScreensaverActive();
#else
    return false;
#endif
}

} // namespace QtWebEngineCore
