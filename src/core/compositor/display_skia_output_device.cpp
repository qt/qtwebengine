// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "display_skia_output_device.h"

#include "compositor_resource_fence.h"
#include "type_conversion.h"

#include "gpu/command_buffer/service/skia_utils.h"
#include "third_party/skia/include/core/SkSurfaceProps.h"

#if QT_CONFIG(webengine_vulkan)
#if defined(USE_OZONE)
#include "ui/ozone/buildflags.h"
#if BUILDFLAG(OZONE_PLATFORM_X11)
// We need to define USE_VULKAN_XCB for proper vulkan function pointers.
// Avoiding it may lead to call wrong vulkan functions.
// This is originally defined in chromium/gpu/vulkan/BUILD.gn.
#define USE_VULKAN_XCB
#endif // BUILDFLAG(OZONE_PLATFORM_X11)
#endif // defined(USE_OZONE)
#include "gpu/vulkan/vulkan_function_pointers.h"

#include "components/viz/common/gpu/vulkan_context_provider.h"
#include "compositor/display_skia_output_device.h"
#include "gpu/vulkan/vma_wrapper.h"
#include "gpu/vulkan/vulkan_device_queue.h"
#include "third_party/vulkan_memory_allocator/include/vk_mem_alloc.h"

#include <QQuickWindow>
#include <QVulkanInstance>
#include <QVulkanFunctions>
#include <QVulkanDeviceFunctions>
#endif // QT_CONFIG(webengine_vulkan)

#include <QSGTexture>

namespace QtWebEngineCore {

class DisplaySkiaOutputDevice::Buffer
{
public:
    Buffer(DisplaySkiaOutputDevice *parent)
        : m_parent(parent)
        , m_shape(m_parent->m_shape)
    {
    }
    void initialize()
    {
        const auto &colorType = m_shape.characterization.colorType();
        DCHECK(colorType != kUnknown_SkColorType);

        m_texture = m_parent->m_contextState->gr_context()->createBackendTexture(
                m_shape.characterization.width(), m_shape.characterization.height(), colorType,
                GrMipMapped::kNo, GrRenderable::kYes);
        DCHECK(m_texture.isValid());

        if (m_texture.backend() == GrBackendApi::kVulkan) {
#if QT_CONFIG(webengine_vulkan)
            initVulkan();
#else
            NOTREACHED();
#endif
        } else {
            auto info = SkImageInfo::Make(m_shape.characterization.width(), m_shape.characterization.height(),
                                          colorType, kUnpremul_SkAlphaType);
            m_estimatedSize = info.computeMinByteSize();
        }
        m_parent->memory_type_tracker_->TrackMemAlloc(m_estimatedSize);

        SkSurfaceProps surfaceProps = SkSurfaceProps{0, kUnknown_SkPixelGeometry};
        m_surface = SkSurface::MakeFromBackendTexture(
                m_parent->m_contextState->gr_context(), m_texture,
                m_parent->capabilities_.output_surface_origin == gfx::SurfaceOrigin::kTopLeft
                ? kTopLeft_GrSurfaceOrigin
                : kBottomLeft_GrSurfaceOrigin,
                0 /* sampleCount */, colorType, m_shape.colorSpace.ToSkColorSpace(),
                &surfaceProps);
    }

    ~Buffer()
    {
#if QT_CONFIG(webengine_vulkan) && defined(Q_OS_WIN)
        CloseHandle(m_win32Handle);
#endif
        if (m_texture.isValid()) {
            DeleteGrBackendTexture(m_parent->m_contextState.get(), &m_texture);
            m_parent->memory_type_tracker_->TrackMemFree(m_estimatedSize);
        }
    }

    void createFence()
    {
        m_fence = CompositorResourceFence::create();
    }

    void consumeFence()
    {
        if (m_fence) {
            m_fence->wait();
            m_fence.reset();
        }
    }

    const Shape &shape() const { return m_shape; }
    const GrBackendTexture &texture() const { return m_texture; }
    SkSurface *surface() const { return m_surface.get(); }

#if QT_CONFIG(webengine_vulkan)
    const VkImageCreateInfo *imageCreateInfo() const { return &m_imageCreateInfo; }
    uint64_t allocationSize() const { return m_estimatedSize; }
    VkImageLayout imageLayout() const { return m_imageLayout; }
    uint32_t memoryTypeIndex() const { return m_memoryTypeIndex.value(); }

#if defined(Q_OS_WIN)
    const VkExternalMemoryHandleTypeFlagBits externalMemoryHandleType() const
    {
        return VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
    }

    HANDLE externalMemoryHandle() const { return m_win32Handle; }
#else
    const VkExternalMemoryHandleTypeFlagBits externalMemoryHandleType() const
    {
        return VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    }

    int externalMemoryHandle() const
    {
        VkMemoryGetFdInfoKHR exportInfo = { VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR };
        exportInfo.pNext = nullptr;
        exportInfo.memory = m_imageInfo.fAlloc.fMemory;
        exportInfo.handleType = externalMemoryHandleType();

        // Importing Vulkan memory object closes the file descriptor.
        int fd = -1;
        if (m_vfp->vkGetMemoryFdKHR(m_vulkanDevice, &exportInfo, &fd) != VK_SUCCESS)
            qFatal("VULKAN: Unable to extract file descriptor out of external VkImage!");

        return fd;
    }
#endif // defined(Q_OS_WIN)
#endif // QT_CONFIG(webengine_vulkan)

private:
    DisplaySkiaOutputDevice *m_parent;
    Shape m_shape;
    GrBackendTexture m_texture;
    sk_sp<SkSurface> m_surface;
    uint64_t m_estimatedSize = 0;
    scoped_refptr<CompositorResourceFence> m_fence;

#if QT_CONFIG(webengine_vulkan)
    static VkSampleCountFlagBits vkSampleCount(uint32_t sampleCount)
    {
        Q_ASSERT(sampleCount >= 1);
        switch (sampleCount) {
        case 1:
            return VK_SAMPLE_COUNT_1_BIT;
        case 2:
            return VK_SAMPLE_COUNT_2_BIT;
        case 4:
            return VK_SAMPLE_COUNT_4_BIT;
        case 8:
            return VK_SAMPLE_COUNT_8_BIT;
        case 16:
            return VK_SAMPLE_COUNT_16_BIT;
        default:
            Q_UNREACHABLE();
        }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    void initVulkan()
    {
        bool success = m_texture.getVkImageInfo(&m_imageInfo);
        if (!success)
            qFatal("VULKAN: Failed to get external Vulkan resources from Skia!");

        m_vfp = gpu::GetVulkanFunctionPointers();
        gpu::VulkanDeviceQueue *vulkanDeviceQueue =
                m_parent->m_contextState->vk_context_provider()->GetDeviceQueue();
        m_vulkanDevice = vulkanDeviceQueue->GetVulkanDevice();

        // Store allocation size for the external VkImage.
        m_estimatedSize = m_imageInfo.fAlloc.fSize;

        // Store initial layout for the imported image.
        if (vulkanDeviceQueue->vk_physical_device_properties().vendorID == 0x10DE) {
            // FIXME: This is a workaround for Nvidia driver.
            // The imported image is empty if the initialLayout is not PREINITIALIZED.
            m_imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        } else {
            // The initialLayout should be undefined for the external image.
            // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageCreateInfo.html#VUID-VkImageCreateInfo-pNext-01443
            m_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }

        // Specify VkCreateImageInfo for the imported VkImage.
        // The specification should match with the texture's VkImage.
        m_externalMemoryImageCreateInfo.sType =
                VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR;
        m_externalMemoryImageCreateInfo.pNext = nullptr;
        m_externalMemoryImageCreateInfo.handleTypes = externalMemoryHandleType();

        m_imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        m_imageCreateInfo.pNext = &m_externalMemoryImageCreateInfo;
        m_imageCreateInfo.flags =
                m_imageInfo.fProtected == GrProtected::kYes ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
        m_imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        m_imageCreateInfo.format = m_imageInfo.fFormat;
        m_imageCreateInfo.extent.width = static_cast<uint32_t>(m_shape.characterization.width());
        m_imageCreateInfo.extent.height = static_cast<uint32_t>(m_shape.characterization.height());
        m_imageCreateInfo.extent.depth = 1;
        m_imageCreateInfo.mipLevels = m_imageInfo.fLevelCount;
        m_imageCreateInfo.arrayLayers = 1;
        m_imageCreateInfo.samples = vkSampleCount(m_imageInfo.fSampleCount);
        m_imageCreateInfo.tiling = m_imageInfo.fImageTiling;
        m_imageCreateInfo.usage = m_imageInfo.fImageUsageFlags;
        m_imageCreateInfo.sharingMode = m_imageInfo.fSharingMode;
        m_imageCreateInfo.queueFamilyIndexCount = 0;
        m_imageCreateInfo.pQueueFamilyIndices = nullptr;
        m_imageCreateInfo.initialLayout = m_imageLayout;

#if defined(Q_OS_WIN)
        // Extract Windows handle for the memory of the external VkImage.
        // Ownership should be released at the destruction of the Buffer object.
        // Multiple handles for the same memory cause issues on Windows even if one is
        // released before extracting the other.
        VkMemoryGetWin32HandleInfoKHR exportInfo = {
            VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR
        };
        exportInfo.pNext = nullptr;
        exportInfo.memory = m_imageInfo.fAlloc.fMemory;
        exportInfo.handleType = externalMemoryHandleType();

        if (m_vfp->vkGetMemoryWin32HandleKHR(m_vulkanDevice, &exportInfo, &m_win32Handle)
            != VK_SUCCESS) {
            qFatal("VULKAN: Unable to extract handle out of external VkImage!");
        }
#endif // defined(Q_OS_WIN)

        VmaAllocator vmaAllocator = vulkanDeviceQueue->vma_allocator();
        const VmaAllocation vmaAllocation =
                reinterpret_cast<const VmaAllocation>(m_imageInfo.fAlloc.fBackendMemory);
        VmaAllocationInfo vmaInfo;
        gpu::vma::GetAllocationInfo(vmaAllocator, vmaAllocation, &vmaInfo);
        m_memoryTypeIndex = vmaInfo.memoryType;
    }

    gpu::VulkanFunctionPointers *m_vfp = nullptr;
    VkDevice m_vulkanDevice = VK_NULL_HANDLE;
    VkImageLayout m_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    GrVkImageInfo m_imageInfo;
    VkExternalMemoryImageCreateInfoKHR m_externalMemoryImageCreateInfo = {};
    VkImageCreateInfo m_imageCreateInfo = {};
#if defined(Q_OS_WIN)
    HANDLE m_win32Handle = nullptr;
#endif
    absl::optional<uint32_t> m_memoryTypeIndex;
#endif // QT_CONFIG(webengine_vulkan)
};

DisplaySkiaOutputDevice::DisplaySkiaOutputDevice(
        scoped_refptr<gpu::SharedContextState> contextState,
        bool requiresAlpha,
        gpu::MemoryTracker *memoryTracker,
        DidSwapBufferCompleteCallback didSwapBufferCompleteCallback)
    : SkiaOutputDevice(contextState->gr_context(), memoryTracker, didSwapBufferCompleteCallback)
    , Compositor(contextState->GrContextIsVulkan() ? Compositor::Type::Vulkan
                                                   : Compositor::Type::OpenGL)
    , m_contextState(contextState)
    , m_requiresAlpha(requiresAlpha)
{
    capabilities_.uses_default_gl_framebuffer = false;
    capabilities_.supports_surfaceless = true;
    capabilities_.preserve_buffer_content = true;
    capabilities_.only_invalidates_damage_rect = false;
    capabilities_.number_of_buffers = 3;

    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBA_8888)] =
            kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::RGBX_8888)] =
            kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRA_8888)] =
            kRGBA_8888_SkColorType;
    capabilities_.sk_color_types[static_cast<int>(gfx::BufferFormat::BGRX_8888)] =
            kRGBA_8888_SkColorType;
}

DisplaySkiaOutputDevice::~DisplaySkiaOutputDevice()
{
}

void DisplaySkiaOutputDevice::SetFrameSinkId(const viz::FrameSinkId &id)
{
    bind(id);
}

bool DisplaySkiaOutputDevice::Reshape(const SkSurfaceCharacterization &characterization,
                                      const gfx::ColorSpace &colorSpace,
                                      float device_scale_factor,
                                      gfx::OverlayTransform transform)
{
    m_shape = Shape{characterization, device_scale_factor, colorSpace};
    DCHECK_EQ(transform, gfx::OVERLAY_TRANSFORM_NONE);
    return true;
}

void DisplaySkiaOutputDevice::SwapBuffers(BufferPresentedCallback feedback,
                                          viz::OutputSurfaceFrame frame)
{
    DCHECK(m_backBuffer);

    StartSwapBuffers(std::move(feedback));
    m_frame = std::move(frame);
    m_backBuffer->createFence();

    {
        QMutexLocker locker(&m_mutex);
        m_taskRunner = base::SingleThreadTaskRunner::GetCurrentDefault();
        std::swap(m_middleBuffer, m_backBuffer);
        m_readyToUpdate = true;
    }

    if (auto obs = observer())
        obs->readyToSwap();
}

void DisplaySkiaOutputDevice::EnsureBackbuffer()
{
}

void DisplaySkiaOutputDevice::DiscardBackbuffer()
{
}

SkSurface *DisplaySkiaOutputDevice::BeginPaint(std::vector<GrBackendSemaphore> *end_semaphores)
{
    if (!m_backBuffer || m_backBuffer->shape() != m_shape) {
        m_backBuffer = std::make_unique<Buffer>(this);
        m_backBuffer->initialize();
    }
    return m_backBuffer->surface();
}

void DisplaySkiaOutputDevice::EndPaint()
{
}

void DisplaySkiaOutputDevice::swapFrame()
{
    QMutexLocker locker(&m_mutex);
    if (m_readyToUpdate) {
        std::swap(m_middleBuffer, m_frontBuffer);
        m_taskRunner->PostTask(FROM_HERE,
                               base::BindOnce(&DisplaySkiaOutputDevice::SwapBuffersFinished,
                                              base::Unretained(this)));
        m_taskRunner.reset();
        m_readyToUpdate = false;
    }
}

void DisplaySkiaOutputDevice::waitForTexture()
{
    if (m_frontBuffer)
        m_frontBuffer->consumeFence();
}

QSGTexture *DisplaySkiaOutputDevice::texture(QQuickWindow *win, uint32_t textureOptions)
{
    if (!m_frontBuffer)
        return nullptr;

    QQuickWindow::CreateTextureOptions texOpts(textureOptions);

    QSGTexture *texture = nullptr;
#if QT_CONFIG(webengine_vulkan)
    if (type() == Type::Vulkan) {
        VkImage image = vkImage(win);
        VkImageLayout layout = vkImageLayout();
        texture = QNativeInterface::QSGVulkanTexture::fromNative(image, layout, win, size(), texOpts);
    } else
#endif
    {
        GrGLTextureInfo info;
        if (m_frontBuffer->texture().getGLTextureInfo(&info))
            texture = QNativeInterface::QSGOpenGLTexture::fromNative(info.fID, win, size(), texOpts);
    }
    return texture;
}

bool DisplaySkiaOutputDevice::textureIsFlipped()
{
    return true;
}

QSize DisplaySkiaOutputDevice::size()
{
    return m_frontBuffer ? toQt(m_frontBuffer->shape().characterization.dimensions()) : QSize();
}

bool DisplaySkiaOutputDevice::requiresAlphaChannel()
{
    return m_requiresAlpha;
}

float DisplaySkiaOutputDevice::devicePixelRatio()
{
    return m_frontBuffer ? m_frontBuffer->shape().devicePixelRatio : 1;
}

#if QT_CONFIG(webengine_vulkan)
VkImage DisplaySkiaOutputDevice::vkImage(QQuickWindow *win)
{
    if (!m_frontBuffer)
        return VK_NULL_HANDLE;

    QSGRendererInterface *ri = win->rendererInterface();
    VkDevice qtVulkanDevice =
            *static_cast<VkDevice *>(ri->getResource(win, QSGRendererInterface::DeviceResource));
    QVulkanDeviceFunctions *df = win->vulkanInstance()->deviceFunctions(qtVulkanDevice);

    df->vkDestroyImage(qtVulkanDevice, m_importedImage, nullptr);
    df->vkFreeMemory(qtVulkanDevice, m_importedImageMemory, nullptr);

    if (df->vkCreateImage(qtVulkanDevice, m_frontBuffer->imageCreateInfo(), nullptr,
                          &m_importedImage)
        != VK_SUCCESS) {
        qFatal("VULKAN: Failed to create imported image!");
    }

    VkMemoryDedicatedAllocateInfoKHR dedicatedAllocateInfo = {
        VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR
    };
    dedicatedAllocateInfo.pNext = nullptr;
    dedicatedAllocateInfo.image = m_importedImage;
    dedicatedAllocateInfo.buffer = VK_NULL_HANDLE;

#if defined(Q_OS_WIN)
    VkImportMemoryWin32HandleInfoKHR importInfo = {
        VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR
    };
    importInfo.handle = m_frontBuffer->externalMemoryHandle();
#else
    VkImportMemoryFdInfoKHR importInfo = { VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR };
    importInfo.fd = m_frontBuffer->externalMemoryHandle();
#endif // defined(Q_OS_WIN)
    importInfo.pNext = &dedicatedAllocateInfo;
    importInfo.handleType = m_frontBuffer->externalMemoryHandleType();

    VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocateInfo.pNext = &importInfo;
    allocateInfo.allocationSize = m_frontBuffer->allocationSize();
    allocateInfo.memoryTypeIndex = m_frontBuffer->memoryTypeIndex();

    if (df->vkAllocateMemory(qtVulkanDevice, &allocateInfo, nullptr, &m_importedImageMemory)
        != VK_SUCCESS) {
        qFatal("VULKAN: Failed to allocate memory for imported VkImage!");
    }

    df->vkBindImageMemory(qtVulkanDevice, m_importedImage, m_importedImageMemory, 0);

    return m_importedImage;
}

VkImageLayout DisplaySkiaOutputDevice::vkImageLayout()
{
    if (!m_frontBuffer)
        return VK_IMAGE_LAYOUT_UNDEFINED;

    return m_frontBuffer->imageLayout();
}

void DisplaySkiaOutputDevice::releaseResources(QQuickWindow *win)
{
    VkDevice *vkDevicePtr = static_cast<VkDevice *>(
            win->rendererInterface()->getResource(win, QSGRendererInterface::DeviceResource));

    if (!vkDevicePtr) {
        Q_ASSERT(m_importedImage == VK_NULL_HANDLE && m_importedImageMemory == VK_NULL_HANDLE);
        return;
    }

    QVulkanDeviceFunctions *df = win->vulkanInstance()->deviceFunctions(*vkDevicePtr);

    if (m_importedImage != VK_NULL_HANDLE) {
        df->vkDestroyImage(*vkDevicePtr, m_importedImage, nullptr);
        m_importedImage = VK_NULL_HANDLE;
    }

    if (m_importedImageMemory != VK_NULL_HANDLE) {
        df->vkFreeMemory(*vkDevicePtr, m_importedImageMemory, nullptr);
        m_importedImageMemory = VK_NULL_HANDLE;
    }
}
#endif // QT_CONFIG(webengine_vulkan)

void DisplaySkiaOutputDevice::SwapBuffersFinished()
{
    {
        QMutexLocker locker(&m_mutex);
        std::swap(m_backBuffer, m_middleBuffer);
    }

    FinishSwapBuffers(gfx::SwapCompletionResult(gfx::SwapResult::SWAP_ACK),
                      gfx::Size(m_shape.characterization.width(), m_shape.characterization.height()),
                      std::move(m_frame));
}

} // namespace QtWebEngineCore
