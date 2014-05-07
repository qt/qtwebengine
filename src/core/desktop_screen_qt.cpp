/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "desktop_screen_qt.h"

bool DesktopScreenQt::IsDIPEnabled()
{
    // Currently only used by GetScaleFactorForNativeView for drag events.
    // Short-circuit this until we can test any implementation properly in real code.
    return false;
}

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
    Q_UNREACHABLE();
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
    Q_UNREACHABLE();
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
