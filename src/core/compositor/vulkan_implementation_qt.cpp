// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "vulkan_implementation_qt.h"

#include "base/environment.h"
#include "base/logging.h"
#include "gpu/vulkan/vulkan_image.h"
#include "gpu/vulkan/vulkan_surface.h"
#include "gpu/vulkan/vulkan_util.h"
#include "ui/gfx/gpu_fence.h"

#include <vulkan/vulkan.h>

namespace gpu {

VulkanImplementationQt::VulkanImplementationQt() : VulkanImplementation(false) { }

VulkanImplementationQt::~VulkanImplementationQt() = default;

bool VulkanImplementationQt::InitializeVulkanInstance(bool /*using_surface*/)
{
    std::vector<const char *> required_extensions = {
        VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
    };

    auto env = base::Environment::Create();
    std::string vulkan_path;
    if (!env->GetVar("QT_VULKAN_LIB", &vulkan_path)) {
#if BUILDFLAG(IS_WIN)
        vulkan_path = "vulkan-1.dll";
#else
        vulkan_path = "libvulkan.so.1";
#endif
    }

    if (!vulkan_instance_.Initialize(base::FilePath::FromUTF8Unsafe(vulkan_path),
                                     required_extensions, {})) {
        LOG(ERROR) << "Failed to initialize vulkan instance";
        return false;
    }

    return true;
}

VulkanInstance *VulkanImplementationQt::GetVulkanInstance()
{
    return &vulkan_instance_;
}

std::unique_ptr<VulkanSurface>
VulkanImplementationQt::CreateViewSurface(gfx::AcceleratedWidget /*window*/)
{
    NOTREACHED();
    return nullptr;
}

bool VulkanImplementationQt::GetPhysicalDevicePresentationSupport(
        VkPhysicalDevice /*device*/,
        const std::vector<VkQueueFamilyProperties> & /*queue_family_properties*/,
        uint32_t /*queue_family_index*/)
{
    NOTREACHED();
    return true;
}

std::vector<const char *> VulkanImplementationQt::GetRequiredDeviceExtensions()
{
    return {
        VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
#if BUILDFLAG(IS_WIN)
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
#else
        VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
#endif
    };
}

std::vector<const char *> VulkanImplementationQt::GetOptionalDeviceExtensions()
{
    return {
        VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
#if BUILDFLAG(IS_WIN)
        VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
#else
        VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
        VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
        VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
#endif
    };
}

VkFence VulkanImplementationQt::CreateVkFenceForGpuFence(VkDevice /*vk_device*/)
{
    NOTREACHED();
    return VK_NULL_HANDLE;
}

std::unique_ptr<gfx::GpuFence>
VulkanImplementationQt::ExportVkFenceToGpuFence(VkDevice /*vk_device*/, VkFence /*vk_fence*/)
{
    NOTREACHED();
    return nullptr;
}

VkSemaphore VulkanImplementationQt::CreateExternalSemaphore(VkDevice vk_device)
{
    return CreateExternalVkSemaphore(
#if BUILDFLAG(IS_WIN)
            vk_device, VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT);
#else
            vk_device, VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT);
#endif
}

VkSemaphore VulkanImplementationQt::ImportSemaphoreHandle(VkDevice vk_device,
                                                          SemaphoreHandle sync_handle)
{
    return ImportVkSemaphoreHandle(vk_device, std::move(sync_handle));
}

SemaphoreHandle VulkanImplementationQt::GetSemaphoreHandle(VkDevice vk_device,
                                                           VkSemaphore vk_semaphore)
{
    return GetVkSemaphoreHandle(vk_device, vk_semaphore,
#if BUILDFLAG(IS_WIN)
                                VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT);
#else
                                VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT);
#endif
}

VkExternalMemoryHandleTypeFlagBits VulkanImplementationQt::GetExternalImageHandleType()
{
#if BUILDFLAG(IS_WIN)
    return VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
#else
    return VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
}

bool VulkanImplementationQt::CanImportGpuMemoryBuffer(
        VulkanDeviceQueue *device_queue,
        gfx::GpuMemoryBufferType memory_buffer_type)
{
#if BUILDFLAG(IS_LINUX)
    const auto &enabled_extensions = device_queue->enabled_extensions();
    return gfx::HasExtension(enabled_extensions,
                             VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME) &&
           gfx::HasExtension(enabled_extensions,
                             VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME) &&
           memory_buffer_type == gfx::GpuMemoryBufferType::NATIVE_PIXMAP;
#else
    return false;
#endif
}

std::unique_ptr<VulkanImage> VulkanImplementationQt::CreateImageFromGpuMemoryHandle(VulkanDeviceQueue *device_queue,
                                                                                    gfx::GpuMemoryBufferHandle gmb_handle,
                                                                                    gfx::Size size,
                                                                                    VkFormat vk_format,
                                                                                    const gfx::ColorSpace &)
{
#if BUILDFLAG(IS_LINUX)
    constexpr auto kUsage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    auto tiling = gmb_handle.native_pixmap_handle.modifier ==
                          gfx::NativePixmapHandle::kNoModifier
                      ? VK_IMAGE_TILING_OPTIMAL
                      : VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    return gpu::VulkanImage::CreateFromGpuMemoryBufferHandle(
        device_queue, std::move(gmb_handle), size, vk_format, kUsage, /*flags=*/0,
        tiling, VK_QUEUE_FAMILY_EXTERNAL);
#else
    NOTIMPLEMENTED();
    return nullptr;
#endif
}

} // namespace gpu
