// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "native_skia_output_device_metal.h"

#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgtexture.h>

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
void releaseMetalTexture(void *texture);

QSGTexture *NativeSkiaOutputDeviceMetal::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer || !m_readyWithTexture)
        return nullptr;

    gfx::ScopedIOSurface ioSurface = m_frontBuffer->ioSurface();
    if (!ioSurface) {
        qWarning("No IOSurface.");
        return nullptr;
    }

    // This is a workaround to not to release metal texture too early.
    // In RHI, QMetalTexture wraps MTLTexture. QMetalTexture seems to be only destructed after the
    // next MTLTexture is imported. The "old" MTLTexture can be still pontentially used by RHI
    // while QMetalTexture is not destructed. Metal Validation Layer also warns about it.
    // Delay releasing MTLTexture after the next one is presented.
    if (m_currentMetalTexture) {
        m_frontBuffer->textureCleanupCallback = [texture = m_currentMetalTexture]() {
            releaseMetalTexture(texture);
        };
        m_currentMetalTexture = nullptr;
    }

    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    QSGTexture *qsgTexture = makeMetalTexture(win, ioSurface.get(), /* plane */ 0, size(), texOpts);

    auto ni = qsgTexture->nativeInterface<QNativeInterface::QSGMetalTexture>();
    m_currentMetalTexture = ni->nativeTexture();

    return qsgTexture;
}

} // namespace QtWebEngineCore
