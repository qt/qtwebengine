// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#import <AppKit/AppKit.h>
#import <IOSurface/IOSurface.h>
#import <Metal/Metal.h>

#include <QtGui/qtguiglobal.h>

QT_BEGIN_NAMESPACE
class QSGTexture;
class QQuickWindow;
QT_END_NAMESPACE

@class MTLDevice;

namespace QtWebEngineCore {
QSGTexture *makeMetalTexture2(QQuickWindow *win, id<MTLTexture> mtlTexture, int width, int height, uint32_t textureOptions);
MTLDevice *getRhiDev(QQuickWindow *win);

QSGTexture *makeMetalTexture(QQuickWindow *win, IOSurfaceRef io_surface, uint io_surface_plane, int width, int height, uint32_t textureOptions)
{
    auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                 width:width height:height mipmapped:false];

    auto device = getRhiDev(win);
    auto texture = [device newTextureWithDescriptor:desc iosurface:io_surface plane:io_surface_plane];
    return makeMetalTexture2(win, texture, width, height, textureOptions);
}

#if QT_CONFIG(opengl)
CGLContextObj getCGLContext(NSOpenGLContext *context)
{
    return [context CGLContextObj];
}
#endif

} // namespace
