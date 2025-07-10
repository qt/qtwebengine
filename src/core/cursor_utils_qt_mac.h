// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CURSOR_ULTIS_QT_MAC_H
#define CURSOR_ULTIS_QT_MAC_H

#include <QtCore/qglobal.h>
#include <QtGui/QImage>
#include "ui/base/cocoa/cursor_utils.h"

namespace QtWebEngineCore {

struct ImageInfo
{
    QImage image;
    QPoint hotSpotData;
};

ImageInfo QImageFromNSCursor(const ui::Cursor &cursor);

} // namespace QtWebEngineCore

#endif // CURSOR_ULTIS_QT_MAC_H
