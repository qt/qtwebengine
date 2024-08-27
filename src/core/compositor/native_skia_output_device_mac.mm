// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// This is a workaround to be able to include Qt headers without
// "redefinition of 'NSString' as different kind of symbol" errors.
// TODO: Remove this when namespace ambiguity issues are fixed properly,
// see get_forward_declaration_macro() in cmake/Functions.cmake
#undef Q_FORWARD_DECLARE_OBJC_CLASS

#import <AppKit/AppKit.h>
#import <IOSurface/IOSurface.h>
#import <Metal/Metal.h>

#include <QtGui/qtguiglobal.h>
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

namespace QtWebEngineCore {

QSGTexture *makeMetalTexture(QQuickWindow *win, IOSurfaceRef ioSurface, uint ioSurfacePlane,
                             const QSize &size, QQuickWindow::CreateTextureOptions texOpts)
{
    auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                   width:size.width()
                                                                  height:size.height()
                                                               mipmapped:false];

    QSGRendererInterface *ri = win->rendererInterface();
    auto device = (__bridge id<MTLDevice>)(ri->getResource(win, QSGRendererInterface::DeviceResource));
    id<MTLTexture> texture = [device newTextureWithDescriptor:desc
                                                    iosurface:ioSurface
                                                        plane:ioSurfacePlane];
    return QNativeInterface::QSGMetalTexture::fromNative(texture, win, size, texOpts);
}

void releaseMetalTexture(void *texture)
{
    [static_cast<id<MTLTexture>>(texture) release];
}

#if QT_CONFIG(opengl)
uint32_t makeCGLTexture(QQuickWindow *win, IOSurfaceRef ioSurface, const QSize &size)
{
    const int width = size.width();
    const int height = size.height();

    auto glContext = QOpenGLContext::currentContext();
    auto glFun = glContext->extraFunctions();
    auto nscontext = glContext->nativeInterface<QNativeInterface::QCocoaGLContext>()->nativeContext();
    CGLContextObj cglContext = [nscontext CGLContextObj];

    win->beginExternalCommands();
    // Bind the IO surface to a texture
    GLuint glTexture;
    glFun->glGenTextures(1, &glTexture);
    glFun->glBindTexture(GL_TEXTURE_RECTANGLE_ARB, glTexture);
    CGLTexImageIOSurface2D(cglContext, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA, width, height, GL_BGRA,
                           GL_UNSIGNED_INT_8_8_8_8_REV, ioSurface, 0);
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

    return glTexture;
}
#endif // QT_CONFIG(opengl)

} // namespace
