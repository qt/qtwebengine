// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "native_skia_output_device_opengl.h"

#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglextrafunctions.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgtexture.h>

#include "ui/base/ozone_buildflags.h"
#include "ui/gl/gl_implementation.h"

#if defined(USE_OZONE)
#include "base/posix/eintr_wrapper.h"
#include "third_party/skia/include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "ui/gfx/linux/drm_util_linux.h"
#include "ui/gfx/linux/native_pixmap_dmabuf.h"

#if BUILDFLAG(IS_OZONE_X11)
#include "ui/gfx/x/connection.h"
#include "ui/gfx/x/dri3.h"
#include "ui/gfx/x/future.h"
#include "ui/gfx/x/glx.h"
#include "ui/gfx/x/xproto.h"

#include "ui/gl/gl_bindings.h"
#undef glBindTexture
#undef glCreateMemoryObjectsEXT
#undef glDeleteMemoryObjectsEXT
#undef glDeleteTextures
#undef glGenTextures
#undef glGetError
#undef glImportMemoryFdEXT
#undef glTextureStorageMem2DEXT
#endif // BUILDFLAG(IS_OZONE_X11)

#if BUILDFLAG(ENABLE_VULKAN)
#if BUILDFLAG(IS_OZONE_X11)
// We need to define USE_VULKAN_XCB for proper vulkan function pointers.
// Avoiding it may lead to call wrong vulkan functions.
// This is originally defined in chromium/gpu/vulkan/BUILD.gn.
#define USE_VULKAN_XCB
#endif // BUILDFLAG(IS_OZONE_X11)
#include "components/viz/common/gpu/vulkan_context_provider.h"
#include "gpu/vulkan/vulkan_device_queue.h"
#include "gpu/vulkan/vulkan_function_pointers.h"
#include "third_party/skia/include/gpu/vk/GrVkTypes.h"
#include "third_party/skia/include/gpu/ganesh/vk/GrVkBackendSurface.h"
#endif // BUILDFLAG(ENABLE_VULKAN)

// Keep it at the end.
#include "ozone/gl_context_qt.h"
#endif // defined(USE_OZONE)

namespace QtWebEngineCore {

#if BUILDFLAG(IS_OZONE_X11)
namespace {

// Based on //ui/ozone/platform/x11/native_pixmap_egl_x11_binding.cc
x11::Pixmap XPixmapFromNativePixmap(const gfx::NativePixmap &nativePixmap)
{
    // Hard coded values for gfx::BufferFormat::BGRA_8888:
    const uint8_t depth = 32;
    const uint8_t bpp = 32;

    const int dmaBufFd = HANDLE_EINTR(dup(nativePixmap.GetDmaBufFd(0)));
    if (dmaBufFd < 0) {
        qWarning("Could not import the dma-buf as an XPixmap because the FD couldn't be dup()ed.");
        return x11::Pixmap::None;
    }
    x11::RefCountedFD refCountedFD(dmaBufFd);

    uint32_t size = base::checked_cast<uint32_t>(nativePixmap.GetDmaBufPlaneSize(0));
    uint16_t width = base::checked_cast<uint16_t>(nativePixmap.GetBufferSize().width());
    uint16_t height = base::checked_cast<uint16_t>(nativePixmap.GetBufferSize().height());
    uint16_t stride = base::checked_cast<uint16_t>(nativePixmap.GetDmaBufPitch(0));

    auto *connection = x11::Connection::Get();
    const x11::Pixmap pixmapId = connection->GenerateId<x11::Pixmap>();
    if (pixmapId == x11::Pixmap::None) {
        qWarning("Could not import the dma-buf as an XPixmap because an ID couldn't be generated.");
        return x11::Pixmap::None;
    }

    auto response = connection->dri3()
                            .PixmapFromBuffer(pixmapId, connection->default_root(), size, width,
                                              height, stride, depth, bpp, refCountedFD)
                            .Sync();

    if (response.error) {
        qWarning() << "Could not import the dma-buf as an XPixmap because "
                      "PixmapFromBuffer() failed; error: "
                   << response.error->ToString();
        return x11::Pixmap::None;
    }

    return pixmapId;
}

GLXFBConfig GetFBConfig(Display *display)
{
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

    int numConfigs = 0;
    GLXFBConfig *configs = glXChooseFBConfig(display, /* screen */ 0, configAttribs, &numConfigs);
    if (!configs || numConfigs < 1)
        qFatal("GLX: Failed to find frame buffer configuration for pixmap.");

    return configs[0];
}

} // namespace
#endif // BUILDFLAG(IS_OZONE_X11)

NativeSkiaOutputDeviceOpenGL::NativeSkiaOutputDeviceOpenGL(
        scoped_refptr<gpu::SharedContextState> contextState, bool requiresAlpha,
        gpu::MemoryTracker *memoryTracker, viz::SkiaOutputSurfaceDependency *dependency,
        gpu::SharedImageFactory *shared_image_factory,
        gpu::SharedImageRepresentationFactory *shared_image_representation_factory,
        DidSwapBufferCompleteCallback didSwapBufferCompleteCallback)
    : NativeSkiaOutputDevice(contextState, requiresAlpha, memoryTracker, dependency,
                             shared_image_factory, shared_image_representation_factory,
                             didSwapBufferCompleteCallback)
{
    SkColorType skColorType = kRGBA_8888_SkColorType;
#if BUILDFLAG(IS_OZONE_X11)
    if (GLContextHelper::getGlxPlatformInterface()
        && m_contextState->gr_context_type() == gpu::GrContextType::kGL) {
        skColorType = kBGRA_8888_SkColorType;
    }
#endif // BUILDFLAG(IS_OZONE_X11)

    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBA_8888)] = skColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBX_8888)] = skColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRA_8888)] = skColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRX_8888)] = skColorType;
}

NativeSkiaOutputDeviceOpenGL::~NativeSkiaOutputDeviceOpenGL() { }

#if defined(Q_OS_MACOS)
uint32_t makeCGLTexture(QQuickWindow *win, IOSurfaceRef ioSurface, const QSize &size);
#endif

QSGTexture *NativeSkiaOutputDeviceOpenGL::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer || !m_readyWithTexture)
        return nullptr;

#if defined(USE_OZONE)
    scoped_refptr<gfx::NativePixmap> nativePixmap = m_frontBuffer->nativePixmap();
#if BUILDFLAG(ENABLE_VULKAN)
    GrVkImageInfo vkImageInfo;
    if (!nativePixmap) {
        if (m_isNativeBufferSupported) {
            qWarning("No native pixmap.");
            return nullptr;
        }

        sk_sp<SkImage> skImage = m_frontBuffer->skImage();
        if (!skImage) {
            qWarning("No SkImage.");
            return nullptr;
        }

        if (!skImage->isTextureBacked()) {
            qWarning("SkImage is not backed by GPU texture.");
            return nullptr;
        }

        GrBackendTexture backendTexture;
        bool success = SkImages::GetBackendTextureFromImage(skImage, &backendTexture, false);
        if (!success || !backendTexture.isValid()) {
            qWarning("Failed to retrieve backend texture from SkImage.");
            return nullptr;
        }

        if (backendTexture.backend() != GrBackendApi::kVulkan) {
            qWarning("Backend texture is not a Vulkan texture.");
            return nullptr;
        }

        GrBackendTextures::GetVkImageInfo(backendTexture, &vkImageInfo);
        if (vkImageInfo.fAlloc.fMemory == VK_NULL_HANDLE) {
            qWarning("Unable to access Vulkan memory.");
            return nullptr;
        }
    }
#else
    if (!nativePixmap) {
        qWarning("No native pixmap.");
        return nullptr;
    }
#endif // BUILDFLAG(ENABLE_VULKAN)
#elif defined(Q_OS_WIN)
    auto overlayImage = m_frontBuffer->overlayImage();
    if (!overlayImage) {
        qWarning("No overlay image.");
        return nullptr;
    }
#elif defined(Q_OS_MACOS)
    gfx::ScopedIOSurface ioSurface = m_frontBuffer->ioSurface();
    if (!ioSurface) {
        qWarning("No IOSurface.");
        return nullptr;
    }
#endif

    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    QSGTexture *texture = nullptr;

#if defined(USE_OZONE)
    QOpenGLContext *glContext = QOpenGLContext::currentContext();
    auto glFun = glContext->functions();
    GLuint glTexture = 0;

    if (nativePixmap) {
        Q_ASSERT(m_contextState->gr_context_type() == gpu::GrContextType::kGL);

#if BUILDFLAG(IS_OZONE_X11)
        if (GLContextHelper::getGlxPlatformInterface()) {
            x11::Pixmap pixmapId =
                    XPixmapFromNativePixmap(*(gfx::NativePixmapDmaBuf *)nativePixmap.get());
            if (pixmapId == x11::Pixmap::None)
                qFatal("GLX: Failed to import XPixmap.");

            // clang-format off
            static const int pixmapAttribs[] = {
                 GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
                 GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
                 0
            };
            // clang-format on

            Display *display = static_cast<Display *>(GLContextHelper::getXDisplay());
            GLXPixmap glxPixmap = glXCreatePixmap(display, GetFBConfig(display),
                                                  static_cast<::Pixmap>(pixmapId), pixmapAttribs);

            glFun->glGenTextures(1, &glTexture);
            glFun->glBindTexture(GL_TEXTURE_2D, glTexture);
            glXBindTexImageEXT(display, glxPixmap, GLX_FRONT_LEFT_EXT, nullptr);
            glFun->glBindTexture(GL_TEXTURE_2D, 0);

            m_frontBuffer->textureCleanupCallback = [glFun, glTexture, display, glxPixmap]() {
                glFun->glDeleteTextures(1, &glTexture);
                glXDestroyGLXPixmap(display, glxPixmap);
            };
        }
#endif // BUILDFLAG(IS_OZONE_X11)

        if (GLContextHelper::getEglPlatformInterface()) {
            EGLHelper *eglHelper = EGLHelper::instance();
            auto *eglFun = eglHelper->functions();

            const int dmaBufFd = HANDLE_EINTR(dup(nativePixmap->GetDmaBufFd(0)));
            if (dmaBufFd < 0) {
                qFatal("Could not import the dma-buf as an EGLImage because the FD couldn't be "
                       "dup()ed.");
            }
            base::ScopedFD scopedFd(dmaBufFd);

            int drmFormat = ui::GetFourCCFormatFromBufferFormat(nativePixmap->GetBufferFormat());
            uint64_t modifier = nativePixmap->GetBufferFormatModifier();

            // clang-format off
            EGLAttrib const attributeList[] = {
                EGL_WIDTH, size().width(),
                EGL_HEIGHT, size().height(),
                EGL_LINUX_DRM_FOURCC_EXT, drmFormat,
                EGL_DMA_BUF_PLANE0_FD_EXT, scopedFd.get(),
                EGL_DMA_BUF_PLANE0_OFFSET_EXT, static_cast<EGLAttrib>(nativePixmap->GetDmaBufOffset(0)),
                EGL_DMA_BUF_PLANE0_PITCH_EXT, nativePixmap->GetDmaBufPitch(0),
                EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, static_cast<EGLAttrib>(modifier & 0xffffffff),
                EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, static_cast<EGLAttrib>(modifier >> 32),
                EGL_NONE
            };
            // clang-format on
            EGLImage eglImage = eglFun->eglCreateImage(GLContextHelper::getEGLDisplay(),
                                                       EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
                                                       (EGLClientBuffer)NULL, attributeList);
            Q_ASSERT(eglImage != EGL_NO_IMAGE_KHR);

            static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC imageTargetTexture =
                    (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)glContext->getProcAddress(
                            "glEGLImageTargetTexture2DOES");

            glFun->glGenTextures(1, &glTexture);
            glFun->glBindTexture(GL_TEXTURE_2D, glTexture);
            imageTargetTexture(GL_TEXTURE_2D, eglImage);
            glFun->glBindTexture(GL_TEXTURE_2D, 0);

            m_frontBuffer->textureCleanupCallback = [glFun, eglFun, glTexture, eglImage]() {
                glFun->glDeleteTextures(1, &glTexture);
                eglFun->eglDestroyImage(GLContextHelper::getEGLDisplay(), eglImage);
            };
        }
    } else {
#if BUILDFLAG(ENABLE_VULKAN)
        Q_ASSERT(m_contextState->gr_context_type() == gpu::GrContextType::kVulkan);

        gpu::VulkanFunctionPointers *vfp = gpu::GetVulkanFunctionPointers();
        gpu::VulkanDeviceQueue *vulkanDeviceQueue =
                m_contextState->vk_context_provider()->GetDeviceQueue();
        VkDevice vulkanDevice = vulkanDeviceQueue->GetVulkanDevice();

        VkDeviceMemory importedImageMemory = vkImageInfo.fAlloc.fMemory;
        VkDeviceSize importedImageSize = vkImageInfo.fAlloc.fSize;

        VkMemoryGetFdInfoKHR exportInfo = { VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR };
        exportInfo.pNext = nullptr;
        exportInfo.memory = importedImageMemory;
        exportInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

        int fd = -1;
        if (vfp->vkGetMemoryFdKHR(vulkanDevice, &exportInfo, &fd) != VK_SUCCESS)
            qFatal("VULKAN: Unable to extract file descriptor out of external VkImage.");

        static PFNGLCREATEMEMORYOBJECTSEXTPROC glCreateMemoryObjectsEXT =
                (PFNGLCREATEMEMORYOBJECTSEXTPROC)glContext->getProcAddress(
                        "glCreateMemoryObjectsEXT");
        static PFNGLIMPORTMEMORYFDEXTPROC glImportMemoryFdEXT =
                (PFNGLIMPORTMEMORYFDEXTPROC)glContext->getProcAddress("glImportMemoryFdEXT");
        static PFNGLTEXTURESTORAGEMEM2DEXTPROC glTextureStorageMem2DEXT =
                (PFNGLTEXTURESTORAGEMEM2DEXTPROC)glContext->getProcAddress(
                        "glTextureStorageMem2DEXT");

        GLuint glMemoryObject;
        glFun->glGenTextures(1, &glTexture);
        glFun->glBindTexture(GL_TEXTURE_2D, glTexture);
        glCreateMemoryObjectsEXT(1, &glMemoryObject);
        glImportMemoryFdEXT(glMemoryObject, importedImageSize, GL_HANDLE_TYPE_OPAQUE_FD_EXT, fd);
        glTextureStorageMem2DEXT(glTexture, 1, GL_RGBA8_OES, size().width(), size().height(),
                                 glMemoryObject, 0);
        glFun->glBindTexture(GL_TEXTURE_2D, 0);

        m_frontBuffer->textureCleanupCallback = [glTexture, glMemoryObject]() {
            QOpenGLContext *glContext = QOpenGLContext::currentContext();
            if (!glContext)
                return;
            auto glFun = glContext->functions();
            Q_ASSERT(glFun->glGetError() == GL_NO_ERROR);

            static PFNGLDELETEMEMORYOBJECTSEXTPROC glDeleteMemoryObjectsEXT =
                    (PFNGLDELETEMEMORYOBJECTSEXTPROC)glContext->getProcAddress(
                            "glDeleteMemoryObjectsEXT");

            glDeleteMemoryObjectsEXT(1, &glMemoryObject);
            glFun->glDeleteTextures(1, &glTexture);
        };
#else
        Q_UNREACHABLE();
#endif // BUILDFLAG(ENABLE_VULKAN)
    }

    texture = QNativeInterface::QSGOpenGLTexture::fromNative(glTexture, win, size(), texOpts);
    Q_ASSERT(glFun->glGetError() == GL_NO_ERROR);
#elif defined(Q_OS_WIN)
    // TODO: Add WGL support over ANGLE.
    QT_NOT_YET_IMPLEMENTED
#elif defined(Q_OS_MACOS)
    uint32_t glTexture = makeCGLTexture(win, ioSurface.get(), size());
    texture = QNativeInterface::QSGOpenGLTexture::fromNative(glTexture, win, size(), texOpts);

    m_frontBuffer->textureCleanupCallback = [glTexture]() {
        auto *glContext = QOpenGLContext::currentContext();
        if (!glContext)
            return;
        auto glFun = glContext->functions();
        glFun->glDeleteTextures(1, &glTexture);
    };
#endif

    return texture;
}

} // namespace QtWebEngineCore
