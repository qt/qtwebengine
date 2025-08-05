// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef EGL_HELPER_H
#define EGL_HELPER_H

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#undef eglCreateImage
#undef eglCreateDRMImageMESA
#undef eglDestroyImage
#undef eglExportDMABUFImageMESA
#undef eglExportDMABUFImageQueryMESA
#undef eglGetCurrentContext
#undef eglGetCurrentDisplay
#undef eglGetCurrentSurface
#undef eglGetError
#undef eglMakeCurrent
#undef eglQueryString

QT_BEGIN_NAMESPACE

class EGLHelper
{
public:
    struct EGLFunctions
    {
        EGLFunctions();

        PFNEGLCREATEIMAGEPROC eglCreateImage;
        PFNEGLCREATEDRMIMAGEMESAPROC eglCreateDRMImageMESA;
        PFNEGLDESTROYIMAGEPROC eglDestroyImage;
        PFNEGLEXPORTDMABUFIMAGEMESAPROC eglExportDMABUFImageMESA;
        PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC eglExportDMABUFImageQueryMESA;
        PFNEGLGETCURRENTCONTEXTPROC eglGetCurrentContext;
        PFNEGLGETCURRENTDISPLAYPROC eglGetCurrentDisplay;
        PFNEGLGETCURRENTSURFACEPROC eglGetCurrentSurface;
        PFNEGLGETERRORPROC eglGetError;
        PFNEGLMAKECURRENTPROC eglMakeCurrent;
        PFNEGLQUERYSTRINGPROC eglQueryString;
    };

    static EGLHelper *instance();

    EGLDisplay getEGLDisplay() const { return m_eglDisplay; }
    EGLFunctions *functions() const { return m_functions.get(); }
    void queryDmaBuf(const int width, const int height, int *fd, int *stride, int *offset,
                     uint64_t *modifiers);
    bool isDmaBufSupported() const { return m_isDmaBufSupported; }
    const char *getLastEGLErrorString() const;

private:
    EGLHelper();

    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    QScopedPointer<EGLFunctions> m_functions;
    bool m_isDmaBufSupported = false;
};

QT_END_NAMESPACE

#endif // EGL_HELPER_H
