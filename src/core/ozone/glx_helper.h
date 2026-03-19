// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef GLX_HELPER_H
#define GLX_HELPER_H

#include <QtCore/qscopedpointer.h>

#include "ui/gfx/buffer_types.h"

#include <vector>

struct xcb_connection_t;
typedef struct xcb_connection_t xcb_connection_t;

struct xcb_screen_t;
typedef struct xcb_screen_t xcb_screen_t;

struct _XDisplay;
typedef struct _XDisplay Display;

typedef unsigned long XID;
typedef XID GLXPixmap;
typedef XID GLXDrawable;

struct __GLXFBConfigRec;
typedef struct __GLXFBConfigRec *GLXFBConfig;

typedef void (*PFNGLXBINDTEXIMAGEEXTPROC)(Display *dpy, GLXDrawable drawable, int buffer,
                                          const int *attrib_list);
typedef void (*PFNGLXRELEASETEXIMAGEEXTPROC)(Display *dpy, GLXDrawable drawable, int buffer);
typedef const char *(*PFNGLXQUERYRENDERERSTRINGMESAPROC)(Display *dpy, int screen, int renderer,
                                                         int attribute);

QT_BEGIN_NAMESPACE

class GbmBufferFactory;

class GLXHelper
{
public:
    struct GLXFunctions
    {
        GLXFunctions();

        PFNGLXBINDTEXIMAGEEXTPROC glXBindTexImageEXT;
        PFNGLXRELEASETEXIMAGEEXTPROC glXReleaseTexImageEXT;
        PFNGLXQUERYRENDERERSTRINGMESAPROC glXQueryRendererStringMESA;
    };

    static GLXHelper *instance();

    ~GLXHelper();

    Display *getXDisplay() const { return m_display; }
    GLXFunctions *functions() const { return m_functions.get(); }
    bool isDmaBufSupported() const { return m_isDmaBufSupported; }
    GbmBufferFactory *gbmFactory() const { return m_gbmBufferFactory.get(); }
    bool canCreateNativePixmapForFormat(gfx::BufferFormat format) const;

    GLXFBConfig getFBConfig();
    GLXPixmap importBufferAsPixmap(int dmaBufFd, uint32_t size, uint16_t width, uint16_t height,
                                   uint16_t stride) const;
    void freePixmap(uint32_t pixmapId) const;
    const std::vector<uint64_t> &getSupportedModifiers() const;

private:
    GLXHelper();
    bool dri3Version(uint32_t *major, uint32_t *minor) const;
    int dri3Open() const;

    QScopedPointer<GLXFunctions> m_functions;
    bool m_isDmaBufSupported = false;

    Display *m_display = nullptr;
    xcb_connection_t *m_connection = nullptr;
    xcb_screen_t *m_screen = nullptr;

    QScopedPointer<GbmBufferFactory> m_gbmBufferFactory;
    GLXFBConfig *m_configs = nullptr;
};

QT_END_NAMESPACE

#endif // GLX_HELPER_H
