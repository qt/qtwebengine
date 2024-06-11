// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#if defined(USE_OZONE)
#include "surface_factory_qt.h"

#include "ozone/gl_context_qt.h"
#include "ozone/gl_ozone_egl_qt.h"
#if defined(USE_GLX)
#include "ozone/gl_ozone_glx_qt.h"
#endif

#include "qtwebenginecoreglobal_p.h"

#if QT_CONFIG(webengine_vulkan)
#include "compositor/vulkan_implementation_qt.h"
#endif

namespace QtWebEngineCore {

SurfaceFactoryQt::SurfaceFactoryQt()
{
#if defined(USE_GLX)
    if (GLContextHelper::getGlxPlatformInterface()) {
        m_impl = { gl::GLImplementationParts(gl::kGLImplementationDesktopGL),
                   gl::GLImplementationParts(gl::kGLImplementationDisabled) };
        m_ozone.reset(new ui::GLOzoneGLXQt());
    } else
#endif
    if (GLContextHelper::getEglPlatformInterface()) {
        m_impl = { gl::GLImplementationParts(gl::kGLImplementationDesktopGL),
                   gl::GLImplementationParts(gl::kGLImplementationEGLGLES2),
                   gl::GLImplementationParts(gl::kGLImplementationDisabled) };
        m_ozone.reset(new ui::GLOzoneEGLQt());
    } else {
        m_impl = { gl::GLImplementationParts(gl::kGLImplementationDisabled) };
    }
}

std::vector<gl::GLImplementationParts> SurfaceFactoryQt::GetAllowedGLImplementations()
{
    return m_impl;
}

ui::GLOzone *SurfaceFactoryQt::GetGLOzone(const gl::GLImplementationParts &implementation)
{
    return m_ozone.get();
}
#if BUILDFLAG(ENABLE_VULKAN)
std::unique_ptr<gpu::VulkanImplementation>
SurfaceFactoryQt::CreateVulkanImplementation(bool /*allow_protected_memory*/,
                                             bool /*enforce_protected_memory*/)
{
#if QT_CONFIG(webengine_vulkan)
    return std::make_unique<gpu::VulkanImplementationQt>();
#else
    return nullptr;
#endif
}
#endif

} // namespace QtWebEngineCore
#endif // defined(USE_OZONE)

