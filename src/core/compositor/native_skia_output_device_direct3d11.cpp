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
        qWarning("D3D: No overlay image.");
        return nullptr;
    }

    Q_ASSERT(overlayImage->type() == gl::DCLayerOverlayType::kNV12Texture);
    Microsoft::WRL::ComPtr<ID3D11Texture2D> chromeTexture = overlayImage->nv12_texture();
    if (!chromeTexture) {
        qWarning("D3D: No D3D texture.");
        return nullptr;
    }

    HRESULT hr;

    Microsoft::WRL::ComPtr<IDXGIResource1> dxgiResource;
    hr = chromeTexture->QueryInterface(IID_PPV_ARGS(&dxgiResource));
    Q_ASSERT(SUCCEEDED(hr));

    HANDLE sharedHandle = INVALID_HANDLE_VALUE;
    hr = dxgiResource->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ, nullptr,
                                          &sharedHandle);
    Q_ASSERT(SUCCEEDED(hr));
    Q_ASSERT(sharedHandle != INVALID_HANDLE_VALUE);

    // Pass texture between two D3D devices:
    QSGRendererInterface *ri = win->rendererInterface();
    ID3D11Device *device = static_cast<ID3D11Device *>(
            ri->getResource(win, QSGRendererInterface::DeviceResource));
    Q_ASSERT(device);

    Microsoft::WRL::ComPtr<ID3D11Device1> device1;
    hr = device->QueryInterface(IID_PPV_ARGS(&device1));
    Q_ASSERT(SUCCEEDED(hr));

    ID3D11Texture2D *qtTexture = nullptr;
    hr = device1->OpenSharedResource1(sharedHandle, IID_PPV_ARGS(&qtTexture));
    if (FAILED(hr)) {
        qWarning("D3D: Failed to share D3D11 texture (%s). This will result in failed rendering. "
                 "Report the bug, and try restarting with QTWEBENGINE_CHROMIUM_FLAGS=--disble-gpu",
                 qPrintable(QSystemError::windowsComString(hr)));
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
