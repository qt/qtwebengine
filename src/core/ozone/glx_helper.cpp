// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtGui/qguiapplication.h>
#include <QtGui/qopenglcontext.h>
#include <qpa/qplatformnativeinterface.h>

#include "glx_helper.h"
#include "ozone_util_qt.h"
#include "web_engine_context.h"

#include "ui/gfx/linux/gpu_memory_buffer_support_x11.h"

#include <unistd.h>
#include <xcb/dri3.h>
#include <xcb/xcb.h>
#include <xcb/xcbext.h>

QT_BEGIN_NAMESPACE

GLXHelper::GLXFunctions::GLXFunctions()
{
    QOpenGLContext *context = OzoneUtilQt::getQOpenGLContext();

    glXBindTexImageEXT = reinterpret_cast<PFNGLXBINDTEXIMAGEEXTPROC>(
            context->getProcAddress("glXBindTexImageEXT"));
    glXReleaseTexImageEXT = reinterpret_cast<PFNGLXRELEASETEXIMAGEEXTPROC>(
            context->getProcAddress("glXReleaseTexImageEXT"));
}

GLXHelper *GLXHelper::instance()
{
    static GLXHelper glxHelper;
    return &glxHelper;
}

GLXHelper::GLXHelper() : m_functions(new GLXHelper::GLXFunctions())
{
    auto *x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!x11Application)
        qFatal("GLX: No X11 Application.");

    m_display = x11Application->display();
    m_connection = x11Application->connection();

    m_isDmaBufSupported = QtWebEngineCore::WebEngineContext::isGbmSupported()
            && ui::GpuMemoryBufferSupportX11::GetInstance()->has_gbm_device();
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
    // Hard coded values for gfx::BufferFormat::BGRA_8888:
    const uint8_t depth = 32;
    const uint8_t bpp = 32;

    const uint32_t pixmapId = xcb_generate_id(m_connection);
    if (!pixmapId) {
        qWarning("GLX: Failed to allocate XID for XPixmap.");
        close(dmaBufFd);
        return 0;
    }

    const xcb_setup_t *setup = xcb_get_setup(m_connection);
    xcb_screen_t *screen = xcb_setup_roots_iterator(setup).data;

    // This call is supposed to close dmaBufFd.
    xcb_void_cookie_t cookie =
            xcb_dri3_pixmap_from_buffer_checked(m_connection, pixmapId, screen->root, size, width,
                                                height, stride, depth, bpp, dmaBufFd);
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

QT_END_NAMESPACE
