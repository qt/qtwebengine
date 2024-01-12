// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef NATIVE_SKIA_OUTPUT_DEVICE_METAL_H
#define NATIVE_SKIA_OUTPUT_DEVICE_METAL_H

#include "native_skia_output_device.h"

namespace QtWebEngineCore {

class NativeSkiaOutputDeviceMetal final : public NativeSkiaOutputDevice
{
public:
    NativeSkiaOutputDeviceMetal(
            scoped_refptr<gpu::SharedContextState> contextState, bool requiresAlpha,
            gpu::MemoryTracker *memoryTracker, viz::SkiaOutputSurfaceDependency *dependency,
            gpu::SharedImageFactory *shared_image_factory,
            gpu::SharedImageRepresentationFactory *shared_image_representation_factory,
            DidSwapBufferCompleteCallback didSwapBufferCompleteCallback);
    ~NativeSkiaOutputDeviceMetal() override;

    // Overridden from Compositor:
    QSGTexture *texture(QQuickWindow *win, uint32_t textureOptions) override;
};

} // namespace QtWebEngineCore

#endif // NATIVE_SKIA_OUTPUT_DEVICE_METAL_H
