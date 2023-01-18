/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "content_gpu_client_qt.h"
#include "web_engine_context.h"
#include "ui/gl/gl_share_group.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gpu_timing.h"

#if QT_CONFIG(opengl)
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#endif

#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>

QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
QT_END_NAMESPACE

namespace QtWebEngineCore {

class QtShareGLContext : public gl::GLContext
{
public:
    QtShareGLContext(QOpenGLContext *qtContext) : gl::GLContext(0), m_handle(0)
    {
        QString platform = qApp->platformName().toLower();
        QPlatformNativeInterface *pni = QGuiApplication::platformNativeInterface();
        if (platform == QLatin1String("xcb") || platform == QLatin1String("offscreen")) {
            if (gl::GetGLImplementation() == gl::kGLImplementationEGLGLES2)
                m_handle =
                        pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
            else
                m_handle =
                        pni->nativeResourceForContext(QByteArrayLiteral("glxcontext"), qtContext);
        } else if (platform == QLatin1String("cocoa"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("cglcontextobj"), qtContext);
        else if (platform == QLatin1String("qnx"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
        else if (platform == QLatin1String("eglfs") || platform == QLatin1String("wayland")
                 || platform == QLatin1String("wayland-egl"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
        else if (platform == QLatin1String("windows")) {
            if (gl::GetGLImplementation() == gl::kGLImplementationEGLGLES2)
                m_handle =
                        pni->nativeResourceForContext(QByteArrayLiteral("eglContext"), qtContext);
            else
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("renderingcontext"),
                                                         qtContext);
        } else {
            qFatal("%s platform not yet supported", platform.toLatin1().constData());
            // Add missing platforms once they work.
            Q_UNREACHABLE();
        }
    }

    void *GetHandle() override { return m_handle; }
    unsigned int CheckStickyGraphicsResetStatusImpl() override
    {
#if QT_CONFIG(opengl)
        if (QOpenGLContext *context = qt_gl_global_share_context()) {
            if (context->format().testOption(QSurfaceFormat::ResetNotification))
                return context->extraFunctions()->glGetGraphicsResetStatus();
        }
#endif
        return 0 /*GL_NO_ERROR*/;
    }

    // We don't care about the rest, this context shouldn't be used except for its handle.
    bool Initialize(gl::GLSurface *, const gl::GLContextAttribs &) override
    {
        Q_UNREACHABLE();
        return false;
    }
    bool MakeCurrentImpl(gl::GLSurface *) override
    {
        Q_UNREACHABLE();
        return false;
    }
    void ReleaseCurrent(gl::GLSurface *) override
    {
        Q_UNREACHABLE();
    }
    bool IsCurrent(gl::GLSurface *) override
    {
        Q_UNREACHABLE();
        return false;
    }
    scoped_refptr<gl::GPUTimingClient> CreateGPUTimingClient() override
    {
        return nullptr;
    }
    const gfx::ExtensionSet &GetExtensions() override
    {
        static const gfx::ExtensionSet s_emptySet;
        return s_emptySet;
    }
    void ResetExtensions() override { }

private:
    void *m_handle;
};

class ShareGroupQtQuick : public gl::GLShareGroup
{
public:
    gl::GLContext *GetContext() override { return m_shareContextQtQuick.get(); }
    void AboutToAddFirstContext() override;

private:
    scoped_refptr<QtShareGLContext> m_shareContextQtQuick;
};

void ShareGroupQtQuick::AboutToAddFirstContext()
{
#if QT_CONFIG(opengl)
    // This currently has to be setup by ::main in all applications using QQuickWebEngineView
    // with de legated rendering.
    QOpenGLContext *shareContext = qt_gl_global_share_context();
    if (!shareContext) {
        qFatal("QWebEngine: OpenGL resource sharing is not set up in QtQuick. Please make sure "
               "to"
               "call QtWebEngine::initialize() in your main() function before QCoreApplication "
               "is "
               "created.");
    }
    m_shareContextQtQuick = new QtShareGLContext(shareContext);
#endif
}

ContentGpuClientQt::ContentGpuClientQt() { }

ContentGpuClientQt::~ContentGpuClientQt() { }

gpu::SyncPointManager *ContentGpuClientQt::GetSyncPointManager()
{
    return WebEngineContext::syncPointManager();
}

gl::GLShareGroup *ContentGpuClientQt::GetInProcessGpuShareGroup()
{
    if (!m_shareGroupQtQuick.get())
        m_shareGroupQtQuick = new ShareGroupQtQuick;
    return m_shareGroupQtQuick.get();
}

} // namespace
