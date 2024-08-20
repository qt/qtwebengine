// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "native_skia_output_device_direct3d11.h"

#include <QtCore/private/qsystemerror_p.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgtexture.h>

#include <d3d11_1.h>

namespace QtWebEngineCore {

NativeSkiaOutputDeviceDirect3D11::NativeSkiaOutputDeviceDirect3D11(
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

NativeSkiaOutputDeviceDirect3D11::~NativeSkiaOutputDeviceDirect3D11() { }

QSGTexture *NativeSkiaOutputDeviceDirect3D11::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer || !m_readyWithTexture)
        return nullptr;

    absl::optional<gl::DCLayerOverlayImage> overlayImage = m_frontBuffer->overlayImage();
    if (!overlayImage) {
        qWarning("No overlay image.");
        return nullptr;
    }

    QSGRendererInterface *ri = win->rendererInterface();

    HRESULT status = S_OK;
    HANDLE sharedHandle = nullptr;
    IDXGIResource1 *resource = nullptr;
    if (!overlayImage->nv12_texture()) {
        qWarning("No D3D texture.");
        return nullptr;
    }
    status = overlayImage->nv12_texture()->QueryInterface(__uuidof(IDXGIResource1),
                                                          (void **)&resource);
    Q_ASSERT(status == S_OK);
    status = resource->CreateSharedHandle(NULL, DXGI_SHARED_RESOURCE_READ, NULL, &sharedHandle);
    Q_ASSERT(status == S_OK);
    Q_ASSERT(sharedHandle);

    // Pass texture between two D3D devices:
    ID3D11Device1 *device = static_cast<ID3D11Device1 *>(
            ri->getResource(win, QSGRendererInterface::DeviceResource));

    ID3D11Texture2D *qtTexture;
    status = device->OpenSharedResource1(sharedHandle, __uuidof(ID3D11Texture2D),
                                         (void **)&qtTexture);
    if (status != S_OK) {
        qWarning("Failed to share D3D11 texture (%s). This will result in failed rendering. Report "
                 "the bug, and try restarting with QTWEBENGINE_CHROMIUM_FLAGS=--disble-gpu",
                 qPrintable(QSystemError::windowsComString(status)));
        ::CloseHandle(sharedHandle);
        return nullptr;
    }

    Q_ASSERT(qtTexture);
    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    QSGTexture *texture =
            QNativeInterface::QSGD3D11Texture::fromNative(qtTexture, win, size(), texOpts);

    m_frontBuffer->textureCleanupCallback = [qtTexture, sharedHandle]() {
        qtTexture->Release();
        ::CloseHandle(sharedHandle);
    };

    return texture;
}

} // namespace QtWebEngineCore
