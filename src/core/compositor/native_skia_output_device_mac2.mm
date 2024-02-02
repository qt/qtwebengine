// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgrendererinterface.h>
#include <QtQuick/qsgtexture.h>

#if QT_CONFIG(opengl)
#include <OpenGL/OpenGL.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglextrafunctions.h>
#include <QtOpenGL/qopengltextureblitter.h>
#include <QtOpenGL/qopenglframebufferobject.h>
#endif

@class MTLDevice;
@protocol MTLTexture;

namespace QtWebEngineCore {

MTLDevice *getRhiDev(QQuickWindow *win)
{
    QSGRendererInterface *ri = win->rendererInterface();
    return static_cast<MTLDevice *>(ri->getResource(win, QSGRendererInterface::DeviceResource));
}

QSGTexture *makeMetalTexture2(QQuickWindow *win, id<MTLTexture> mtlTexture, int width, int height, uint32_t textureOptions)
{
    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    return QNativeInterface::QSGMetalTexture::fromNative(mtlTexture, win, {width, height}, texOpts);
}

#if QT_CONFIG(opengl)
CGLContextObj getCGLContext(NSOpenGLContext *context);

void releaseGlTexture(uint32_t glTexture)
{
    auto *glContext = QOpenGLContext::currentContext();
    if (!glContext)
        return;
    auto glFun = glContext->functions();
    glFun->glDeleteTextures(1, &glTexture);
}

QSGTexture *makeCGLTexture(QQuickWindow *win, IOSurfaceRef io_surface, int width, int height, uint32_t textureOptions, uint32_t *heldTexture)
{
    auto glContext = QOpenGLContext::currentContext();
    auto glFun = glContext->extraFunctions();
    auto nscontext = glContext->nativeInterface<QNativeInterface::QCocoaGLContext>()->nativeContext();
    CGLContextObj cglContext = getCGLContext(nscontext);

    win->beginExternalCommands();
    // Bind the IO surface to a texture
    GLuint glTexture;
    glFun->glGenTextures(1, &glTexture);
    glFun->glBindTexture(GL_TEXTURE_RECTANGLE_ARB, glTexture);
    CGLTexImageIOSurface2D(cglContext, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, io_surface, 0);
    glFun->glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    glFun->glViewport(0, 0, width, height);

    // The bound IO surface is a weird dynamic bind, so take a snapshot of it to a normal texture
    {
        QOpenGLFramebufferObject fbo(width, height, GL_TEXTURE_2D);
        auto success = fbo.bind();
        Q_ASSERT(success);

        QOpenGLTextureBlitter blitter;
        success = blitter.create();
        Q_ASSERT(success);
        glFun->glDisable(GL_BLEND);
        glFun->glDisable(GL_SCISSOR_TEST);
        blitter.bind(GL_TEXTURE_RECTANGLE_ARB);
        blitter.blit(glTexture, {}, QOpenGLTextureBlitter::OriginBottomLeft);
        blitter.release();
        blitter.destroy();

        glFun->glDeleteTextures(1, &glTexture);
        glTexture = fbo.takeTexture();
        fbo.release();
    }
    win->endExternalCommands();

    *heldTexture = glTexture;
    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    return QNativeInterface::QSGOpenGLTexture::fromNative(glTexture, win, {width, height}, texOpts);
}
#endif

} // namespace
