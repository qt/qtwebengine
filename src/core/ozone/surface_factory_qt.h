// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SURFACE_FACTORY_QT
#define SURFACE_FACTORY_QT

#if defined(USE_OZONE)

#include "ui/ozone/public/surface_factory_ozone.h"

namespace QtWebEngineCore {

class SurfaceFactoryQt : public ui::SurfaceFactoryOzone
{
public:
    SurfaceFactoryQt();
    std::vector<gl::GLImplementationParts> GetAllowedGLImplementations() override;
    ui::GLOzone *GetGLOzone(const gl::GLImplementationParts &implementation) override;
#if BUILDFLAG(ENABLE_VULKAN)
    std::unique_ptr<gpu::VulkanImplementation>
    CreateVulkanImplementation(bool allow_protected_memory, bool enforce_protected_memory) override;
#endif
    bool CanCreateNativePixmapForFormat(gfx::BufferFormat format) override;
    scoped_refptr<gfx::NativePixmap> CreateNativePixmap(
        gfx::AcceleratedWidget widget,
        gpu::VulkanDeviceQueue* device_queue,
        gfx::Size size,
        gfx::BufferFormat format,
        gfx::BufferUsage usage,
        absl::optional<gfx::Size> framebuffer_size = absl::nullopt) override;
    void CreateNativePixmapAsync(gfx::AcceleratedWidget widget,
                                 gpu::VulkanDeviceQueue* device_queue,
                                 gfx::Size size,
                                 gfx::BufferFormat format,
                                 gfx::BufferUsage usage,
                                 NativePixmapCallback callback) override;
    scoped_refptr<gfx::NativePixmap> CreateNativePixmapFromHandle(
        gfx::AcceleratedWidget widget,
        gfx::Size size,
        gfx::BufferFormat format,
        gfx::NativePixmapHandle handle) override;

private:
    std::vector<gl::GLImplementationParts> m_impl;
    std::unique_ptr<ui::GLOzone> m_ozone;
};

} // namespace QtWebEngineCore

#endif // defined(USE_OZONE)

#endif // SURFACE_FACTORY_QT

