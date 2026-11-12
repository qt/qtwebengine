// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CURSOR_UTILS_QT_MAC_H
#define CURSOR_UTILS_QT_MAC_H

#include <QtCore/qglobal.h>
#include <QtCore/QAbstractNativeEventFilter>
#include <QtCore/QPointF>
#include <QtGui/QImage>
#include <functional>
#include "ui/base/cocoa/cursor_utils.h"

namespace QtWebEngineCore {

struct ImageInfo
{
    QImage image;
    QPoint hotSpotData;
};

ImageInfo QImageFromNSCursor(const ui::Cursor &cursor);

void setCursorLocked(bool locked);

// While the cursor is frozen by setCursorLocked(true) its position no longer changes, so Qt
// drops every mouse move. This filter reads the relative movement straight from the native
// NSEvents and passes it to the callback.
class PointerLockEventFilter : public QAbstractNativeEventFilter
{
public:
    explicit PointerLockEventFilter(std::function<void(const QPointF &)> onMove);
    ~PointerLockEventFilter() override;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

private:
    std::function<void(const QPointF &)> m_onMove;
};

} // namespace QtWebEngineCore

#endif // CURSOR_UTILS_QT_MAC_H
