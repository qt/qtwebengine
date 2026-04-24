// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "glx_helper.h"

#include "compositor/compositor.h"
#include "ozone/gbm_buffer_factory.h"
#include "ozone/ozone_util_qt.h"
#include "rhi_gpu_info.h"

#include "base/files/scoped_file.h"
#include "base/posix/eintr_wrapper.h"

#include <QtGui/qguiapplication.h>
#include <QtGui/qopenglcontext.h>
#include <qpa/qplatformnativeinterface.h>

#include <algorithm>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <GL/glx.h>
#include <mutex>
#include <unistd.h>
#include <xcb/dri3.h>
#include <xcb/xcb.h>
#include <xcb/xcbext.h>
#include <X11/Xlib.h>

QT_BEGIN_NAMESPACE

// Hard coded values for gfx::BufferFormat::BGRA_8888
const uint8_t kDepth = 32; // Same as NativePixmapEGLX11Binding::Depth()
const uint8_t kBpp = 32; // Same as NativePixmapEGLX11Binding::Bpp()

GLXHelper::GLXFunctions::GLXFunctions()
{
    QOpenGLContext *context = OzoneUtilQt::getQOpenGLContext();

    glXBindTexImageEXT = reinterpret_cast<PFNGLXBINDTEXIMAGEEXTPROC>(
            context->getProcAddress("glXBindTexImageEXT"));
    glXReleaseTexImageEXT = reinterpret_cast<PFNGLXRELEASETEXIMAGEEXTPROC>(
            context->getProcAddress("glXReleaseTexImageEXT"));
    glXQueryRendererStringMESA = reinterpret_cast<PFNGLXQUERYRENDERERSTRINGMESAPROC>(
            context->getProcAddress("glXQueryRendererStringMESA"));
}

GLXHelper *GLXHelper::instance()
{
    static GLXHelper glxHelper;
    return &glxHelper;
}

GLXHelper::GLXHelper()
    : m_functions(new GLXHelper::GLXFunctions())
    , m_isDmaBufSupported(QtWebEngineCore::RhiGpuInfo::instance()->isGbmSupported())
{
    // The rest of the initialization required for DMA-BUF support.
    if (!m_isDmaBufSupported)
        return;

    auto *x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!x11Application) {
        qWarning("GLX: No X11 Application.");
        m_isDmaBufSupported = false;
        return;
    }

    m_display = x11Application->display();
    if (!m_display) {
        qWarning("GLX: No X11 Display.");
        m_isDmaBufSupported = false;
        return;
    }

    m_connection = x11Application->connection();
    if (!m_connection) {
        qWarning("GLX: No XCB Connection.");
        m_isDmaBufSupported = false;
        return;
    }

    if (int error = xcb_connection_has_error(m_connection)) {
        qWarning("GLX: XCB Connection error: 0x%x", error);
        m_isDmaBufSupported = false;
        return;
    }

    const xcb_query_extension_reply_t *dri3Ext = xcb_get_extension_data(m_connection, &xcb_dri3_id);
    if (!dri3Ext || !dri3Ext->present) {
        qWarning("GLX: No DRI3 Extension.");
        m_isDmaBufSupported = false;
        return;
    }

    uint32_t driMajorVersion;
    uint32_t driMinorVersion;
    if (dri3Version(&driMajorVersion, &driMinorVersion)) {
        qCDebug(QtWebEngineCore::lcWebEngineCompositor, "GLX: DRI3 Version: %u.%u", driMajorVersion,
                driMinorVersion);
    }

    const xcb_setup_t *setup = xcb_get_setup(m_connection);
    Q_ASSERT(setup);
    m_screen = xcb_setup_roots_iterator(setup).data;
    Q_ASSERT(m_screen);

    if (Q_UNLIKELY(QtWebEngineCore::lcWebEngineCompositor().isDebugEnabled())) {
        const char *renderer = nullptr;
        if (m_functions->glXQueryRendererStringMESA) {
            renderer = m_functions->glXQueryRendererStringMESA(m_display, 0, 0,
                                                               GLX_RENDERER_DEVICE_ID_MESA);
        }

        if (renderer)
            qCDebug(QtWebEngineCore::lcWebEngineCompositor, "GLX: Device found: %s.", renderer);
        else
            qCDebug(QtWebEngineCore::lcWebEngineCompositor, "GLX: Failed to query device.");
    }

    // Obtain an authenticated DRM FD.
    base::ScopedFD drmNodeFD(dri3Open());
    if (!drmNodeFD.is_valid()) {
        qWarning("GLX: Failed to obtain a valid DRM file descriptor.");
        m_isDmaBufSupported = false;
        return;
    }

    m_gbmBufferFactory.reset(new GbmBufferFactory(std::move(drmNodeFD)));
    m_isDmaBufSupported = m_gbmBufferFactory->hasDevice();
}

GLXHelper::~GLXHelper()
{
    XFree(m_configs);
}

bool GLXHelper::canCreateNativePixmapForFormat(gfx::BufferFormat format) const
{
    // Currently limited to BGRA_8888 and RGBA_8888 for compatibility.
    // This is consistent with the hardcoded constraints in NativePixmapEGLX11Binding.
    if (format != gfx::BufferFormat::BGRA_8888 && format != gfx::BufferFormat::RGBA_8888)
        return false;

    return m_gbmBufferFactory->canCreateNativePixmapForFormat(format);
}

GLXFBConfig GLXHelper::getFBConfig()
{
    if (m_configs)
        return m_configs[0];

    // clang-format off
    static const int configAttribs[] = {
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_BUFFER_SIZE, 32,
        GLX_BIND_TO_TEXTURE_RGBA_EXT, 1,
        GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT,
        GLX_BIND_TO_TEXTURE_TARGETS_EXT, GLX_TEXTURE_2D_BIT_EXT,
        GLX_DOUBLEBUFFER, 0,
        GLX_Y_INVERTED_EXT, static_cast<int>(GLX_DONT_CARE),
        0
    };
    // clang-format on

    if (Q_UNLIKELY(!m_isDmaBufSupported)) {
        qWarning("GLX: Frame buffer configuration is not expected to be used without dma-buf "
                 "support.");
    }

    int numConfigs = 0;
    m_configs = glXChooseFBConfig(m_display, /* screen */ 0, configAttribs, &numConfigs);
    if (!m_configs || numConfigs < 1)
        qFatal("GLX: Failed to find frame buffer configuration.");

    return m_configs[0];
}

GLXPixmap GLXHelper::importBufferAsPixmap(int dmaBufFd, uint32_t size, uint16_t width,
                                          uint16_t height, uint16_t stride) const
{
    const uint32_t pixmapId = xcb_generate_id(m_connection);
    if (!pixmapId) {
        qWarning("GLX: Failed to allocate XID for XPixmap.");
        close(dmaBufFd);
        return 0;
    }

    // This call is supposed to close dmaBufFd.
    xcb_void_cookie_t cookie =
            xcb_dri3_pixmap_from_buffer_checked(m_connection, pixmapId, m_screen->root, size, width,
                                                height, stride, kDepth, kBpp, dmaBufFd);
    xcb_generic_error_t *error = xcb_request_check(m_connection, cookie);
    if (error) {
        qWarning("GLX: XCB_DRI3_PIXMAP_FROM_BUFFER failed with error code: 0x%x",
                 error->error_code);
        free(error);
        return 0;
    }

    return pixmapId;
}

void GLXHelper::freePixmap(uint32_t pixmapId) const
{
    xcb_void_cookie_t cookie = xcb_free_pixmap_checked(m_connection, pixmapId);
    xcb_generic_error_t *error = xcb_request_check(m_connection, cookie);
    if (error) {
        qWarning("GLX: XCB_FREE_PIXMAP failed with error code: 0x%x", error->error_code);
        free(error);
    }
}

const std::vector<uint64_t> &GLXHelper::getSupportedModifiers() const
{
    static std::vector<uint64_t> supportedModifiers;
    static std::once_flag flag;

    std::call_once(flag, [this]() {
        const xcb_setup_t *setup = xcb_get_setup(m_connection);
        xcb_screen_t *screen = xcb_setup_roots_iterator(setup).data;

        // The DRI3 query is currently locked to a 32-bit depth/BPP to match
        // gfx::BufferFormat::BGRA_8888. This is consistent with the hardcoded
        // constraints in NativePixmapEGLX11Binding.
        xcb_dri3_get_supported_modifiers_cookie_t cookie =
                xcb_dri3_get_supported_modifiers(m_connection, screen->root, kDepth, kBpp);

        xcb_generic_error_t *error = nullptr;
        xcb_dri3_get_supported_modifiers_reply_t *reply =
                xcb_dri3_get_supported_modifiers_reply(m_connection, cookie, &error);
        if (error) {
            qWarning("GLX: XCB_DRI3_GET_SUPPORTED_MODIFIERS failed with error code: 0x%x",
                     error->error_code);
            free(error);
            return;
        }

        uint64_t *windowModifiers = xcb_dri3_get_supported_modifiers_window_modifiers(reply);
        for (size_t i = 0; i < reply->num_window_modifiers; ++i)
            supportedModifiers.push_back(windowModifiers[i]);
        uint64_t *screenModifiers = xcb_dri3_get_supported_modifiers_screen_modifiers(reply);
        for (size_t i = 0; i < reply->num_screen_modifiers; ++i)
            supportedModifiers.push_back(screenModifiers[i]);
        free(reply);

        // Remove invalid modifier.
        supportedModifiers.erase(std::remove(supportedModifiers.begin(), supportedModifiers.end(),
                                             DRM_FORMAT_MOD_INVALID),
                                 supportedModifiers.end());

        // Remove multi-planar modifiers.
        supportedModifiers.erase(
                std::remove_if(supportedModifiers.begin(), supportedModifiers.end(),
                               [this](uint64_t modifier) {
                                   return !m_gbmBufferFactory->isSinglePlanar(DRM_FORMAT_ARGB8888,
                                                                              modifier);
                               }),
                supportedModifiers.end());

        if (supportedModifiers.empty())
            return;

        // Remove duplicates.
        std::sort(supportedModifiers.begin(), supportedModifiers.end());
        supportedModifiers.erase(std::unique(supportedModifiers.begin(), supportedModifiers.end()),
                                 supportedModifiers.end());
    });

    return supportedModifiers;
}

bool GLXHelper::dri3Version(uint32_t *major, uint32_t *minor) const
{
    xcb_dri3_query_version_cookie_t cookie = xcb_dri3_query_version(m_connection, 1, 2);

    xcb_generic_error_t *error = nullptr;
    xcb_dri3_query_version_reply_t *reply =
            xcb_dri3_query_version_reply(m_connection, cookie, &error);
    if (error) {
        qWarning("GLX: XCB_DRI3_QUERY_VERSION failed with error code: 0x%x", error->error_code);
        free(error);
        return false;
    }

    if (!reply) {
        qWarning("GLX: XCB_DRI3_QUERY_VERSION failed.");
        return false;
    }

    *major = reply->major_version;
    *minor = reply->minor_version;

    free(reply);
    return true;
}

// Based on CreateX11GbmDevice() in //ui/gfx/linux/gpu_memory_buffer_support_x11.cc
int GLXHelper::dri3Open() const
{
    xcb_dri3_open_cookie_t cookie = xcb_dri3_open(m_connection, m_screen->root, 0);

    xcb_generic_error_t *error = nullptr;
    xcb_dri3_open_reply_t *reply = xcb_dri3_open_reply(m_connection, cookie, &error);
    if (error) {
        qWarning("GLX: XCB_DRI3_OPEN failed with error code: 0x%x", error->error_code);
        free(error);
        return -1;
    }

    if (!reply) {
        qWarning("GLX: XCB_DRI3_OPEN failed.");
        return -1;
    }

    if (reply->nfd != 1) {
        qWarning("GLX: XCB_DRI3_OPEN reply expected to contain 1 file descriptor, received %u.",
                 reply->nfd);
        free(reply);
        return -1;
    }

    int fd = xcb_dri3_open_reply_fds(m_connection, reply)[0];
    free(reply);

    // Prevent the file descriptor from being inherited by child processes, such as render process.
    if (HANDLE_EINTR(fcntl(fd, F_SETFD, FD_CLOEXEC)) == -1)
        qWarning("GLX: Failed to set CLOEXEC on DRM FD.");

    return fd;
}

QT_END_NAMESPACE
