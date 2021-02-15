/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#include "desktop_screen_qt.h"

#include "ui/display/display.h"

#include "type_conversion.h"

#include <QGuiApplication>
#include <QScreen>

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
    display.set_device_scale_factor(screen->devicePixelRatio());
    display.set_is_monochrome(screen->depth() == 1);
    display.set_color_depth(screen->depth());
    display.set_depth_per_component(8); // FIXME: find the real value
    display.set_display_frequency(std::ceil(screen->refreshRate()));
    display.set_rotation(toDisplayRotation(screen->orientation()));
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
    for (auto conn : qAsConst(m_connections))
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

} // namespace QtWebEngineCore
