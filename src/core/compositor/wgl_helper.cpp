// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "wgl_helper.h"

#include <QtCore/private/qsystemerror_p.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglfunctions.h>

#if !defined(WGL_NV_DX_interop)
#define WGL_ACCESS_READ_ONLY_NV 0x00000000
#define WGL_ACCESS_READ_WRITE_NV 0x00000001
#define WGL_ACCESS_WRITE_DISCARD_NV 0x00000002
#endif

QT_BEGIN_NAMESPACE

WGLHelper::WGLFunctions::WGLFunctions()
{
    auto *glContext = QOpenGLContext::currentContext();
    Q_ASSERT(glContext);

    wglDXOpenDeviceNV = reinterpret_cast<PFNWGLDXOPENDEVICENVPROC>(
            glContext->getProcAddress("wglDXOpenDeviceNV"));
    wglDXCloseDeviceNV = reinterpret_cast<PFNWGLDXCLOSEDEVICENVPROC>(
            glContext->getProcAddress("wglDXCloseDeviceNV"));
    wglDXRegisterObjectNV = reinterpret_cast<PFNWGLDXREGISTEROBJECTNVPROC>(
            glContext->getProcAddress("wglDXRegisterObjectNV"));
    wglDXUnregisterObjectNV = reinterpret_cast<PFNWGLDXUNREGISTEROBJECTNVPROC>(
            glContext->getProcAddress("wglDXUnregisterObjectNV"));
    wglDXLockObjectsNV = reinterpret_cast<PFNWGLDXLOCKOBJECTSNVPROC>(
            glContext->getProcAddress("wglDXLockObjectsNV"));
    wglDXUnlockObjectsNV = reinterpret_cast<PFNWGLDXUNLOCKOBJECTSNVPROC>(
            glContext->getProcAddress("wglDXUnlockObjectsNV"));
}

WGLHelper *WGLHelper::instance()
{
    static WGLHelper wglHelper;
    return &wglHelper;
}

WGLHelper::WGLHelper() : m_functions(new WGLHelper::WGLFunctions())
{
    HRESULT hr;

    hr = CreateDXGIFactory(IID_PPV_ARGS(&m_factory));
    if (FAILED(hr)) {
        qFatal("WGL: Failed to create DXGI Factory: %ls",
               qUtf16Printable(QSystemError::windowsComString(hr)));
    }

    hr = m_factory->EnumAdapters(0, &m_adapter);
    if (FAILED(hr)) {
        qFatal("WGL: Failed to enumerate adapters: %ls",
               qUtf16Printable(QSystemError::windowsComString(hr)));
    }

    uint devFlags = 0;
#if !defined(QT_NO_DEBUG)
    devFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
    hr = D3D11CreateDevice(m_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, /*Software=*/nullptr, devFlags,
                           featureLevels, std::size(featureLevels), D3D11_SDK_VERSION, &m_device,
                           /*pFeatureLevel=*/nullptr, &m_immediateContext);
    if (FAILED(hr)) {
        qFatal("WGL: Failed to create D3D11 device: %ls",
               qUtf16Printable(QSystemError::windowsComString(hr)));
    }

    m_interopDevice = m_functions->wglDXOpenDeviceNV(m_device.Get());
    if (m_interopDevice == INVALID_HANDLE_VALUE) {
        qWarning("WGL: Failed to open interop device: %ls",
                 qUtf16Printable(QSystemError::windowsString()));
    }
}

WGLHelper::~WGLHelper()
{
    if (m_interopDevice != INVALID_HANDLE_VALUE)
        m_functions->wglDXCloseDeviceNV(m_interopDevice);
}

D3DSharedTexture::D3DSharedTexture(WGLHelper::WGLFunctions *wglFun, ID3D11Device *device,
                                   ID3D11DeviceContext *immediateContext, HANDLE interopDevice,
                                   HANDLE dxgiSharedHandle)
    : m_wglFun(wglFun), m_interopDevice(interopDevice)
{
    HRESULT hr;

    Microsoft::WRL::ComPtr<ID3D11Device1> device1;
    hr = device->QueryInterface(IID_PPV_ARGS(&device1));
    Q_ASSERT(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<ID3D11Texture2D> srcTexture;
    hr = device1->OpenSharedResource1(dxgiSharedHandle, IID_PPV_ARGS(&srcTexture));
    if (FAILED(hr)) {
        qWarning("WGL: Failed to share D3D11 texture (%ls). This will result in failed rendering. "
                 "Report the bug, and try restarting with QTWEBENGINE_CHROMIUM_FLAGS=--disble-gpu",
                 qUtf16Printable(QSystemError::windowsComString(hr)));
        return;
    }
    Q_ASSERT(srcTexture);

    D3D11_TEXTURE2D_DESC srcDesc;
    srcTexture->GetDesc(&srcDesc);

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = srcDesc.Width;
    textureDesc.Height = srcDesc.Height;
    textureDesc.MipLevels = srcDesc.MipLevels;
    textureDesc.ArraySize = srcDesc.ArraySize;
    textureDesc.Format = srcDesc.Format;
    textureDesc.SampleDesc = srcDesc.SampleDesc;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;

    device->CreateTexture2D(&textureDesc, nullptr, &m_d3dTexture);

    // Copy texture from GPU thread to UI thread.
    // This is a workaround because Intel driver doesn't seem to support interop
    // for an already shared texture.
    immediateContext->CopyResource(m_d3dTexture.Get(), srcTexture.Get());

    auto *glContext = QOpenGLContext::currentContext();
    Q_ASSERT(glContext);
    auto *glFun = glContext->functions();

    glFun->glGenTextures(1, &m_glTexture);

    // Bind the DXGI texture to a GL texture.
    m_glTextureHandle = m_wglFun->wglDXRegisterObjectNV(
            interopDevice, m_d3dTexture.Get(), m_glTexture, GL_TEXTURE_2D, WGL_ACCESS_READ_ONLY_NV);
    Q_ASSERT(glFun->glGetError() == GL_NO_ERROR);
    Q_ASSERT(m_glTextureHandle != INVALID_HANDLE_VALUE);
}

D3DSharedTexture::~D3DSharedTexture()
{
    if (m_glTextureHandle != INVALID_HANDLE_VALUE) {
        if (m_isLocked) {
            qWarning("WGL: Shared texture is still locked during destruction!");
            unlockObject();
        }

        m_wglFun->wglDXUnregisterObjectNV(m_interopDevice, m_glTextureHandle);
    }

    auto *glContext = QOpenGLContext::currentContext();
    if (m_glTexture && glContext) {
        auto *glFun = glContext->functions();
        glFun->glDeleteTextures(1, &m_glTexture);
    }
}

void D3DSharedTexture::lockObject()
{
    if (m_isLocked) {
        qWarning("WGL: Shared texture is already locked!");
        return;
    }

    bool status = m_wglFun->wglDXLockObjectsNV(m_interopDevice, 1, &m_glTextureHandle);
    if (!status) {
        qWarning("WGL: Failed to lock shared texture: %ls",
                 qUtf16Printable(QSystemError::windowsString()));
        return;
    }

    m_isLocked = true;
}

void D3DSharedTexture::unlockObject()
{
    if (!m_isLocked) {
        qWarning("WGL: Shared texture is already unlocked!");
        return;
    }

    bool status = m_wglFun->wglDXUnlockObjectsNV(m_interopDevice, 1, &m_glTextureHandle);
    if (!status) {
        qWarning("WGL: Failed to unlock shared texture: %ls",
                 qUtf16Printable(QSystemError::windowsString()));
        return;
    }

    m_isLocked = false;
}

QT_END_NAMESPACE
