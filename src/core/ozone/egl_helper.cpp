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

class ScopedGLContext
{
public:
    ScopedGLContext(QOffscreenSurface *surface, EGLHelper::EGLFunctions *eglFun)
        : m_context(new QOpenGLContext()), m_eglFun(eglFun)
    {
        if ((m_previousEGLContext = m_eglFun->eglGetCurrentContext())) {
            m_previousEGLDrawSurface = m_eglFun->eglGetCurrentSurface(EGL_DRAW);
            m_previousEGLReadSurface = m_eglFun->eglGetCurrentSurface(EGL_READ);
            m_previousEGLDisplay = m_eglFun->eglGetCurrentDisplay();
        }

        if (!m_context->create()) {
            qWarning("Failed to create OpenGL context.");
            return;
        }

        Q_ASSERT(surface->isValid());
        if (!m_context->makeCurrent(surface)) {
            qWarning("Failed to make OpenGL context current.");
            return;
        }
    }

    ~ScopedGLContext()
    {
        if (!m_textures.empty()) {
            auto *glFun = m_context->functions();
            glFun->glDeleteTextures(m_textures.size(), m_textures.data());
        }

        if (m_previousEGLContext) {
            // Make sure the scoped context is not current when restoring the previous
            // EGL context otherwise the QOpenGLContext destructor resets the restored
            // current context.
            m_context->doneCurrent();

            m_eglFun->eglMakeCurrent(m_previousEGLDisplay, m_previousEGLDrawSurface,
                                     m_previousEGLReadSurface, m_previousEGLContext);
            if (m_eglFun->eglGetError() != EGL_SUCCESS)
                qWarning("Failed to restore EGL context.");
        }
    }

    bool isValid() const { return m_context->isValid() && (m_context->surface() != nullptr); }

    EGLContext eglContext() const
    {
        QNativeInterface::QEGLContext *nativeInterface =
                m_context->nativeInterface<QNativeInterface::QEGLContext>();
        return nativeInterface->nativeContext();
    }

    uint createTexture(int width, int height)
    {
        auto *glFun = m_context->functions();

        uint glTexture;
        glFun->glGenTextures(1, &glTexture);
        glFun->glBindTexture(GL_TEXTURE_2D, glTexture);
        glFun->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                            NULL);
        glFun->glBindTexture(GL_TEXTURE_2D, 0);

        m_textures.push_back(glTexture);
        return glTexture;
    }

private:
    QScopedPointer<QOpenGLContext> m_context;
    EGLHelper::EGLFunctions *m_eglFun;
    EGLContext m_previousEGLContext = nullptr;
    EGLSurface m_previousEGLDrawSurface = nullptr;
    EGLSurface m_previousEGLReadSurface = nullptr;
    EGLDisplay m_previousEGLDisplay = nullptr;
    std::vector<uint> m_textures;
};

EGLHelper::EGLFunctions::EGLFunctions()
{
    QOpenGLContext *context = OzoneUtilQt::getQOpenGLContext();

    eglCreateImage =
            reinterpret_cast<PFNEGLCREATEIMAGEPROC>(context->getProcAddress("eglCreateImage"));
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

    ScopedGLContext context(m_offscreenSurface.get(), m_functions.get());
    if (!context.isValid())
        return;

    EGLContext eglContext = context.eglContext();
    if (!eglContext) {
        qWarning("EGL: No EGLContext.");
        return;
    }

    uint64_t textureId = context.createTexture(width, height);
    EGLImage eglImage = m_functions->eglCreateImage(m_eglDisplay, eglContext, EGL_GL_TEXTURE_2D,
                                                    (EGLClientBuffer)textureId, NULL);
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
