// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef WGL_HELPER_H
#define WGL_HELPER_H

#include <QtCore/qscopedpointer.h>

#include <d3d11_1.h>
#include <wrl.h>
#include <GL/gl.h>

#undef wglDXOpenDeviceNV
#undef wglDXCloseDeviceNV
#undef wglDXRegisterObjectNV
#undef wglDXUnregisterObjectNV
#undef wglDXLockObjectsNV
#undef wglDXUnlockObjectsNV

QT_BEGIN_NAMESPACE

class QOpenGLContext;
class QSurface;

class WGLHelper
{
public:
    struct WGLFunctions
    {
        WGLFunctions();

#if !defined(WGL_NV_DX_interop)
        typedef BOOL(WINAPI * PFNWGLDXSETRESOURCESHAREHANDLENVPROC)(void *dxObject, HANDLE shareHandle);
        typedef HANDLE(WINAPI * PFNWGLDXOPENDEVICENVPROC)(void *dxDevice);
        typedef BOOL(WINAPI * PFNWGLDXCLOSEDEVICENVPROC)(HANDLE hDevice);
        typedef HANDLE(WINAPI * PFNWGLDXREGISTEROBJECTNVPROC)(HANDLE hDevice, void *dxObject,
                                                              GLuint name, GLenum type, GLenum access);
        typedef BOOL(WINAPI * PFNWGLDXUNREGISTEROBJECTNVPROC)(HANDLE hDevice, HANDLE hObject);
        typedef BOOL(WINAPI * PFNWGLDXOBJECTACCESSNVPROC)(HANDLE hObject, GLenum access);
        typedef BOOL(WINAPI * PFNWGLDXLOCKOBJECTSNVPROC)(HANDLE hDevice, GLint count,
                                                         HANDLE * hObjects);
        typedef BOOL(WINAPI * PFNWGLDXUNLOCKOBJECTSNVPROC)(HANDLE hDevice, GLint count,
                                                           HANDLE * hObjects);
#endif

        PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV;
        PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV;
        PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV;
        PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV;
        PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV;
        PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV;
    };

    static WGLHelper *instance();

    ~WGLHelper();

    WGLFunctions *functions() const { return m_functions.get(); }
    ID3D11Device *device() const { return m_device.Get(); }
    ID3D11DeviceContext *immediateContext() const { return m_immediateContext.Get(); }
    HANDLE interopDevice() const { return m_interopDevice; }

private:
    WGLHelper();

    QScopedPointer<WGLFunctions> m_functions;

    Microsoft::WRL::ComPtr<IDXGIFactory> m_factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter> m_adapter;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_immediateContext;

    HANDLE m_interopDevice = INVALID_HANDLE_VALUE;
};

class D3DSharedTexture
{
public:
    D3DSharedTexture(WGLHelper::WGLFunctions *wglFun, ID3D11Device *device,
                     ID3D11DeviceContext *immediateContext, HANDLE interopDevice,
                     HANDLE dxgiSharedHandle);
    ~D3DSharedTexture();

    void lockObject();
    void unlockObject();
    GLuint glTexture() const { return m_glTexture; }

private:
    WGLHelper::WGLFunctions *m_wglFun;
    HANDLE m_interopDevice;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_d3dTexture;

    QOpenGLContext *m_createContext = nullptr;
    QSurface *m_createSurface = nullptr;
    GLuint m_glTexture = 0;
    HANDLE m_glTextureHandle = INVALID_HANDLE_VALUE;

    bool m_isLocked = false;
};

QT_END_NAMESPACE

#endif // WGL_HELPER_H
