// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "egl_helper.h"

#include "compositor/compositor.h"
#include "ozone/gbm_buffer_factory.h"
#include "ozone/ozone_util_qt.h"
#include "rhi_gpu_info.h"

#include "ui/gfx/buffer_format_util.h"
#include "ui/gfx/linux/drm_util_linux.h"
#include "ui/gl/gl_display.h"
#include "ui/gl/gl_implementation.h"

#include <QtCore/qthread.h>
#include <QtGui/qguiapplication.h>
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

class ScopedNativeEGLContext
{
public:
    ScopedNativeEGLContext(EGLDisplay eglDisplay, EGLHelper::EGLFunctions *eglFun)
        : m_display(eglDisplay), m_eglFun(eglFun)
    {
        if ((m_previousContext = m_eglFun->eglGetCurrentContext())) {
            m_previousDrawSurface = m_eglFun->eglGetCurrentSurface(EGL_DRAW);
            m_previousReadSurface = m_eglFun->eglGetCurrentSurface(EGL_READ);
            m_previousDisplay = m_eglFun->eglGetCurrentDisplay();
        }

        EGLint configAttribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE,
                                   EGL_PBUFFER_BIT, EGL_NONE };
        EGLConfig config;
        EGLint numConfigs;
        m_eglFun->eglChooseConfig(m_display, configAttribs, &config, 1, &numConfigs);

        // Create GLES2 Context.
        EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
        m_scopedContext =
                m_eglFun->eglCreateContext(m_display, config, EGL_NO_CONTEXT, contextAttribs);
        if (m_scopedContext == EGL_NO_CONTEXT) {
            qWarning("EGL: Failed to create native context: %s",
                     getEGLErrorString(m_eglFun->eglGetError()));
            return;
        }

        // Try surfaceless context.
        if (m_eglFun->eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_scopedContext))
            return; // Succeeded.

        // Swallow the the error from the failed surfaceless attempt.
        m_eglFun->eglGetError();

        // Create an offscreen surface if surfaceless failed.
        EGLint pbufferAttribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
        m_scopedSurface = m_eglFun->eglCreatePbufferSurface(m_display, config, pbufferAttribs);
        if (m_scopedSurface == EGL_NO_SURFACE) {
            qWarning("EGL: Failed to create native offscreen surface: %s",
                     getEGLErrorString(m_eglFun->eglGetError()));
            m_eglFun->eglDestroyContext(m_display, m_scopedContext);
            m_scopedContext = EGL_NO_CONTEXT;
            return;
        }

        if (!m_eglFun->eglMakeCurrent(m_display, m_scopedSurface, m_scopedSurface,
                                      m_scopedContext)) {
            qWarning("EGL: Failed to make native context current: %s",
                     getEGLErrorString(m_eglFun->eglGetError()));
            m_eglFun->eglDestroyContext(m_display, m_scopedContext);
            m_eglFun->eglDestroySurface(m_display, m_scopedSurface);
            m_scopedContext = EGL_NO_CONTEXT;
            m_scopedSurface = EGL_NO_SURFACE;
            return;
        }
    }

    ~ScopedNativeEGLContext()
    {
        if (!m_textures.empty()) {
            Q_ASSERT(m_scopedContext != EGL_NO_CONTEXT);
            typedef void (*PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint *textures);
            auto glDeleteTexturesPtr = reinterpret_cast<PFNGLDELETETEXTURESPROC>(
                    m_eglFun->eglGetProcAddress("glDeleteTextures"));
            glDeleteTexturesPtr(m_textures.size(), m_textures.data());
        }

        if (m_previousContext) {
            m_eglFun->eglMakeCurrent(m_previousDisplay, m_previousDrawSurface,
                                     m_previousReadSurface, m_previousContext);
            if (m_eglFun->eglGetError() != EGL_SUCCESS) {
                qWarning("EGL: Failed to restore Chromium's context: %s",
                         getEGLErrorString(m_eglFun->eglGetError()));
            }
        } else {
            m_eglFun->eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        }

        if (m_scopedContext != EGL_NO_CONTEXT)
            m_eglFun->eglDestroyContext(m_display, m_scopedContext);

        if (m_scopedSurface != EGL_NO_SURFACE)
            m_eglFun->eglDestroySurface(m_display, m_scopedSurface);
    }

    bool isValid() const { return m_scopedContext != EGL_NO_CONTEXT; }
    EGLContext eglContext() const { return m_scopedContext; }

    uint createTexture(int width, int height)
    {
        typedef void (*PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
        auto glGenTexturesPtr = reinterpret_cast<PFNGLGENTEXTURESPROC>(
                m_eglFun->eglGetProcAddress("glGenTextures"));
        typedef void (*PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
        auto glBindTexturePtr = reinterpret_cast<PFNGLBINDTEXTUREPROC>(
                m_eglFun->eglGetProcAddress("glBindTexture"));
        typedef void (*PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat,
                                            GLsizei width, GLsizei height, GLint border,
                                            GLenum format, GLenum type, const void *pixels);
        auto glTexImage2DPtr =
                reinterpret_cast<PFNGLTEXIMAGE2DPROC>(m_eglFun->eglGetProcAddress("glTexImage2D"));
        typedef void (*PFNGLFLUSHPROC)(void);
        auto glFlushPtr = reinterpret_cast<PFNGLFLUSHPROC>(m_eglFun->eglGetProcAddress("glFlush"));

        uint glTexture;
        glGenTexturesPtr(1, &glTexture);
        glBindTexturePtr(GL_TEXTURE_2D, glTexture);
        glTexImage2DPtr(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                        NULL);
        glBindTexturePtr(GL_TEXTURE_2D, 0);
        glFlushPtr();

        m_textures.push_back(glTexture);
        return glTexture;
    }

private:
    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLHelper::EGLFunctions *m_eglFun = nullptr;

    EGLContext m_previousContext = EGL_NO_CONTEXT;
    EGLSurface m_previousDrawSurface = EGL_NO_SURFACE;
    EGLSurface m_previousReadSurface = EGL_NO_SURFACE;
    EGLDisplay m_previousDisplay = EGL_NO_DISPLAY;

    EGLContext m_scopedContext = EGL_NO_CONTEXT;
    EGLSurface m_scopedSurface = EGL_NO_SURFACE;

    std::vector<uint> m_textures;
};

EGLHelper::EGLFunctions::EGLFunctions()
{
    QOpenGLContext *context = OzoneUtilQt::getQOpenGLContext();

    // clang-format off
    eglGetError = reinterpret_cast<PFNEGLGETERRORPROC>(
            context->getProcAddress("eglGetError"));
    eglQueryString = reinterpret_cast<PFNEGLQUERYSTRINGPROC>(
            context->getProcAddress("eglQueryString"));

    eglQueryDevices = reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(
            context->getProcAddress("eglQueryDevicesEXT"));
    eglQueryDeviceString = reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(
            context->getProcAddress("eglQueryDeviceStringEXT"));
    eglQueryDisplayAttrib = reinterpret_cast<PFNEGLQUERYDISPLAYATTRIBEXTPROC>(
            context->getProcAddress("eglQueryDisplayAttribEXT"));

    eglQueryDmaBufModifiers = reinterpret_cast<PFNEGLQUERYDMABUFMODIFIERSEXTPROC>(
            context->getProcAddress("eglQueryDmaBufModifiersEXT"));

    eglChooseConfig = reinterpret_cast<PFNEGLCHOOSECONFIGPROC>(
            context->getProcAddress("eglChooseConfig"));
    eglCreateContext = reinterpret_cast<PFNEGLCREATECONTEXTPROC>(
            context->getProcAddress("eglCreateContext"));
    eglCreateImage = reinterpret_cast<PFNEGLCREATEIMAGEPROC>(
            context->getProcAddress("eglCreateImage"));
    eglCreatePbufferSurface = reinterpret_cast<PFNEGLCREATEPBUFFERSURFACEPROC>(
            context->getProcAddress("eglCreatePbufferSurface"));
    eglDestroyContext = reinterpret_cast<PFNEGLDESTROYCONTEXTPROC>(
            context->getProcAddress("eglDestroyContext"));
    eglDestroyImage = reinterpret_cast<PFNEGLDESTROYIMAGEPROC>(
            context->getProcAddress("eglDestroyImage"));
    eglDestroySurface = reinterpret_cast<PFNEGLDESTROYSURFACEPROC>(
            context->getProcAddress("eglDestroySurface"));
    eglGetCurrentContext = reinterpret_cast<PFNEGLGETCURRENTCONTEXTPROC>(
            context->getProcAddress("eglGetCurrentContext"));
    eglGetCurrentDisplay = reinterpret_cast<PFNEGLGETCURRENTDISPLAYPROC>(
            context->getProcAddress("eglGetCurrentDisplay"));
    eglGetCurrentSurface = reinterpret_cast<PFNEGLGETCURRENTSURFACEPROC>(
            context->getProcAddress("eglGetCurrentSurface"));
    eglGetProcAddress = reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(
            context->getProcAddress("eglGetProcAddress"));
    eglInitialize = reinterpret_cast<PFNEGLINITIALIZEPROC>(
            context->getProcAddress("eglInitialize"));
    eglMakeCurrent = reinterpret_cast<PFNEGLMAKECURRENTPROC>(
            context->getProcAddress("eglMakeCurrent"));

    eglExportDMABUFImageMESA = reinterpret_cast<PFNEGLEXPORTDMABUFIMAGEMESAPROC>(
            context->getProcAddress("eglExportDMABUFImageMESA"));
    eglExportDMABUFImageQueryMESA = reinterpret_cast<PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC>(
            context->getProcAddress("eglExportDMABUFImageQueryMESA"));
    // clang-format on
}

EGLHelper *EGLHelper::instance()
{
    static EGLHelper eglHelper;
    return &eglHelper;
}

EGLHelper::EGLHelper()
    : m_eglDisplay(qApp->platformNativeInterface()->nativeResourceForIntegration("egldisplay"))
    , m_functions(new EGLHelper::EGLFunctions())
    , m_isDmaBufSupported(QtWebEngineCore::RhiGpuInfo::instance()->isGbmSupported())
{
    // The rest of the initialization required for DMA-BUF support.
    if (!m_isDmaBufSupported)
        return;

    const char *extensions = m_functions->eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!extensions) {
        qWarning("EGL: Failed to query EGL extensions.");
        m_isDmaBufSupported = false;
        return;
    }

    if (m_eglDisplay == EGL_NO_DISPLAY) {
        qWarning("EGL: No EGL display.");
        m_isDmaBufSupported = false;
        return;
    }

    const char *displayExtensions = m_functions->eglQueryString(m_eglDisplay, EGL_EXTENSIONS);
    if (!displayExtensions) {
        qWarning("EGL: Failed to query EGL Display extensions.");
        m_isDmaBufSupported = false;
        return;
    }

    if (!strstr(displayExtensions, "EGL_EXT_image_dma_buf_import")) {
        qWarning("EGL: EGL_EXT_image_dma_buf_import extension is not supported.");
        m_isDmaBufSupported = false;
        return;
    }

    // TODO: Remove this in a future release.
    // GBM is now used directly for buffer creation. The EGL-based allocation path is kept
    // temporarily as a fallback and can be preferred for configurations where GBM does not work
    // correctly.
    const char kUseEGLBufferAllocEnv[] = "QTWEBENGINE_USE_EGL_BUFFER_ALLOCATION";
    bool useEGLImageToExportHandle = false;
    if (Q_UNLIKELY(qEnvironmentVariableIsSet(kUseEGLBufferAllocEnv))) {
        bool ok;
        int value = qEnvironmentVariableIntValue(kUseEGLBufferAllocEnv, &ok);
        if (ok && value != 0) {
            qWarning("%s environment variable is set and it is for debugging purposes only.",
                     kUseEGLBufferAllocEnv);
            qWarning("Bypassing direct GBM API in favor of EGL-based allocation.");
            useEGLImageToExportHandle = true;
        }

        if (!ok) {
            qWarning("Ignoring invalid value of %s. Use '1' to prefer EGL-based allocation.",
                     kUseEGLBufferAllocEnv);
        }
    }

    // Create GBM device.
    if (!useEGLImageToExportHandle) {
        const std::string nodePath = getDrmRenderNodeFilePath(extensions);
        qCDebug(QtWebEngineCore::lcWebEngineCompositor, "EGL: DRM Render Node file path: %s",
                nodePath.data());
        m_gbmBufferFactory.reset(new GbmBufferFactory(nodePath));
    }

    // Check necessary extensions for EGL-based allocation fallback.
    m_isImageDmaBufExportSupported = strstr(displayExtensions, "EGL_MESA_image_dma_buf_export");
    if (!m_isImageDmaBufExportSupported) {
        qCDebug(QtWebEngineCore::lcWebEngineCompositor,
                "EGL: EGL_MESA_image_dma_buf_export extension is not supported. EGL-based "
                "allocation fallback is disabled.");
    }

    m_isDmaBufSupported = (m_gbmBufferFactory && m_gbmBufferFactory->hasDevice())
            || m_isImageDmaBufExportSupported;
}

EGLHelper::~EGLHelper() = default;

bool EGLHelper::canCreateNativePixmapForFormat(gfx::BufferFormat format) const
{
    // TODO: Temporary EGL-based fallback. Be permissive here to simplify the fallback path.
    if (m_isImageDmaBufExportSupported)
        return true;

    if (!m_gbmBufferFactory)
        return false;

    return m_gbmBufferFactory->canCreateNativePixmapForFormat(format);
}

const std::vector<uint64_t> &EGLHelper::getSupportedModifiers(gfx::BufferFormat format) const
{
    auto it = m_supportedModifiers.find(format);
    if (it != m_supportedModifiers.end())
        return it->second;

    // Create an empty entry for the format.
    std::vector<uint64_t> &cachedModifiers = m_supportedModifiers[format];

    const char *displayExtensions = m_functions->eglQueryString(m_eglDisplay, EGL_EXTENSIONS);
    if (!displayExtensions) {
        qWarning("EGL: Failed to query EGL Display extensions.");
        return cachedModifiers;
    }

    if (!strstr(displayExtensions, "EGL_EXT_image_dma_buf_import_modifiers")) {
        qWarning("EGL: EGL_EXT_image_dma_buf_import_modifiers extension is not supported.");
        return cachedModifiers;
    }

    const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
    EGLint numModifiers = 0;
    EGLBoolean success;
    success = m_functions->eglQueryDmaBufModifiers(m_eglDisplay, fourccFormat, 0, nullptr, nullptr,
                                                   &numModifiers);
    if (!success || numModifiers == 0) {
        qWarning("EGL: Failed to query DRM format modifiers.");
        return cachedModifiers;
    }

    std::vector<EGLuint64KHR> modifiers(numModifiers);
    std::vector<EGLBoolean> externalOnly(numModifiers);
    success = m_functions->eglQueryDmaBufModifiers(m_eglDisplay, fourccFormat, numModifiers,
                                                   modifiers.data(), externalOnly.data(),
                                                   &numModifiers);
    if (!success || numModifiers == 0) {
        qWarning("EGL: Failed to query DRM format modifiers.");
        return cachedModifiers;
    }

    cachedModifiers.reserve(numModifiers);
    for (EGLint i = 0; i < numModifiers; ++i) {
        if (!externalOnly[i] && m_gbmBufferFactory->isSinglePlanar(fourccFormat, modifiers[i]))
            cachedModifiers.push_back(modifiers[i]);
    }

    return cachedModifiers;
}

gfx::NativePixmapHandle EGLHelper::exportHandleFromEGLImage(const gfx::Size &size)
{
    if (!m_isDmaBufSupported || !m_isImageDmaBufExportSupported)
        return gfx::NativePixmapHandle();

    // ANGLE cannot export DMA-BUFs directly so a native EGL context is required.
    // Ensure the current ANGLE backend allows switching to a native EGL context.
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        if (gl::GetGLImplementation() != gl::kGLImplementationEGLANGLE)
            return;

        if (gl::GLDisplayEGL *display = gl::GLDisplayEGL::GetDisplayForCurrentContext())
            m_isImageDmaBufExportSupported = display->IsANGLEExternalContextAndSurfaceSupported();
    });

    if (!m_isImageDmaBufExportSupported) {
        qWarning("EGL: ANGLE backend lacks native interop for EGL-based allocation. "
                 "Try forcing native EGL with: --use-gl=egl.");
        return gfx::NativePixmapHandle();
    }

    ScopedNativeEGLContext nativeContext(m_eglDisplay, m_functions.get());
    if (!nativeContext.isValid()) {
        qWarning("EGL: Failed to create valid GL context.");
        return gfx::NativePixmapHandle();
    }

    EGLContext eglContext = nativeContext.eglContext();
    if (!eglContext) {
        qWarning("EGL: No EGLContext.");
        return gfx::NativePixmapHandle();
    }

    uint64_t textureId = nativeContext.createTexture(size.width(), size.height());
    EGLImage eglImage = m_functions->eglCreateImage(m_eglDisplay, eglContext, EGL_GL_TEXTURE_2D,
                                                    (EGLClientBuffer)textureId, NULL);

    if (eglImage == EGL_NO_IMAGE) {
        qWarning("EGL: Failed to create EGLImage: %s", getLastEGLErrorString());
        return gfx::NativePixmapHandle();
    }

    int numPlanes = 0;
    uint64_t modifiers;
    if (!m_functions->eglExportDMABUFImageQueryMESA(m_eglDisplay, eglImage, nullptr, &numPlanes,
                                                    &modifiers)) {
        qWarning("EGL: Failed to retrieve the pixel format of the buffer: %s",
                 getLastEGLErrorString());
        return gfx::NativePixmapHandle();
    }

    gfx::NativePixmapHandle bufferHandle;
    bufferHandle.modifier = modifiers;

    Q_ASSERT(numPlanes == 1);
    int fd = -1;
    int stride;
    int offset;
    if (!m_functions->eglExportDMABUFImageMESA(m_eglDisplay, eglImage, &fd, &stride, &offset)) {
        qWarning("EGL: Failed to retrieve the dma_buf file descriptor: %s",
                 getLastEGLErrorString());
        return gfx::NativePixmapHandle();
    }
    m_functions->eglDestroyImage(m_eglDisplay, eglImage);

    if (fd == -1) {
        qWarning("EGL: Failed to query DRM FD.");
        return gfx::NativePixmapHandle();
    }

    const uint64_t planeSize = uint64_t(size.width() * size.height() * 4);
    gfx::NativePixmapPlane plane(stride, offset, planeSize, base::ScopedFD(::dup(fd)));
    bufferHandle.planes.push_back(std::move(plane));

    return bufferHandle;
}

gfx::NativePixmapHandle EGLHelper::exportHandleFromEGLImage(gfx::BufferFormat format,
                                                            const gfx::Size &size,
                                                            gfx::NativePixmapHandle handle)
{
    if (!m_isDmaBufSupported || !m_isImageDmaBufExportSupported)
        return gfx::NativePixmapHandle();

    const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
    const size_t numPlanes = handle.planes.size();

    std::vector<EGLAttrib> attrs;
    attrs.push_back(EGL_WIDTH);
    attrs.push_back(size.width());
    attrs.push_back(EGL_HEIGHT);
    attrs.push_back(size.height());
    attrs.push_back(EGL_LINUX_DRM_FOURCC_EXT);
    attrs.push_back(fourccFormat);
    for (size_t planeIndex = 0; planeIndex < numPlanes; ++planeIndex) {
        attrs.push_back(EGL_DMA_BUF_PLANE0_FD_EXT + planeIndex * 3);
        attrs.push_back(handle.planes[planeIndex].fd.get());
        attrs.push_back(EGL_DMA_BUF_PLANE0_OFFSET_EXT + planeIndex * 3);
        attrs.push_back(handle.planes[planeIndex].offset);
        attrs.push_back(EGL_DMA_BUF_PLANE0_PITCH_EXT + planeIndex * 3);
        attrs.push_back(handle.planes[planeIndex].stride);
        attrs.push_back(EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT + planeIndex * 2);
        attrs.push_back(handle.modifier & 0xffffffff);
        attrs.push_back(EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT + planeIndex * 2);
        attrs.push_back(handle.modifier >> 32);
    }
    attrs.push_back(EGL_NONE);

    EGLImage eglImage =
            m_functions->eglCreateImage(m_eglDisplay, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
                                        (EGLClientBuffer)NULL, attrs.data());
    if (eglImage == EGL_NO_IMAGE) {
        qWarning("EGL: Failed to create EGLImage: %s", getLastEGLErrorString());
        return gfx::NativePixmapHandle();
    }

    Q_ASSERT(numPlanes <= 3);
    int fds[3];
    int strides[3];
    int offsets[3];
    if (!m_functions->eglExportDMABUFImageMESA(m_eglDisplay, eglImage, fds, strides, offsets)) {
        qWarning("EGL: Failed to retrieve the dma_buf file descriptor: %s",
                 getLastEGLErrorString());
        return gfx::NativePixmapHandle();
    }
    m_functions->eglDestroyImage(m_eglDisplay, eglImage);

    gfx::NativePixmapHandle bufferHandle;
    bufferHandle.modifier = handle.modifier;
    for (size_t i = 0; i < numPlanes; ++i) {
        int fd = fds[i];
        int stride = strides[i];
        int offset = offsets[i];
        int planeSize = handle.planes[i].size;

        if (fd == -1) {
            fd = fds[0];
            stride = handle.planes[i].stride;
            offset = handle.planes[i].offset;
        }

        gfx::NativePixmapPlane plane(stride, offset, planeSize, base::ScopedFD(::dup(fd)));
        bufferHandle.planes.push_back(std::move(plane));
    }

    return bufferHandle;
}

std::string EGLHelper::getDrmRenderNodeFilePath(const char *extensions) const
{
    const char *kDefaultPath = "/dev/dri/renderD128";

    if (!strstr(extensions, "EGL_EXT_device_base")) {
        qWarning("EGL: EGL_EXT_device_base extension is not supported. Fallback to %s.",
                 kDefaultPath);
        return kDefaultPath;
    }

    EGLDeviceEXT eglDevice = EGL_NO_DEVICE_EXT;

    // Try to get the EGLDevice from the current EGLDisplay.
    if (m_eglDisplay != EGL_NO_DISPLAY && strstr(extensions, "EGL_EXT_device_query")) {
        EGLAttrib attrib;
        if (m_functions->eglQueryDisplayAttrib(m_eglDisplay, EGL_DEVICE_EXT, &attrib))
            eglDevice = reinterpret_cast<EGLDeviceEXT>(attrib);
    }

    // Try to get the first available EGLDevice if query from EGLDisplay failed.
    if (eglDevice == EGL_NO_DEVICE_EXT && strstr(extensions, "EGL_EXT_device_enumeration")) {
        EGLint numDevices = 0;
        m_functions->eglQueryDevices(1, &eglDevice, &numDevices);
    }

    if (eglDevice == EGL_NO_DEVICE_EXT) {
        qWarning("EGL: Could not find EGL device. Fallback to %s.", kDefaultPath);
        return kDefaultPath;
    }

    const char *deviceExtensions = m_functions->eglQueryDeviceString(eglDevice, EGL_EXTENSIONS);

    if (Q_UNLIKELY(QtWebEngineCore::lcWebEngineCompositor().isDebugEnabled())) {
        const char *renderer = nullptr;
        if (strstr(deviceExtensions, "EGL_EXT_device_query_name"))
            renderer = m_functions->eglQueryDeviceString(eglDevice, EGL_RENDERER_EXT);

        if (renderer)
            qCDebug(QtWebEngineCore::lcWebEngineCompositor, "EGL: Device found: %s.", renderer);
        else
            qCDebug(QtWebEngineCore::lcWebEngineCompositor, "EGL: Failed to query device.");
    }

    if (!strstr(deviceExtensions, "EGL_EXT_device_drm_render_node")) {
        qWarning("EGL: EGL_EXT_device_drm_render_node extension is not supported. Fallback to %s.",
                 kDefaultPath);
        return kDefaultPath;
    }

    const char *path = m_functions->eglQueryDeviceString(eglDevice, EGL_DRM_RENDER_NODE_FILE_EXT);
    if (!path) {
        qWarning("EGL: Failed to query DRM render node file path. Fallback to %s.", kDefaultPath);
        return kDefaultPath;
    }

    return path;
}

const char *EGLHelper::getLastEGLErrorString() const
{
    return getEGLErrorString(m_functions->eglGetError());
}

QT_END_NAMESPACE
