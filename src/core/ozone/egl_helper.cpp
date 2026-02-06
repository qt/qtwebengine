// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "egl_helper.h"

#include "compositor/compositor.h"
#include "ozone_util_qt.h"
#include "rhi_gpu_info.h"

#include "ui/gfx/linux/drm_util_linux.h"
#include "ui/gfx/linux/gbm_buffer.h"
#include "ui/gfx/linux/gbm_device.h"
#include "ui/gfx/linux/gbm_util.h"
#include "ui/gfx/linux/gbm_wrapper.h"
#include "ui/ozone/platform/wayland/common/drm_render_node_handle.h"

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

    // clang-format off
    eglCreateImage = reinterpret_cast<PFNEGLCREATEIMAGEPROC>(
            context->getProcAddress("eglCreateImage"));
    eglDestroyImage = reinterpret_cast<PFNEGLDESTROYIMAGEPROC>(
            context->getProcAddress("eglDestroyImage"));
    eglQueryDevices = reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(
            context->getProcAddress("eglQueryDevicesEXT"));
    eglQueryDeviceString = reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(
            context->getProcAddress("eglQueryDeviceStringEXT"));
    eglQueryDisplayAttrib = reinterpret_cast<PFNEGLQUERYDISPLAYATTRIBEXTPROC>(
            context->getProcAddress("eglQueryDisplayAttribEXT"));
    eglQueryString = reinterpret_cast<PFNEGLQUERYSTRINGPROC>(
            context->getProcAddress("eglQueryString"));

    eglGetCurrentContext = reinterpret_cast<PFNEGLGETCURRENTCONTEXTPROC>(
            context->getProcAddress("eglGetCurrentContext"));
    eglGetCurrentDisplay = reinterpret_cast<PFNEGLGETCURRENTDISPLAYPROC>(
            context->getProcAddress("eglGetCurrentDisplay"));
    eglGetCurrentSurface = reinterpret_cast<PFNEGLGETCURRENTSURFACEPROC>(
            context->getProcAddress("eglGetCurrentSurface"));
    eglGetError = reinterpret_cast<PFNEGLGETERRORPROC>(
            context->getProcAddress("eglGetError"));
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
{
    // Expected to be created on the UI thread.
    Q_ASSERT(QThread::currentThread() == qApp->thread());
    QMutexLocker locker(&m_mutex);

    const char *extensions = m_functions->eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!extensions) {
        qWarning("EGL: Failed to query EGL extensions.");
        return;
    }

    if (m_eglDisplay == EGL_NO_DISPLAY) {
        qWarning("EGL: No EGL display.");
        return;
    }

    const char *displayExtensions = m_functions->eglQueryString(m_eglDisplay, EGL_EXTENSIONS);
    if (!strstr(displayExtensions, "EGL_EXT_image_dma_buf_import")) {
        qWarning("EGL: EGL_EXT_image_dma_buf_import extension is not supported.");
        return;
    }

    // Enable DMA-BUF if GBM and all the required extensions are supported.
    m_isDmaBufSupported = QtWebEngineCore::RhiGpuInfo::instance()->isGbmSupported();
    if (!m_isDmaBufSupported)
        return;

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
        // It is safe to initialize the GBM device on the UI thread and allocate buffers on the GPU
        // thread because m_mutex serializes access and guarantess the device is fully initialized
        // before allocation begins.
        m_gbmDevice.reset(new ScopedGbmDevice(nodePath));
    }

    // Create an offscreen surface on the GUI thread for EGL-based allocation fallback.
    m_isImageDmaBufExportSupported = strstr(displayExtensions, "EGL_MESA_image_dma_buf_export");
    if (m_isImageDmaBufExportSupported) {
        m_offscreenSurface.reset(new QOffscreenSurface());
        // Surface must be created on the UI thread.
        m_offscreenSurface->create();
    } else
        qCDebug(QtWebEngineCore::lcWebEngineCompositor,
                "EGL: EGL_MESA_image_dma_buf_export extension is not supported. EGL-based "
                "allocation fallback is disabled.");
}

std::unique_ptr<ui::GbmBuffer> EGLHelper::createBuffer(gfx::BufferFormat format, gfx::Size size,
                                                       gfx::BufferUsage usage)
{
    QMutexLocker locker(&m_mutex);
    if (!m_isDmaBufSupported || !m_gbmDevice || !m_gbmDevice->device)
        return nullptr;

    const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
    const uint32_t gbmFlags = ui::BufferUsageToGbmFlags(usage);
    // FIXME: CreateBufferWithModifiers for wayland?
    return m_gbmDevice->device->CreateBuffer(fourccFormat, size, gbmFlags);
}

std::unique_ptr<ui::GbmBuffer> EGLHelper::createBufferFromHandle(gfx::Size size,
                                                                 gfx::BufferFormat format,
                                                                 gfx::NativePixmapHandle handle)
{
    QMutexLocker locker(&m_mutex);
    if (!m_isDmaBufSupported || !m_gbmDevice || !m_gbmDevice->device)
        return nullptr;

    const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
    return m_gbmDevice->device->CreateBufferFromHandle(fourccFormat, size, std::move(handle));
}

gfx::NativePixmapHandle EGLHelper::exportHandleFromEGLImage(const gfx::Size &size)
{
    QMutexLocker locker(&m_mutex);
    if (!m_isDmaBufSupported || !m_isImageDmaBufExportSupported)
        return gfx::NativePixmapHandle();

    ScopedGLContext glContext(m_offscreenSurface.get(), m_functions.get());
    if (!glContext.isValid()) {
        qWarning("EGL: Failed to create valid GL context.");
        return gfx::NativePixmapHandle();
    }

    EGLContext eglContext = glContext.eglContext();
    if (!eglContext) {
        qWarning("EGL: No EGLContext.");
        return gfx::NativePixmapHandle();
    }

    uint64_t textureId = glContext.createTexture(size.width(), size.height());
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

gfx::NativePixmapHandle EGLHelper::exportHandleFromEGLImage(const gfx::Size &size,
                                                            gfx::BufferFormat format,
                                                            gfx::NativePixmapHandle handle)
{
    QMutexLocker locker(&m_mutex);
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
        if (strstr(deviceExtensions, "EGL_EXT_device_query_name")) {
            const char *renderer = m_functions->eglQueryDeviceString(eglDevice, EGL_RENDERER_EXT);
            qCDebug(QtWebEngineCore::lcWebEngineCompositor, "EGL: Device found: %s.",
                    renderer ? renderer : "FAILED!");
        }
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

EGLHelper::ScopedGbmDevice::ScopedGbmDevice(const std::string &nodePath)
{
    ui::DrmRenderNodeHandle nodeHandle;
    if (!nodeHandle.Initialize(base::FilePath(nodePath))) {
        qWarning("EGL: Failed to initialize DRM render node handle: %s\n", nodePath.data());
        return;
    }

    nodeFD = nodeHandle.PassFD();
    device = ui::CreateGbmDevice(nodeFD.get());
    if (!device)
        qWarning("EGL: Failed to initialize GBM device.");
}

QT_END_NAMESPACE
