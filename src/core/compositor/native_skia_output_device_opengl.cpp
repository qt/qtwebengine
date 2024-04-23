// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "native_skia_output_device_opengl.h"

#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglextrafunctions.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgtexture.h>

namespace QtWebEngineCore {

NativeSkiaOutputDeviceOpenGL::NativeSkiaOutputDeviceOpenGL(
        scoped_refptr<gpu::SharedContextState> contextState, bool requiresAlpha,
        gpu::MemoryTracker *memoryTracker, viz::SkiaOutputSurfaceDependency *dependency,
        gpu::SharedImageFactory *shared_image_factory,
        gpu::SharedImageRepresentationFactory *shared_image_representation_factory,
        DidSwapBufferCompleteCallback didSwapBufferCompleteCallback)
    : NativeSkiaOutputDevice(contextState, requiresAlpha, memoryTracker, dependency,
                             shared_image_factory, shared_image_representation_factory,
                             didSwapBufferCompleteCallback)
{
    SkColorType skColorType = kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBA_8888)] = skColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBX_8888)] = skColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRA_8888)] = skColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRX_8888)] = skColorType;
}

NativeSkiaOutputDeviceOpenGL::~NativeSkiaOutputDeviceOpenGL() { }

#if defined(Q_OS_MACOS)
uint32_t makeCGLTexture(QQuickWindow *win, IOSurfaceRef ioSurface, const QSize &size);
#endif

QSGTexture *NativeSkiaOutputDeviceOpenGL::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer || !m_readyWithTexture)
        return nullptr;

#if defined(USE_OZONE)
    scoped_refptr<gfx::NativePixmap> nativePixmap = m_frontBuffer->nativePixmap();
    if (!nativePixmap) {
        qWarning("No native pixmap.");
        return nullptr;
    }
#elif defined(Q_OS_WIN)
    auto overlayImage = m_frontBuffer->overlayImage();
    if (!overlayImage) {
        qWarning("No overlay image.");
        return nullptr;
    }
#elif defined(Q_OS_MACOS)
    gfx::ScopedIOSurface ioSurface = m_frontBuffer->ioSurface();
    if (!ioSurface) {
        qWarning("No IOSurface.");
        return nullptr;
    }
#endif

    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    QSGTexture *texture = nullptr;

#if defined(USE_OZONE)
    // TODO(QTBUG-112281): Add ANGLE support to Linux.
    QT_NOT_YET_IMPLEMENTED
#elif defined(Q_OS_WIN)
    // TODO: Add WGL support over ANGLE.
    QT_NOT_YET_IMPLEMENTED
#elif defined(Q_OS_MACOS)
    uint32_t glTexture = makeCGLTexture(win, ioSurface.get(), size());
    texture = QNativeInterface::QSGOpenGLTexture::fromNative(glTexture, win, size(), texOpts);

    m_frontBuffer->textureCleanupCallback = [glTexture]() {
        auto *glContext = QOpenGLContext::currentContext();
        if (!glContext)
            return;
        auto glFun = glContext->functions();
        glFun->glDeleteTextures(1, &glTexture);
    };
#endif

    return texture;
}

} // namespace QtWebEngineCore
