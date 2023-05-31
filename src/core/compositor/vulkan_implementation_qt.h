// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef VULKAN_IMPLEMENTATION_QT_H
#define VULKAN_IMPLEMENTATION_QT_H

#include "gpu/vulkan/vulkan_implementation.h"
#include "gpu/vulkan/vulkan_instance.h"

namespace gpu {

class VulkanImplementationQt : public VulkanImplementation
{
public:
    VulkanImplementationQt();
    ~VulkanImplementationQt() override;

    // Overridden from VulkanImplementation.
    bool InitializeVulkanInstance(bool using_surface) override;
    VulkanInstance *GetVulkanInstance() override;
    std::unique_ptr<VulkanSurface> CreateViewSurface(gfx::AcceleratedWidget window) override;
    bool GetPhysicalDevicePresentationSupport(
            VkPhysicalDevice device,
            const std::vector<VkQueueFamilyProperties> &queue_family_properties,
            uint32_t queue_family_index) override;
    std::vector<const char *> GetRequiredDeviceExtensions() override;
    std::vector<const char *> GetOptionalDeviceExtensions() override;
    VkFence CreateVkFenceForGpuFence(VkDevice vk_device) override;
    std::unique_ptr<gfx::GpuFence> ExportVkFenceToGpuFence(VkDevice vk_device,
                                                           VkFence vk_fence) override;
    VkSemaphore CreateExternalSemaphore(VkDevice vk_device) override;
    VkSemaphore ImportSemaphoreHandle(VkDevice vk_device, SemaphoreHandle handle) override;
    SemaphoreHandle GetSemaphoreHandle(VkDevice vk_device, VkSemaphore vk_semaphore) override;
    VkExternalMemoryHandleTypeFlagBits GetExternalImageHandleType() override;
    bool CanImportGpuMemoryBuffer(VulkanDeviceQueue* device_queue,
                                  gfx::GpuMemoryBufferType memory_buffer_type) override;
    std::unique_ptr<VulkanImage> CreateImageFromGpuMemoryHandle(VulkanDeviceQueue *device_queue,
                                                                gfx::GpuMemoryBufferHandle gmb_handle,
                                                                gfx::Size size, VkFormat vk_format,
                                                                const gfx::ColorSpace &color_space) override;

private:
    VulkanInstance vulkan_instance_;
};

} // namespace gpu

#endif // VULKAN_IMPLEMENTATION_QT_H
