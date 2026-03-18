// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef EGL_HELPER_H
#define EGL_HELPER_H

#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>

#include "ui/gfx/buffer_types.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_pixmap_handle.h"

#include <string>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#undef eglCreateImage
#undef eglDestroyImage
#undef eglQueryDevices
#undef eglQueryDeviceString
#undef eglQueryDisplayAttrib
#undef eglQueryString

#undef eglGetCurrentContext
#undef eglGetCurrentDisplay
#undef eglGetCurrentSurface
#undef eglGetError
#undef eglMakeCurrent

#undef eglExportDMABUFImageMESA
#undef eglExportDMABUFImageQueryMESA

QT_BEGIN_NAMESPACE

class GbmBufferFactory;
class QOffscreenSurface;

class EGLHelper
{
public:
    struct EGLFunctions
    {
        EGLFunctions();

        PFNEGLCREATEIMAGEPROC eglCreateImage;
        PFNEGLDESTROYIMAGEPROC eglDestroyImage;
        PFNEGLQUERYDEVICESEXTPROC eglQueryDevices;
        PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceString;
        PFNEGLQUERYDISPLAYATTRIBEXTPROC eglQueryDisplayAttrib;
        PFNEGLQUERYSTRINGPROC eglQueryString;

        // Used for EGL-based allocation:
        PFNEGLGETCURRENTCONTEXTPROC eglGetCurrentContext;
        PFNEGLGETCURRENTDISPLAYPROC eglGetCurrentDisplay;
        PFNEGLGETCURRENTSURFACEPROC eglGetCurrentSurface;
        PFNEGLGETERRORPROC eglGetError;
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

    gfx::NativePixmapHandle exportHandleFromEGLImage(const gfx::Size &size);
    gfx::NativePixmapHandle exportHandleFromEGLImage(gfx::BufferFormat format,
                                                     const gfx::Size &size,
                                                     gfx::NativePixmapHandle handle);

private:
    EGLHelper();
    std::string getDrmRenderNodeFilePath(const char *extensions) const;
    const char *getLastEGLErrorString() const;

    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    QScopedPointer<EGLFunctions> m_functions;
    bool m_isDmaBufSupported = false;

    bool m_isImageDmaBufExportSupported = false;
    QScopedPointer<GbmBufferFactory> m_gbmBufferFactory;
    QScopedPointer<QOffscreenSurface> m_offscreenSurface;

    // Synchronizes m_offscreenSurface access between UI thread (initialization) and
    // the GPU thread (usage).
    mutable QMutex m_mutex;
};

QT_END_NAMESPACE

#endif // EGL_HELPER_H
