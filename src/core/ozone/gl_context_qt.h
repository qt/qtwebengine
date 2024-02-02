// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GL_GL_CONTEXT_QT_H_
#define GL_GL_CONTEXT_QT_H_

#include <QObject>
#include "ui/gl/gl_context.h"

namespace gl {
class GLSurface;
}

QT_BEGIN_NAMESPACE

class GLContextHelper : public QObject {
    Q_OBJECT
public:
    static void initialize();
    static void destroy();
    static bool initializeContext(gl::GLContext* context, gl::GLSurface* surface, gl::GLContextAttribs attribs);

    static void* getEGLConfig();
    static void* getGlXConfig();
    static void* getEGLDisplay();
    static void* getXDisplay();
    static void* getNativeDisplay();
    static QFunctionPointer getGlXGetProcAddress();
    static QFunctionPointer getEglGetProcAddress();
    static bool isCreateContextRobustnessSupported();
    static void *getGlxPlatformInterface();
    static void *getEglPlatformInterface();

private:
    Q_INVOKABLE bool initializeContextOnBrowserThread(gl::GLContext* context, gl::GLSurface* surface, gl::GLContextAttribs attribs);

    static GLContextHelper* contextHelper;
    bool m_robustness = false;
};

QT_END_NAMESPACE

#endif

