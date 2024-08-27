// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "gl_context_qt.h"

#include <QGuiApplication>
#include <QOpenGLContext>
#include <QThread>
#include <QtGui/private/qtgui-config_p.h>
#include <qpa/qplatformnativeinterface.h>

#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface.h"

#if defined(USE_OZONE)
#include "ui/gl/egl_util.h"

#include <QOpenGLFunctions>
#include <QOffscreenSurface>

#include <vector>
#endif

#if BUILDFLAG(IS_WIN)
#include "ui/gl/gl_context_egl.h"
#include "ui/gl/gl_context_wgl.h"
#endif

QT_BEGIN_NAMESPACE

Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
GLContextHelper* GLContextHelper::contextHelper = nullptr;

namespace {

inline void *resourceForContext(const QByteArray &resource)
{
#if QT_CONFIG(opengl)
    QOpenGLContext *shareContext = qt_gl_global_share_context();
    if (!shareContext) {
        qFatal("QWebEngine: OpenGL resource sharing is not set up in QtQuick. Please make sure to "
               "call QtWebEngineQuick::initialize() in your main() function.");
    }
    return qApp->platformNativeInterface()->nativeResourceForContext(resource, shareContext);
#else
    return nullptr;
#endif
}

inline void *resourceForIntegration(const QByteArray &resource)
{
    return qApp->platformNativeInterface()->nativeResourceForIntegration(resource);
}

}

void GLContextHelper::initialize()
{
    if (!contextHelper)
        contextHelper = new GLContextHelper;
#if QT_CONFIG(opengl)
    if (QGuiApplication::platformName() == QLatin1String("offscreen")){
        contextHelper->m_robustness = false;
        return;
    }

    if (QOpenGLContext *context = qt_gl_global_share_context())
        contextHelper->m_robustness = context->format().testOption(QSurfaceFormat::ResetNotification);
#endif
}

void GLContextHelper::destroy()
{
    delete contextHelper;
    contextHelper = nullptr;
}

bool GLContextHelper::initializeContextOnBrowserThread(gl::GLContext* context, gl::GLSurface* surface, gl::GLContextAttribs attribs)
{
    return context->Initialize(surface, attribs);
}

bool GLContextHelper::initializeContext(gl::GLContext* context, gl::GLSurface* surface, gl::GLContextAttribs attribs)
{
    bool ret = false;
    Qt::ConnectionType connType = (QThread::currentThread() == qApp->thread()) ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
    QMetaObject::invokeMethod(contextHelper, "initializeContextOnBrowserThread", connType,
            Q_RETURN_ARG(bool, ret),
            Q_ARG(gl::GLContext*, context),
            Q_ARG(gl::GLSurface*, surface),
            Q_ARG(gl::GLContextAttribs, attribs));
    return ret;
}

void* GLContextHelper::getEGLConfig()
{
    QByteArray resource = QByteArrayLiteral("eglconfig");
    return resourceForContext(resource);
}

void* GLContextHelper::getGlXConfig()
{
    QByteArray resource = QByteArrayLiteral("glxconfig");
    return resourceForContext(resource);
}

void* GLContextHelper::getEGLDisplay()
{
#if BUILDFLAG(IS_WIN)
    // Windows QPA plugin does not implement resourceForIntegration for "egldisplay".
    // Use resourceForContext instead.
    return resourceForContext(QByteArrayLiteral("egldisplay"));
#else
    return resourceForIntegration(QByteArrayLiteral("egldisplay"));
#endif
}

void* GLContextHelper::getXDisplay()
{
#if QT_CONFIG(xcb)
    auto *x11app = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    return x11app ? x11app->display() : nullptr;
#else
    return nullptr;
#endif
}

void* GLContextHelper::getNativeDisplay()
{
    return resourceForIntegration(QByteArrayLiteral("nativedisplay"));
}

QFunctionPointer GLContextHelper::getGlXGetProcAddress()
{
     QFunctionPointer get_proc_address = nullptr;
#if QT_CONFIG(xcb_glx)
    if (QOpenGLContext *context = qt_gl_global_share_context()) {
        get_proc_address = context->getProcAddress("glXGetProcAddress");
    }
#endif
    return get_proc_address;
}

QFunctionPointer GLContextHelper::getEglGetProcAddress()
{
     QFunctionPointer get_proc_address = nullptr;
#if QT_CONFIG(opengl)
    if (QOpenGLContext *context = qt_gl_global_share_context()) {
        get_proc_address = context->getProcAddress("eglGetProcAddress");
    }
#endif
    return get_proc_address;
}

void *GLContextHelper::getGlxPlatformInterface()
{
#if QT_CONFIG(xcb_glx)
    if (QOpenGLContext *context = qt_gl_global_share_context())
        return context->nativeInterface<QNativeInterface::QGLXContext>();
#endif
    return nullptr;
}

void *GLContextHelper::getEglPlatformInterface()
{
#if QT_CONFIG(opengl) && QT_CONFIG(egl)
    if (QOpenGLContext *context = qt_gl_global_share_context())
        return context->nativeInterface<QNativeInterface::QEGLContext>();
#endif
    return nullptr;
}

bool GLContextHelper::isCreateContextRobustnessSupported()
{
    return contextHelper->m_robustness;
}

#if QT_CONFIG(opengl) && defined(USE_OZONE)
class ScopedGLContext
{
public:
    ScopedGLContext(QOffscreenSurface *surface)
        : m_context(new QOpenGLContext())
        , m_previousContext(gl::GLContext::GetCurrent())
        , m_previousSurface(gl::GLSurface::GetCurrent())
    {
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
            auto glFun = m_context->functions();
            glFun->glDeleteTextures(m_textures.size(), m_textures.data());
        }

        if (m_previousContext)
            m_previousContext->MakeCurrent(m_previousSurface);
    }

    bool isValid() const { return m_context->isValid() && (m_context->surface() != nullptr); }

    EGLDisplay eglDisplay() const
    {
        QNativeInterface::QEGLContext *nativeInterface =
                m_context->nativeInterface<QNativeInterface::QEGLContext>();
        return nativeInterface->display();
    }

    EGLContext eglContext() const
    {
        QNativeInterface::QEGLContext *nativeInterface =
                m_context->nativeInterface<QNativeInterface::QEGLContext>();
        return nativeInterface->nativeContext();
    }

    uint createTexture(int width, int height)
    {
        auto glFun = m_context->functions();

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
    gl::GLContext *m_previousContext;
    gl::GLSurface *m_previousSurface;
    std::vector<uint> m_textures;
};

EGLHelper::EGLFunctions::EGLFunctions()
{
    const static auto getProcAddress =
            reinterpret_cast<gl::GLGetProcAddressProc>(GLContextHelper::getEglGetProcAddress());

    eglCreateImage = reinterpret_cast<PFNEGLCREATEIMAGEPROC>(getProcAddress("eglCreateImage"));
    eglDestroyImage = reinterpret_cast<PFNEGLDESTROYIMAGEPROC>(getProcAddress("eglDestroyImage"));
    eglGetError = reinterpret_cast<PFNEGLGETERRORPROC>(getProcAddress("eglGetError"));
    eglExportDMABUFImageMESA = reinterpret_cast<PFNEGLEXPORTDMABUFIMAGEMESAPROC>(
            getProcAddress("eglExportDMABUFImageMESA"));
    eglExportDMABUFImageQueryMESA = reinterpret_cast<PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC>(
            getProcAddress("eglExportDMABUFImageQueryMESA"));
    eglQueryString = reinterpret_cast<PFNEGLQUERYSTRINGPROC>(getProcAddress("eglQueryString"));
}

EGLHelper *EGLHelper::instance()
{
    static EGLHelper eglHelper;
    return &eglHelper;
}

EGLHelper::EGLHelper()
    : m_functions(new EGLHelper::EGLFunctions()), m_offscreenSurface(new QOffscreenSurface())
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

    auto eglDisplay = GLContextHelper::getEGLDisplay();
    if (!eglDisplay) {
        qWarning("EGL: No EGL display.");
        return;
    }

    Q_ASSERT(QThread::currentThread() == qApp->thread());
    m_offscreenSurface->create();

    const char *displayExtensions = m_functions->eglQueryString(eglDisplay, EGL_EXTENSIONS);
    m_isDmaBufSupported = strstr(displayExtensions, "EGL_EXT_image_dma_buf_import")
            && strstr(displayExtensions, "EGL_EXT_image_dma_buf_import_modifiers")
            && strstr(displayExtensions, "EGL_MESA_image_dma_buf_export");

    if (m_isDmaBufSupported) {
        // FIXME: This disables GBM for nvidia. Remove this when nvidia fixes its GBM support.
        //
        // "Buffer allocation and submission to DRM KMS using gbm is not currently supported."
        // See: https://download.nvidia.com/XFree86/Linux-x86_64/550.40.07/README/kms.html
        //
        // Chromium uses GBM to allocate scanout buffers. Scanout requires DRM KMS. If KMS is
        // enabled, gbm_device and gbm_buffer are created without any issues but rendering to the
        // buffer will malfunction. It is not known how to detect this problem before rendering
        // so we just disable GBM for nvidia.
        const char *displayVendor = m_functions->eglQueryString(eglDisplay, EGL_VENDOR);
        m_isDmaBufSupported = !strstr(displayVendor, "NVIDIA");
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

    ScopedGLContext context(m_offscreenSurface.get());
    if (!context.isValid())
        return;

    EGLDisplay eglDisplay = context.eglDisplay();
    EGLContext eglContext = context.eglContext();
    if (!eglContext) {
        qWarning("EGL: No EGLContext.");
        return;
    }

    uint64_t textureId = context.createTexture(width, height);
    EGLImage eglImage = m_functions->eglCreateImage(eglDisplay, eglContext, EGL_GL_TEXTURE_2D,
                                                    (EGLClientBuffer)textureId, NULL);
    if (eglImage == EGL_NO_IMAGE) {
        qWarning() << "EGL: Failed to create EGLImage:"
                   << ui::GetEGLErrorString(m_functions->eglGetError());
        return;
    }

    int numPlanes = 0;
    if (!m_functions->eglExportDMABUFImageQueryMESA(eglDisplay, eglImage, nullptr, &numPlanes,
                                                    modifiers))
        qWarning() << "EGL: Failed to retrieve the pixel format of the buffer:"
                   << ui::GetEGLErrorString(m_functions->eglGetError());
    Q_ASSERT(numPlanes == 1);

    if (!m_functions->eglExportDMABUFImageMESA(eglDisplay, eglImage, fd, stride, offset))
        qWarning() << "EGL: Failed to retrieve the dma_buf file descriptor:"
                   << ui::GetEGLErrorString(m_functions->eglGetError());

    m_functions->eglDestroyImage(eglDisplay, eglImage);
}

bool EGLHelper::isDmaBufSupported()
{
    return m_isDmaBufSupported;
}
#endif // QT_CONFIG(opengl) && defined(USE_OZONE)

QT_END_NAMESPACE

#if BUILDFLAG(IS_WIN)
namespace gl {
namespace init {

scoped_refptr<GLContext> CreateGLContext(GLShareGroup *share_group,
                                         GLSurface *compatible_surface,
                                         const GLContextAttribs &attribs)
{
    switch (GetGLImplementation()) {
    case kGLImplementationDesktopGLCoreProfile:
    case kGLImplementationDesktopGL: {
        scoped_refptr<GLContext> context = new GLContextWGL(share_group);
        if (!context->Initialize(compatible_surface, attribs))
            return nullptr;
        return context;
    }
    case kGLImplementationEGLANGLE:
    case kGLImplementationEGLGLES2:
        return InitializeGLContext(new GLContextEGL(share_group),
                                   compatible_surface, attribs);
    case kGLImplementationDisabled:
        return nullptr;
    default:
        break;
    }
    Q_UNREACHABLE();
    return nullptr;
}

}  // namespace init
}  // namespace gl

#endif // BUILDFLAG(IS_WIN)
