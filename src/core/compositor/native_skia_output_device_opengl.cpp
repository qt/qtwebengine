// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "native_skia_output_device_opengl.h"

#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglfunctions.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgtexture.h>

#include "ui/base/ozone_buildflags.h"

#if BUILDFLAG(IS_OZONE)
#include "ozone/gl_helper.h"
#include "ozone/ozone_util_qt.h"

#include "base/posix/eintr_wrapper.h"
#include "third_party/skia/include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "ui/gfx/linux/drm_util_linux.h"
#include "ui/gfx/linux/native_pixmap_dmabuf.h"

#if BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
#include "ozone/glx_helper.h"
#endif

#if QT_CONFIG(egl)
#include "ozone/egl_helper.h"
#endif

#if BUILDFLAG(ENABLE_VULKAN)
#if BUILDFLAG(IS_OZONE_X11)
// We need to define USE_VULKAN_XCB for proper vulkan function pointers.
// Avoiding it may lead to call wrong vulkan functions.
// This is originally defined in chromium/gpu/vulkan/BUILD.gn.
#define USE_VULKAN_XCB
#endif // BUILDFLAG(IS_OZONE_X11)
#include "gpu/vulkan/vulkan_function_pointers.h"

#include "components/viz/common/gpu/vulkan_context_provider.h"
#include "gpu/vulkan/vulkan_device_queue.h"
#include "third_party/skia/include/gpu/vk/GrVkTypes.h"
#include "third_party/skia/include/gpu/ganesh/vk/GrVkBackendSurface.h"
#endif // BUILDFLAG(ENABLE_VULKAN)
#endif // BUILDFLAG(IS_OZONE)

#if defined(Q_OS_WIN)
#include "compositor/wgl_helper.h"
#endif

namespace QtWebEngineCore {

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
    qCDebug(lcWebEngineCompositor, "Native Skia Output Device: OpenGL");

    SkColorType skColorType = kRGBA_8888_SkColorType;
#if BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
    if (OzoneUtilQt::usingGLX() && m_contextState->gr_context_type() == gpu::GrContextType::kGL)
        skColorType = kBGRA_8888_SkColorType;
#endif

    capabilities_.sk_color_type_map[viz::SinglePlaneFormat::kRGBA_8888] = skColorType;
    capabilities_.sk_color_type_map[viz::SinglePlaneFormat::kRGBX_8888] = skColorType;
    capabilities_.sk_color_type_map[viz::SinglePlaneFormat::kBGRA_8888] = skColorType;
    capabilities_.sk_color_type_map[viz::SinglePlaneFormat::kBGRX_8888] = skColorType;
}

NativeSkiaOutputDeviceOpenGL::~NativeSkiaOutputDeviceOpenGL() { }

#if defined(Q_OS_MACOS)
uint32_t makeCGLTexture(QQuickWindow *win, IOSurfaceRef ioSurface, const QSize &size);
#endif

QSGTexture *NativeSkiaOutputDeviceOpenGL::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer || !m_readyWithTexture)
        return nullptr;

#if BUILDFLAG(IS_OZONE)
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
#endif // BUILDFLAG(IS_OZONE)

    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    QSGTexture *texture = nullptr;

#if BUILDFLAG(IS_OZONE)
    QOpenGLContext *glContext = QOpenGLContext::currentContext();
    Q_ASSERT(glContext);
    auto glFun = glContext->functions();
    GLuint glTexture = 0;

#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
    // Log and clear error flags for assert at end of function
    while (true) {
        auto glError = glFun->glGetError();
        if (glError == GL_NO_ERROR || glError == GL_CONTEXT_LOST)
            break;
        qWarning("GL error flag set on entry: %s", getGLErrorString(glError));
    }
#endif // !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)

    if (nativePixmap) {
        Q_ASSERT(m_contextState->gr_context_type() == gpu::GrContextType::kGL);

#if BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)
        if (OzoneUtilQt::usingGLX()) {
            qCDebug(lcWebEngineCompositor, "GLX: Importing NativePixmap into GL Texture.");

            GLXHelper *glxHelper = GLXHelper::instance();
            auto *glxFun = glxHelper->functions();

            const auto dmaBufFd = HANDLE_EINTR(dup(nativePixmap->GetDmaBufFd(0)));
            if (dmaBufFd < 0) {
                qFatal("GLX: Could not import the dma-buf as an XPixmap because the FD couldn't be "
                       "dup()ed.");
            }
            base::ScopedFD scopedFd(dmaBufFd);

            DCHECK(base::IsValueInRangeForNumericType<uint32_t>(
                    nativePixmap->GetDmaBufPlaneSize(0)));
            uint32_t size = base::checked_cast<uint32_t>(nativePixmap->GetDmaBufPlaneSize(0));
            DCHECK(base::IsValueInRangeForNumericType<uint16_t>(
                    nativePixmap->GetBufferSize().width()));
            uint16_t width = base::checked_cast<uint16_t>(nativePixmap->GetBufferSize().width());
            DCHECK(base::IsValueInRangeForNumericType<uint16_t>(
                    nativePixmap->GetBufferSize().height()));
            uint16_t height = base::checked_cast<uint16_t>(nativePixmap->GetBufferSize().height());
            DCHECK(base::IsValueInRangeForNumericType<uint16_t>(nativePixmap->GetDmaBufPitch(0)));
            uint16_t stride = base::checked_cast<uint16_t>(nativePixmap->GetDmaBufPitch(0));

            uint32_t pixmapId = glxHelper->importBufferAsPixmap(scopedFd.release(), size, width,
                                                                height, stride);
            if (!pixmapId)
                qFatal("GLX: Could not import the dma-buf as an XPixmap.");

            // clang-format off
            static const int pixmapAttribs[] = {
                 GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
                 GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
                 0
            };
            // clang-format on

            Display *display = glxHelper->getXDisplay();
            GLXPixmap glxPixmap = glXCreatePixmap(display, glxHelper->getFBConfig(),
                                                  static_cast<::Pixmap>(pixmapId), pixmapAttribs);

            glFun->glGenTextures(1, &glTexture);
            glFun->glBindTexture(GL_TEXTURE_2D, glTexture);
            glxFun->glXBindTexImageEXT(display, glxPixmap, GLX_FRONT_LEFT_EXT, nullptr);
            glFun->glBindTexture(GL_TEXTURE_2D, 0);

            m_frontBuffer->textureCleanupCallback = [glFun, glxFun, display, glxPixmap, glTexture,
                                                     glxHelper, pixmapId]() {
                glxFun->glXReleaseTexImageEXT(display, glxPixmap, GLX_FRONT_LEFT_EXT);
                glFun->glDeleteTextures(1, &glTexture);
                glXDestroyGLXPixmap(display, glxPixmap);
                glxHelper->freePixmap(pixmapId);
            };
        }
#endif // BUILDFLAG(IS_OZONE_X11) && QT_CONFIG(xcb_glx_plugin)

#if QT_CONFIG(egl)
        if (OzoneUtilQt::usingEGL()) {
            qCDebug(lcWebEngineCompositor, "EGL: Importing NativePixmap into GL Texture.");

            EGLHelper *eglHelper = EGLHelper::instance();
            auto *eglFun = eglHelper->functions();
            auto *glExtFun = GLHelper::instance()->functions();

            const auto dmaBufFd = HANDLE_EINTR(dup(nativePixmap->GetDmaBufFd(0)));
            if (dmaBufFd < 0) {
                qFatal("EGL: Could not import the dma-buf as an EGLImage because the FD couldn't "
                       "be dup()ed.");
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
            EGLDisplay eglDisplay = eglHelper->getEGLDisplay();
            EGLImage eglImage =
                    eglFun->eglCreateImage(eglDisplay, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
                                           (EGLClientBuffer)NULL, attributeList);
            Q_ASSERT(eglImage != EGL_NO_IMAGE_KHR);

            glFun->glGenTextures(1, &glTexture);
            glFun->glBindTexture(GL_TEXTURE_2D, glTexture);
            glExtFun->glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);
            glFun->glBindTexture(GL_TEXTURE_2D, 0);

            m_frontBuffer->textureCleanupCallback = [glFun, eglFun, glTexture, eglDisplay,
                                                     eglImage]() {
                glFun->glDeleteTextures(1, &glTexture);
                eglFun->eglDestroyImage(eglDisplay, eglImage);
            };
        }
#endif // QT_CONFIG(egl)
    } else {
#if BUILDFLAG(ENABLE_VULKAN)
        qCDebug(lcWebEngineCompositor, "VULKAN: Importing VkImage into GL Texture.");
        Q_ASSERT(m_contextState->gr_context_type() == gpu::GrContextType::kVulkan);

        gpu::VulkanFunctionPointers *vfp = gpu::GetVulkanFunctionPointers();
        gpu::VulkanDeviceQueue *vulkanDeviceQueue =
                m_contextState->vk_context_provider()->GetDeviceQueue();
        VkDevice vulkanDevice = vulkanDeviceQueue->GetVulkanDevice();

        VkDeviceMemory importedImageMemory = vkImageInfo.fAlloc.fMemory;
        VkDeviceSize importedImageSize = vkImageInfo.fAlloc.fSize;

        VkMemoryGetFdInfoKHR exportInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
            .pNext = nullptr,
            .memory = importedImageMemory,
            .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR,
        };

        int fd = -1;
        if (vfp->vkGetMemoryFdKHR(vulkanDevice, &exportInfo, &fd) != VK_SUCCESS)
            qFatal("VULKAN: Unable to extract file descriptor out of external VkImage.");

        auto *glExtFun = GLHelper::instance()->functions();

        // Import memory object
        GLuint glMemoryObject;
        glExtFun->glCreateMemoryObjectsEXT(1, &glMemoryObject);
        GLint dedicated = GL_TRUE;
        glExtFun->glMemoryObjectParameterivEXT(glMemoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                               &dedicated);
        glExtFun->glImportMemoryFdEXT(glMemoryObject, importedImageSize,
                                      GL_HANDLE_TYPE_OPAQUE_FD_EXT, fd);
        if (!glExtFun->glIsMemoryObjectEXT(glMemoryObject))
            qFatal("VULKAN: Failed to import memory object.");

        // Bind memory object to texture
        glFun->glGenTextures(1, &glTexture);
        glFun->glBindTexture(GL_TEXTURE_2D, glTexture);
        glFun->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_TILING_EXT,
                               vkImageInfo.fImageTiling == VK_IMAGE_TILING_OPTIMAL
                                       ? GL_OPTIMAL_TILING_EXT
                                       : GL_LINEAR_TILING_EXT);
        glExtFun->glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, size().width(), size().height(),
                                       glMemoryObject, 0);
        glFun->glBindTexture(GL_TEXTURE_2D, 0);

        m_frontBuffer->textureCleanupCallback = [glFun, glExtFun, glTexture, glMemoryObject]() {
            Q_ASSERT(glFun->glGetError() == GL_NO_ERROR);

            glExtFun->glDeleteMemoryObjectsEXT(1, &glMemoryObject);
            glFun->glDeleteTextures(1, &glTexture);
        };
#else
        Q_UNREACHABLE();
#endif // BUILDFLAG(ENABLE_VULKAN)
    }

    texture = QNativeInterface::QSGOpenGLTexture::fromNative(glTexture, win, size(), texOpts);
    Q_ASSERT(glFun->glGetError() == GL_NO_ERROR);
#elif defined(Q_OS_WIN)
    qCDebug(lcWebEngineCompositor, "WGL: Importing DXGI Resource into GL Texture.");
    Q_ASSERT(m_contextState->gr_context_type() == gpu::GrContextType::kGL);

    Q_ASSERT(overlayImage->type() == gl::DCLayerOverlayType::kNV12Texture);
    Microsoft::WRL::ComPtr<ID3D11Texture2D> chromeTexture = overlayImage->nv12_texture();
    if (!chromeTexture) {
        qWarning("WGL: No D3D texture.");
        return nullptr;
    }

    HRESULT hr;

    Microsoft::WRL::ComPtr<IDXGIResource1> dxgiResource;
    hr = chromeTexture->QueryInterface(IID_PPV_ARGS(&dxgiResource));
    Q_ASSERT(SUCCEEDED(hr));

    HANDLE sharedHandle = INVALID_HANDLE_VALUE;
    hr = dxgiResource->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ, nullptr,
                                          &sharedHandle);
    Q_ASSERT(SUCCEEDED(hr));
    Q_ASSERT(sharedHandle != INVALID_HANDLE_VALUE);

    WGLHelper *wglHelper = WGLHelper::instance();
    D3DSharedTexture *d3dSharedTexture =
            new D3DSharedTexture(wglHelper->functions(), wglHelper->device(),
                                 wglHelper->immediateContext(),
                                 wglHelper->interopDevice(), sharedHandle);
    d3dSharedTexture->lockObject();
    ::CloseHandle(sharedHandle);

    texture = QNativeInterface::QSGOpenGLTexture::fromNative(d3dSharedTexture->glTexture(), win,
                                                             size(), texOpts);

    m_frontBuffer->textureCleanupCallback = [d3dSharedTexture]() {
        d3dSharedTexture->unlockObject();
        delete d3dSharedTexture;
    };
#elif defined(Q_OS_MACOS)
    qCDebug(lcWebEngineCompositor, "CGL: Importing IOSurface into GL Texture.");
    uint32_t glTexture = makeCGLTexture(win, ioSurface.get(), size());
    texture = QNativeInterface::QSGOpenGLTexture::fromNative(glTexture, win, size(), texOpts);

    m_frontBuffer->textureCleanupCallback = [glTexture]() {
        auto *glContext = QOpenGLContext::currentContext();
        if (!glContext)
            return;
        auto glFun = glContext->functions();
        glFun->glDeleteTextures(1, &glTexture);
    };
#endif // BUILDFLAG(IS_OZONE)

    return texture;
}

} // namespace QtWebEngineCore
