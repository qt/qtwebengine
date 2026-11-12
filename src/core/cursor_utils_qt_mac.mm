// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#undef Q_FORWARD_DECLARE_OBJC_CLASS

#include "cursor_utils_qt_mac.h"
#include "ui/base/cocoa/cursor_utils.h"
#include <QImage>
#include <QtCore/QCoreApplication>
#include <AppKit/AppKit.h>
#include <QtGui/private/qcoregraphics_p.h>

namespace QtWebEngineCore {

CGImageRef CGImageFromNSImage(NSImage *image)
{
    NSSize size = image.size;
    CGRect rect = CGRectMake(0, 0, size.width, size.height);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(NULL, size.width, size.height, 8, 0, colorSpace,
                                             kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);

    NSGraphicsContext *gc = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:NO];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:gc];
    [image drawInRect:rect fromRect:NSZeroRect operation:NSCompositingOperationCopy fraction:1.0];
    [NSGraphicsContext restoreGraphicsState];

    CGImageRef cgImage = CGBitmapContextCreateImage(ctx);
    CGContextRelease(ctx);
    return cgImage;
}

ImageInfo QImageFromNSCursor(const ui::Cursor &cursor)
{
    ImageInfo imageInfo;
    NSCursor *nativeCursor = ui::GetNativeCursor(cursor);
    NSImage *image = [nativeCursor image];
    NSPoint hotSpot = [nativeCursor hotSpot];
    imageInfo.hotSpotData.setX(hotSpot.x);
    imageInfo.hotSpotData.setX(hotSpot.y);

    CGImageRef cgImageRef = CGImageFromNSImage(image);
    imageInfo.image = qt_mac_toQImage(cgImageRef);
    CGImageRelease(cgImageRef);
    return imageInfo;
}

void setCursorLocked(bool locked)
{
    CGAssociateMouseAndMouseCursorPosition(!locked);
    if (locked)
        [NSCursor hide];
    else
        [NSCursor unhide];
}

PointerLockEventFilter::PointerLockEventFilter(std::function<void(const QPointF &)> onMove)
    : m_onMove(std::move(onMove))
{
    QCoreApplication::instance()->installNativeEventFilter(this);
}

PointerLockEventFilter::~PointerLockEventFilter()
{
    if (QCoreApplication *app = QCoreApplication::instance())
        app->removeNativeEventFilter(this);
}

bool PointerLockEventFilter::nativeEventFilter(const QByteArray &eventType, void *message,
                                               qintptr *)
{
    if (eventType != "mac_generic_NSEvent")
        return false;

    NSEvent *event = static_cast<NSEvent *>(message);
    switch (event.type) {
    case NSEventTypeMouseMoved:
    case NSEventTypeLeftMouseDragged:
    case NSEventTypeRightMouseDragged:
    case NSEventTypeOtherMouseDragged:
        if (event.deltaX != 0 || event.deltaY != 0)
            m_onMove(QPointF(event.deltaX, event.deltaY));
        break;
    default:
        break;
    }
    return false;
}

} // namespace QtWebEngineCore
