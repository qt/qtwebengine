// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <qquickwindow.h>
#include <qsgrendererinterface.h>
#include <qsgtexture.h>

@class MTLDevice;
@protocol MTLTexture;

namespace QtWebEngineCore {

MTLDevice *getRhiDev(QQuickWindow *win)
{
    QSGRendererInterface *ri = win->rendererInterface();
    return static_cast<MTLDevice *>(ri->getResource(win, QSGRendererInterface::DeviceResource));
}

QSGTexture *makeMetalTexture2(QQuickWindow *win, id<MTLTexture> mtlTexture, int width, int height, uint32_t textureOptions)
{
    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    return QNativeInterface::QSGMetalTexture::fromNative(mtlTexture, win, {width, height}, texOpts);
}

} // namespace
