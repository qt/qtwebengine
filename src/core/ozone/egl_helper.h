// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef EGL_HELPER_H
#define EGL_HELPER_H

#include <QtCore/qscopedpointer.h>

#include "ui/gfx/buffer_types.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_pixmap_handle.h"

#include <map>
#include <string>
#include <vector>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#undef eglGetError
#undef eglQueryString

#undef eglQueryDevices
#undef eglQueryDeviceString
#undef eglQueryDisplayAttrib

#undef eglQueryDmaBufModifiers

#undef eglChooseConfig
#undef eglCreateContext
#undef eglCreateImage
#undef eglCreatePbufferSurface
#undef eglDestroyContext
#undef eglDestroyImage
#undef eglDestroySurface
#undef eglGetCurrentContext
#undef eglGetCurrentDisplay
#undef eglGetCurrentSurface
#undef eglGetProcAddress
#undef eglInitialize
#undef eglMakeCurrent

#undef eglExportDMABUFImageMESA
#undef eglExportDMABUFImageQueryMESA

QT_BEGIN_NAMESPACE

class GbmBufferFactory;

class EGLHelper
{
public:
    struct EGLFunctions
    {
        EGLFunctions();

        PFNEGLGETERRORPROC eglGetError;
        PFNEGLQUERYSTRINGPROC eglQueryString;

        // EGL_EXT_device_enumeration:
        PFNEGLQUERYDEVICESEXTPROC eglQueryDevices;

        // EGL_EXT_device_query:
        PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceString;
        PFNEGLQUERYDISPLAYATTRIBEXTPROC eglQueryDisplayAttrib;

        // EGL_EXT_image_dma_buf_import_modifiers:
        PFNEGLQUERYDMABUFMODIFIERSEXTPROC eglQueryDmaBufModifiers;

        // EGL-based allocation:
        PFNEGLCHOOSECONFIGPROC eglChooseConfig;
        PFNEGLCREATECONTEXTPROC eglCreateContext;
        PFNEGLCREATEIMAGEPROC eglCreateImage;
        PFNEGLCREATEPBUFFERSURFACEPROC eglCreatePbufferSurface;
        PFNEGLDESTROYCONTEXTPROC eglDestroyContext;
        PFNEGLDESTROYIMAGEPROC eglDestroyImage;
        PFNEGLDESTROYSURFACEPROC eglDestroySurface;
        PFNEGLGETCURRENTCONTEXTPROC eglGetCurrentContext;
        PFNEGLGETCURRENTDISPLAYPROC eglGetCurrentDisplay;
        PFNEGLGETCURRENTSURFACEPROC eglGetCurrentSurface;
        PFNEGLGETPROCADDRESSPROC eglGetProcAddress;
        PFNEGLINITIALIZEPROC eglInitialize;
        PFNEGLMAKECURRENTPROC eglMakeCurrent;

        // EGL_MESA_image_dma_buf_export:
        PFNEGLEXPORTDMABUFIMAGEMESAPROC eglExportDMABUFImageMESA;
        PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC eglExportDMABUFImageQueryMESA;
    };

    static EGLHelper *instance();
    ~EGLHelper();

    EGLDisplay getEGLDisplay() const { return m_eglDisplay; }
    EGLFunctions *functions() const { return m_functions.get(); }
    bool isDmaBufSupported() const { return m_isDmaBufSupported; }
    GbmBufferFactory *gbmFactory() const { return m_gbmBufferFactory.get(); }
    bool canCreateNativePixmapForFormat(gfx::BufferFormat format) const;

    const std::vector<uint64_t> &getSupportedModifiers(gfx::BufferFormat format) const;
    gfx::NativePixmapHandle exportHandleFromEGLImage(const gfx::Size &size);
    gfx::NativePixmapHandle exportHandleFromEGLImage(gfx::BufferFormat format,
                                                     const gfx::Size &size,
                                                     gfx::NativePixmapHandle handle);

    const char *getLastEGLErrorString() const;

private:
    EGLHelper();
    std::string getDrmRenderNodeFilePath(const char *extensions) const;

    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    QScopedPointer<EGLFunctions> m_functions;
    bool m_isDmaBufSupported = false;

    bool m_isImageDmaBufExportSupported = false;
    QScopedPointer<GbmBufferFactory> m_gbmBufferFactory;
    mutable std::map<gfx::BufferFormat, std::vector<uint64_t>> m_supportedModifiers;
};

QT_END_NAMESPACE

#endif // EGL_HELPER_H
