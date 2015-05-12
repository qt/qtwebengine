/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "desktop_screen_qt.h"

namespace QtWebEngineCore {

gfx::Point DesktopScreenQt::GetCursorScreenPoint()
{
    Q_UNREACHABLE();
    return gfx::Point();
}

gfx::NativeWindow DesktopScreenQt::GetWindowUnderCursor()
{
    Q_UNREACHABLE();
    return gfx::NativeWindow();
}

gfx::NativeWindow DesktopScreenQt::GetWindowAtScreenPoint(const gfx::Point& point)
{
    Q_UNREACHABLE();
    return gfx::NativeWindow();
}

int DesktopScreenQt::GetNumDisplays() const
{
    Q_UNREACHABLE();
    return 0;
}

std::vector<gfx::Display> DesktopScreenQt::GetAllDisplays() const
{
    Q_UNREACHABLE();
    return std::vector<gfx::Display>();
}

gfx::Display DesktopScreenQt::GetDisplayNearestWindow(gfx::NativeView window) const
{
    // RenderViewHostImpl::OnStartDragging uses this to determine
    // the scale factor for the view.
    return gfx::Display();
}

gfx::Display DesktopScreenQt::GetDisplayNearestPoint(const gfx::Point& point) const
{
    Q_UNREACHABLE();
    return gfx::Display();
}

gfx::Display DesktopScreenQt::GetDisplayMatching(const gfx::Rect& match_rect) const
{
    Q_UNREACHABLE();
    return gfx::Display();
}

gfx::Display DesktopScreenQt::GetPrimaryDisplay() const
{
    return gfx::Display();
}

void DesktopScreenQt::AddObserver(gfx::DisplayObserver* observer)
{
    Q_UNREACHABLE();
}

void DesktopScreenQt::RemoveObserver(gfx::DisplayObserver* observer)
{
    Q_UNREACHABLE();
}

} // namespace QtWebEngineCore
