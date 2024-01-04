// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "native_skia_output_device_metal.h"

#include <QtQuick/qquickwindow.h>

namespace QtWebEngineCore {

NativeSkiaOutputDeviceMetal::NativeSkiaOutputDeviceMetal(
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

NativeSkiaOutputDeviceMetal::~NativeSkiaOutputDeviceMetal() { }

QSGTexture *makeMetalTexture(QQuickWindow *win, IOSurfaceRef ioSurface, uint ioSurfacePlane,
                             const QSize &size, QQuickWindow::CreateTextureOptions texOpts);

QSGTexture *NativeSkiaOutputDeviceMetal::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer || !m_readyWithTexture)
        return nullptr;

    gfx::ScopedIOSurface ioSurface = m_frontBuffer->ioSurface();
    if (!ioSurface) {
        qWarning("No IOSurface.");
        return nullptr;
    }

    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    return makeMetalTexture(win, ioSurface.release(), /* plane */ 0, size(), texOpts);
}

} // namespace QtWebEngineCore
