// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "native_skia_output_device_vulkan.h"

#include "gpu/command_buffer/service/shared_image/shared_image_format_service_utils.h"
#include "ui/base/ozone_buildflags.h"

#include <QtGui/qvulkaninstance.h>
#include <QtGui/qvulkanfunctions.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgtexture.h>

#if defined(USE_OZONE)
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
#endif // defined(USE_OZONE)

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
    SkColorType skColorType = kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBA_8888)] = skColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBX_8888)] = skColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRA_8888)] = skColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRX_8888)] = skColorType;
}

NativeSkiaOutputDeviceVulkan::~NativeSkiaOutputDeviceVulkan() { }

QSGTexture *NativeSkiaOutputDeviceVulkan::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer || !m_readyWithTexture)
        return nullptr;

#if defined(USE_OZONE)
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

    absl::optional<gl::DCLayerOverlayImage> overlayImage = m_frontBuffer->overlayImage();
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

    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkPhysicalDeviceProperties deviceProperties;
    f->vkGetPhysicalDeviceProperties(qtPhysicalDevice, &deviceProperties);
    if (deviceProperties.vendorID == 0x10DE) {
        // FIXME: This is a workaround for Nvidia driver.
        // The imported image is empty if the initialLayout is not
        // VK_IMAGE_LAYOUT_PREINITIALIZED.
        imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    }

    VkExternalMemoryImageCreateInfoKHR externalMemoryImageCreateInfo = {
        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR
    };
#if defined(USE_OZONE)
    VkSubresourceLayout planeLayout = {};
    VkImageDrmFormatModifierExplicitCreateInfoEXT modifierInfo = {
        VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT
    };
    base::ScopedFD scopedFd;

    if (nativePixmap) {
        gfx::NativePixmapHandle nativePixmapHandle = nativePixmap->ExportHandle();
        if (nativePixmapHandle.planes.size() != 1)
            qFatal("VULKAN: Multiple planes are not supported.");

        planeLayout.offset = nativePixmapHandle.planes[0].offset;
        planeLayout.size = 0;
        planeLayout.rowPitch = nativePixmapHandle.planes[0].stride;
        planeLayout.arrayPitch = 0;
        planeLayout.depthPitch = 0;

        modifierInfo.drmFormatModifier = nativePixmapHandle.modifier;
        modifierInfo.drmFormatModifierPlaneCount = 1;
        modifierInfo.pPlaneLayouts = &planeLayout;

        externalMemoryImageCreateInfo.pNext = &modifierInfo;
        externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

        scopedFd = std::move(nativePixmapHandle.planes[0].fd);
    } else {
        externalMemoryImageCreateInfo.pNext = nullptr;
        externalMemoryImageCreateInfo.handleTypes =
                VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

        VkMemoryGetFdInfoKHR exportInfo = { VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR };
        exportInfo.pNext = nullptr;
        exportInfo.memory = vkImageInfo.fAlloc.fMemory;
        exportInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

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
    externalMemoryImageCreateInfo.pNext = nullptr;
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

    constexpr VkImageUsageFlags kUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    VkImageCreateInfo importedImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    importedImageCreateInfo.pNext = &externalMemoryImageCreateInfo;
    importedImageCreateInfo.flags = 0;
    importedImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    importedImageCreateInfo.format = gpu::ToVkFormatSinglePlanar(m_frontBuffer->sharedImageFormat());
    importedImageCreateInfo.extent.width = static_cast<uint32_t>(size().width());
    importedImageCreateInfo.extent.height = static_cast<uint32_t>(size().height());
    importedImageCreateInfo.extent.depth = 1;
    importedImageCreateInfo.mipLevels = 1;
    importedImageCreateInfo.arrayLayers = 1;
    importedImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    importedImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    importedImageCreateInfo.usage = kUsage;
    importedImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    importedImageCreateInfo.queueFamilyIndexCount = 0;
    importedImageCreateInfo.pQueueFamilyIndices = nullptr;
    importedImageCreateInfo.initialLayout = imageLayout;

#if defined(USE_OZONE)
    if (nativePixmap)
        importedImageCreateInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    else
        importedImageCreateInfo.tiling = vkImageInfo.fImageTiling;
#endif

    VkResult result;
    VkImage importedImage = VK_NULL_HANDLE;
    result = df->vkCreateImage(qtVulkanDevice, &importedImageCreateInfo, nullptr /* pAllocator */,
                               &importedImage);
    if (result != VK_SUCCESS)
        qFatal() << "VULKAN: vkCreateImage failed result:" << result;

#if defined(USE_OZONE)
    VkImportMemoryFdInfoKHR importMemoryHandleInfo = {
        VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR
    };
    importMemoryHandleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    importMemoryHandleInfo.fd = scopedFd.release();

    if (nativePixmap)
        importMemoryHandleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
#elif defined(Q_OS_WIN)
    VkImportMemoryWin32HandleInfoKHR importMemoryHandleInfo = {
        VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR
    };
    importMemoryHandleInfo.pNext = nullptr;
    importMemoryHandleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
    importMemoryHandleInfo.handle = sharedHandle;
#endif

    VkMemoryDedicatedAllocateInfoKHR dedicatedMemoryInfo = {
        VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR
    };
    dedicatedMemoryInfo.pNext = &importMemoryHandleInfo;
    dedicatedMemoryInfo.image = importedImage;

    VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    memoryAllocateInfo.pNext = &dedicatedMemoryInfo;

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

    memoryAllocateInfo.allocationSize = requirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory importedImageMemory = VK_NULL_HANDLE;
    result = df->vkAllocateMemory(qtVulkanDevice, &memoryAllocateInfo, nullptr /* pAllocator */,
                                  &importedImageMemory);
    if (result != VK_SUCCESS)
        qFatal() << "VULKAN: vkAllocateMemory failed result:" << result;

    df->vkBindImageMemory(qtVulkanDevice, importedImage, importedImageMemory, 0);

    QQuickWindow::CreateTextureOptions texOpts(textureOptions);
    QSGTexture *texture = QNativeInterface::QSGVulkanTexture::fromNative(importedImage, imageLayout,
                                                                         win, size(), texOpts);

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
