// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef EGL_HELPER_H
#define EGL_HELPER_H

#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>

#include "base/files/scoped_file.h"
#include "ui/gfx/native_pixmap_handle.h"

#include <memory>

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

namespace ui {
class GbmBuffer;
class GbmDevice;
}

QT_BEGIN_NAMESPACE

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

    EGLDisplay getEGLDisplay() const { return m_eglDisplay; }
    EGLFunctions *functions() const { return m_functions.get(); }
    bool isDmaBufSupported() const { return m_isDmaBufSupported; }

    std::unique_ptr<ui::GbmBuffer> createBuffer(gfx::BufferFormat format, gfx::Size size,
                                                gfx::BufferUsage usage);
    std::unique_ptr<ui::GbmBuffer> createBufferFromHandle(gfx::Size size, gfx::BufferFormat format,
                                                          gfx::NativePixmapHandle handle);

    gfx::NativePixmapHandle exportHandleFromEGLImage(const gfx::Size &size);
    gfx::NativePixmapHandle exportHandleFromEGLImage(const gfx::Size &size,
                                                     gfx::BufferFormat format,
                                                     gfx::NativePixmapHandle handle);

private:
    struct ScopedGbmDevice
    {
        ScopedGbmDevice(const std::string &nodePath);

        base::ScopedFD nodeFD;
        std::unique_ptr<ui::GbmDevice> device;
    };

    EGLHelper();
    std::string getDrmRenderNodeFilePath(const char *extensions) const;
    const char *getLastEGLErrorString() const;

    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    QScopedPointer<EGLFunctions> m_functions;

    bool m_isDmaBufSupported = false;
    bool m_isImageDmaBufExportSupported = false;
    QScopedPointer<ScopedGbmDevice> m_gbmDevice;
    QScopedPointer<QOffscreenSurface> m_offscreenSurface;

    mutable QMutex m_mutex;
};

QT_END_NAMESPACE

#endif // EGL_HELPER_H
