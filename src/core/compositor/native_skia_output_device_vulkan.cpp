// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "native_skia_output_device_vulkan.h"

#include "gpu/command_buffer/service/shared_image/shared_image_format_service_utils.h"
#include "ui/base/ozone_buildflags.h"

#include <QtGui/qvulkaninstance.h>
#include <QtGui/qvulkanfunctions.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgtexture.h>

#if BUILDFLAG(IS_OZONE)
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
#endif // BUILDFLAG(IS_OZONE)

namespace QtWebEngineCore {

NativeSkiaOutputDeviceVulkan::NativeSkiaOutputDeviceVulkan(
        scoped_refptr<gpu::SharedContextState> contextState, bool requiresAlpha,
        gpu::MemoryTracker *memoryTracker, viz::SkiaOutputSurfaceDependency *dependency,
        gpu::SharedImageFactory *shared_image_factory,
        gpu::SharedImageRepresentationFactory *shared_image_representation_factory,
        DidSwapBufferCompleteCallback didSwapBufferCompleteCallback)
    : NativeSkiaOutputDevice(contextState, requiresAlpha, memoryTracker, dependency,
                             shared_image_factory, shared_image_representation_factory,
                             didSwapBufferCompleteCallback)
{
    qCDebug(lcWebEngineCompositor, "Native Skia Output Device: Vulkan");

    SkColorType skColorType = kRGBA_8888_SkColorType;
    capabilities_.sk_color_type_map[viz::SinglePlaneFormat::kRGBA_8888] = skColorType;
    capabilities_.sk_color_type_map[viz::SinglePlaneFormat::kRGBX_8888] = skColorType;
    capabilities_.sk_color_type_map[viz::SinglePlaneFormat::kBGRA_8888] = skColorType;
    capabilities_.sk_color_type_map[viz::SinglePlaneFormat::kBGRX_8888] = skColorType;
}

NativeSkiaOutputDeviceVulkan::~NativeSkiaOutputDeviceVulkan() { }

QSGTexture *NativeSkiaOutputDeviceVulkan::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer || !m_readyWithTexture)
        return nullptr;

#if BUILDFLAG(IS_OZONE)
    Q_ASSERT(m_contextState->gr_context_type() == gpu::GrContextType::kVulkan);

    GrVkImageInfo vkImageInfo;
    scoped_refptr<gfx::NativePixmap> nativePixmap = m_frontBuffer->nativePixmap();
    if (!nativePixmap) {
        if (m_isNativeBufferSupported) {
            qWarning("VULKAN: No NativePixmap.");
            return nullptr;
        }

        sk_sp<SkImage> skImage = m_frontBuffer->skImage();
        if (!skImage) {
            qWarning("VULKAN: No SkImage.");
            return nullptr;
        }

        if (!skImage->isTextureBacked()) {
            qWarning("VULKAN: SkImage is not backed by GPU texture.");
            return nullptr;
        }

        GrBackendTexture backendTexture;
        bool success = SkImages::GetBackendTextureFromImage(skImage, &backendTexture, false);
        if (!success || !backendTexture.isValid()) {
            qWarning("VULKAN: Failed to retrieve backend texture from SkImage.");
            return nullptr;
        }

        if (backendTexture.backend() != GrBackendApi::kVulkan) {
            qWarning("VULKAN: Backend texture is not a Vulkan texture.");
            return nullptr;
        }

        GrBackendTextures::GetVkImageInfo(backendTexture, &vkImageInfo);

        if (vkImageInfo.fAlloc.fMemory == VK_NULL_HANDLE) {
            qWarning("VULKAN: Unable to access Vulkan memory.");
            return nullptr;
        }
    }
#elif defined(Q_OS_WIN)
    Q_ASSERT(m_contextState->gr_context_type() == gpu::GrContextType::kGL);

    std::optional<gl::DCLayerOverlayImage> overlayImage = m_frontBuffer->overlayImage();
    if (!overlayImage) {
        qWarning("No overlay image.");
        return nullptr;
    }
#endif

    QSGRendererInterface *ri = win->rendererInterface();
    VkDevice qtVulkanDevice =
            *static_cast<VkDevice *>(ri->getResource(win, QSGRendererInterface::DeviceResource));
    VkPhysicalDevice qtPhysicalDevice = *static_cast<VkPhysicalDevice *>(
            ri->getResource(win, QSGRendererInterface::PhysicalDeviceResource));
    QVulkanFunctions *f = win->vulkanInstance()->functions();
    QVulkanDeviceFunctions *df = win->vulkanInstance()->deviceFunctions(qtVulkanDevice);

    VkExternalMemoryImageCreateInfoKHR externalMemoryImageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .handleTypes = 0,
    };
#if BUILDFLAG(IS_OZONE)
    base::ScopedFD scopedFd;

    VkSubresourceLayout planeLayout = {
        .offset = 0,
        .size = 0,
        .rowPitch = 0,
        .arrayPitch = 0,
        .depthPitch = 0,
    };

    VkImageDrmFormatModifierExplicitCreateInfoEXT modifierInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT,
        .pNext = nullptr,
        .drmFormatModifier = 0,
        .drmFormatModifierPlaneCount = 1,
        .pPlaneLayouts = &planeLayout,
    };

    bool usingDrmModifier = false;
    if (nativePixmap) {
        qCDebug(lcWebEngineCompositor, "VULKAN: Importing NativePixmap into VkImage.");
        gfx::NativePixmapHandle nativePixmapHandle = nativePixmap->ExportHandle();
        qCDebug(lcWebEngineCompositor, "  DRM Format Modifier: 0x%lx", nativePixmapHandle.modifier);

        if (nativePixmapHandle.modifier != gfx::NativePixmapHandle::kNoModifier) {
            usingDrmModifier = true;
            if (nativePixmapHandle.planes.size() != 1)
                qFatal("VULKAN: Multiple planes are not supported.");

            planeLayout.offset = nativePixmapHandle.planes[0].offset;
            planeLayout.rowPitch = nativePixmapHandle.planes[0].stride;
            modifierInfo.drmFormatModifier = nativePixmapHandle.modifier;

            externalMemoryImageCreateInfo.pNext = &modifierInfo;
        }
        externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

        scopedFd = std::move(nativePixmapHandle.planes[0].fd);
    } else {
        qCDebug(lcWebEngineCompositor, "VULKAN: Importing VkImage into VkImage.");
        externalMemoryImageCreateInfo.handleTypes =
                VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

        VkMemoryGetFdInfoKHR exportInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
            .pNext = nullptr,
            .memory = vkImageInfo.fAlloc.fMemory,
            .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR,
        };

        gpu::VulkanFunctionPointers *vfp = gpu::GetVulkanFunctionPointers();
        gpu::VulkanDeviceQueue *vulkanDeviceQueue =
                m_contextState->vk_context_provider()->GetDeviceQueue();
        VkDevice vulkanDevice = vulkanDeviceQueue->GetVulkanDevice();

        int fd = -1;
        if (vfp->vkGetMemoryFdKHR(vulkanDevice, &exportInfo, &fd) != VK_SUCCESS)
            qFatal("VULKAN: Unable to extract file descriptor out of external VkImage.");

        scopedFd.reset(fd);
    }

    if (!scopedFd.is_valid())
        qFatal("VULKAN: Unable to extract file descriptor.");
#elif defined(Q_OS_WIN)
    qCDebug(lcWebEngineCompositor, "VULKAN: Importing DXGI Resource into VkImage.");
    externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;

    Q_ASSERT(overlayImage->type() == gl::DCLayerOverlayType::kNV12Texture);
    Microsoft::WRL::ComPtr<ID3D11Texture2D> chromeTexture = overlayImage->nv12_texture();
    if (!chromeTexture) {
        qWarning("VULKAN: No D3D texture.");
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
#endif

    VkImageCreateInfo importedImageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = &externalMemoryImageCreateInfo,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = gpu::ToVkFormatSinglePlanar(m_frontBuffer->sharedImageFormat()),
        .extent = {
            .width = static_cast<uint32_t>(size().width()),
            .height = static_cast<uint32_t>(size().height()),
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        // The image is fed into a combined image sampler
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        // VkExternalMemoryImageCreateInfo only allows UNDEFINED
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

#if BUILDFLAG(IS_OZONE)
    if (usingDrmModifier)
        importedImageCreateInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    else if (vkImageInfo.fAlloc.fMemory != VK_NULL_HANDLE)
        importedImageCreateInfo.tiling = vkImageInfo.fImageTiling;
#endif

    VkResult result;
    VkImage importedImage = VK_NULL_HANDLE;
    result = df->vkCreateImage(qtVulkanDevice, &importedImageCreateInfo, nullptr /* pAllocator */,
                               &importedImage);
    if (result != VK_SUCCESS)
        qFatal("VULKAN: vkCreateImage failed result: %d", static_cast<int>(result));

#if BUILDFLAG(IS_OZONE)
    VkImportMemoryFdInfoKHR importMemoryHandleInfo = {
        .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR,
        .pNext = nullptr,
        .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR,
        .fd = scopedFd.release(),
    };

    if (nativePixmap)
        importMemoryHandleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
#elif defined(Q_OS_WIN)
    VkImportMemoryWin32HandleInfoKHR importMemoryHandleInfo = {
        .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
        .pNext = nullptr,
        .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT,
        .handle = sharedHandle,
    };
#endif

    VkMemoryRequirements requirements;
    df->vkGetImageMemoryRequirements(qtVulkanDevice, importedImage, &requirements);
    if (!requirements.memoryTypeBits)
        qFatal("VULKAN: vkGetImageMemoryRequirements failed.");

    VkPhysicalDeviceMemoryProperties memoryProperties;
    f->vkGetPhysicalDeviceMemoryProperties(qtPhysicalDevice, &memoryProperties);
    constexpr VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    constexpr uint32_t kMaxIndex = 31;
    uint32_t memoryTypeIndex = kMaxIndex + 1;
    for (uint32_t i = 0; i <= kMaxIndex; i++) {
        if (((1u << i) & requirements.memoryTypeBits) == 0)
            continue;
        if ((memoryProperties.memoryTypes[i].propertyFlags & flags) != flags)
            continue;
        memoryTypeIndex = i;
        break;
    }

    if (memoryTypeIndex > kMaxIndex)
        qFatal("VULKAN: Cannot find valid memory type index.");

    VkMemoryDedicatedAllocateInfoKHR dedicatedMemoryInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR,
        .pNext = &importMemoryHandleInfo,
        .image = importedImage,
        .buffer = VK_NULL_HANDLE,
    };

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &dedicatedMemoryInfo,
        .allocationSize = requirements.size,
        .memoryTypeIndex = memoryTypeIndex,
    };

    VkDeviceMemory importedImageMemory = VK_NULL_HANDLE;
    result = df->vkAllocateMemory(qtVulkanDevice, &memoryAllocateInfo, nullptr /* pAllocator */,
                                  &importedImageMemory);
    if (result != VK_SUCCESS)
        qFatal("VULKAN: vkAllocateMemory failed result: %d", static_cast<int>(result));

    df->vkBindImageMemory(qtVulkanDevice, importedImage, importedImageMemory, 0);

    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    QSGTexture *texture = QNativeInterface::QSGVulkanTexture::fromNative(
            importedImage, importedImageCreateInfo.initialLayout, win, size(), texOpts);

    m_frontBuffer->textureCleanupCallback = [=]() {
        df->vkDestroyImage(qtVulkanDevice, importedImage, nullptr);
        df->vkFreeMemory(qtVulkanDevice, importedImageMemory, nullptr);
#if defined(Q_OS_WIN)
        ::CloseHandle(sharedHandle);
#endif
    };

    return texture;
}

} // namespace QtWebEngineCore
