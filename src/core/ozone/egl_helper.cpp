// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "egl_helper.h"
#include "ozone_util_qt.h"
#include "web_engine_context.h"

#include <QtCore/qthread.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qoffscreensurface.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglfunctions.h>
#include <qpa/qplatformnativeinterface.h>

#include <cstdint>
#include <unistd.h>
#include <vector>

namespace {
static const char *getEGLErrorString(uint32_t error)
{
    switch (error) {
    case EGL_SUCCESS:
        return "EGL_SUCCESS";
    case EGL_NOT_INITIALIZED:
        return "EGL_NOT_INITIALIZED";
    case EGL_BAD_ACCESS:
        return "EGL_BAD_ACCESS";
    case EGL_BAD_ALLOC:
        return "EGL_BAD_ALLOC";
    case EGL_BAD_ATTRIBUTE:
        return "EGL_BAD_ATTRIBUTE";
    case EGL_BAD_CONFIG:
        return "EGL_BAD_CONFIG";
    case EGL_BAD_CONTEXT:
        return "EGL_BAD_CONTEXT";
    case EGL_BAD_CURRENT_SURFACE:
        return "EGL_BAD_CURRENT_SURFACE";
    case EGL_BAD_DISPLAY:
        return "EGL_BAD_DISPLAY";
    case EGL_BAD_MATCH:
        return "EGL_BAD_MATCH";
    case EGL_BAD_NATIVE_PIXMAP:
        return "EGL_BAD_NATIVE_PIXMAP";
    case EGL_BAD_NATIVE_WINDOW:
        return "EGL_BAD_NATIVE_WINDOW";
    case EGL_BAD_PARAMETER:
        return "EGL_BAD_PARAMETER";
    case EGL_BAD_SURFACE:
        return "EGL_BAD_SURFACE";
    case EGL_CONTEXT_LOST:
        return "EGL_CONTEXT_LOST";
    default:
        return "UNKNOWN";
    }
}
} // namespace

QT_BEGIN_NAMESPACE

EGLHelper::EGLFunctions::EGLFunctions()
{
    QOpenGLContext *context = OzoneUtilQt::getQOpenGLContext();

    eglCreateImage =
            reinterpret_cast<PFNEGLCREATEIMAGEPROC>(context->getProcAddress("eglCreateImage"));
    eglCreateDRMImageMESA = reinterpret_cast<PFNEGLCREATEDRMIMAGEMESAPROC>(
            context->getProcAddress("eglCreateDRMImageMESA"));
    eglDestroyImage =
            reinterpret_cast<PFNEGLDESTROYIMAGEPROC>(context->getProcAddress("eglDestroyImage"));
    eglExportDMABUFImageMESA = reinterpret_cast<PFNEGLEXPORTDMABUFIMAGEMESAPROC>(
            context->getProcAddress("eglExportDMABUFImageMESA"));
    eglExportDMABUFImageQueryMESA = reinterpret_cast<PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC>(
            context->getProcAddress("eglExportDMABUFImageQueryMESA"));
    eglGetCurrentContext = reinterpret_cast<PFNEGLGETCURRENTCONTEXTPROC>(
            context->getProcAddress("eglGetCurrentContext"));
    eglGetCurrentDisplay = reinterpret_cast<PFNEGLGETCURRENTDISPLAYPROC>(
            context->getProcAddress("eglGetCurrentDisplay"));
    eglGetCurrentSurface = reinterpret_cast<PFNEGLGETCURRENTSURFACEPROC>(
            context->getProcAddress("eglGetCurrentSurface"));
    eglGetError = reinterpret_cast<PFNEGLGETERRORPROC>(context->getProcAddress("eglGetError"));
    eglMakeCurrent =
            reinterpret_cast<PFNEGLMAKECURRENTPROC>(context->getProcAddress("eglMakeCurrent"));
    eglQueryString =
            reinterpret_cast<PFNEGLQUERYSTRINGPROC>(context->getProcAddress("eglQueryString"));
}

EGLHelper *EGLHelper::instance()
{
    static EGLHelper eglHelper;
    return &eglHelper;
}

EGLHelper::EGLHelper()
    : m_eglDisplay(qApp->platformNativeInterface()->nativeResourceForIntegration("egldisplay"))
    , m_functions(new EGLHelper::EGLFunctions())
    , m_offscreenSurface(new QOffscreenSurface())
{
    const char *extensions = m_functions->eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!extensions) {
        qWarning("EGL: Failed to query EGL extensions.");
        return;
    }

    if (strstr(extensions, "EGL_KHR_base_image")) {
        qWarning("EGL: EGL_KHR_base_image extension is not supported.");
        return;
    }

    if (m_eglDisplay == EGL_NO_DISPLAY) {
        qWarning("EGL: No EGL display.");
        return;
    }

    Q_ASSERT(QThread::currentThread() == qApp->thread());
    m_offscreenSurface->create();

    m_isDmaBufSupported = QtWebEngineCore::WebEngineContext::isGbmSupported();

    // Check extensions.
    if (m_isDmaBufSupported) {
        const char *displayExtensions = m_functions->eglQueryString(m_eglDisplay, EGL_EXTENSIONS);
        m_isDmaBufSupported = strstr(displayExtensions, "EGL_EXT_image_dma_buf_import")
                && strstr(displayExtensions, "EGL_EXT_image_dma_buf_import_modifiers")
                && strstr(displayExtensions, "EGL_MESA_drm_image")
                && strstr(displayExtensions, "EGL_MESA_image_dma_buf_export");
    }

    // Try to create dma-buf.
    if (m_isDmaBufSupported) {
        int fd = -1;
        queryDmaBuf(2, 2, &fd, nullptr, nullptr, nullptr);
        if (fd == -1)
            m_isDmaBufSupported = false;
        else
            close(fd);
    }
}

void EGLHelper::queryDmaBuf(const int width, const int height, int *fd, int *stride, int *offset,
                            uint64_t *modifiers)
{
    if (!m_isDmaBufSupported)
        return;

    // clang-format off
    EGLint attribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_DRM_BUFFER_FORMAT_MESA, EGL_DRM_BUFFER_FORMAT_ARGB32_MESA,
        EGL_DRM_BUFFER_USE_MESA, EGL_DRM_BUFFER_USE_SHARE_MESA,
        EGL_NONE
    };
    // clang-format on

    EGLImage eglImage = m_functions->eglCreateDRMImageMESA(m_eglDisplay, attribs);
    if (eglImage == EGL_NO_IMAGE) {
        qWarning("EGL: Failed to create EGLImage: %s", getLastEGLErrorString());
        return;
    }

    int numPlanes = 0;
    if (!m_functions->eglExportDMABUFImageQueryMESA(m_eglDisplay, eglImage, nullptr, &numPlanes,
                                                    modifiers)) {
        qWarning("EGL: Failed to retrieve the pixel format of the buffer: %s",
                 getLastEGLErrorString());
    }
    Q_ASSERT(numPlanes == 1);

    if (!m_functions->eglExportDMABUFImageMESA(m_eglDisplay, eglImage, fd, stride, offset)) {
        qWarning("EGL: Failed to retrieve the dma_buf file descriptor: %s",
                 getLastEGLErrorString());
    }

    m_functions->eglDestroyImage(m_eglDisplay, eglImage);
}

const char *EGLHelper::getLastEGLErrorString() const
{
    return getEGLErrorString(m_functions->eglGetError());
}

QT_END_NAMESPACE
