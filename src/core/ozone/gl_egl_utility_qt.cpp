// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

// Based on the following implementations:
//  - //ui/ozone/platform/x11/gl_egl_egl_utility_x11.cc
//  - //ui/ozone/platform/wayland/gpu/wayland_gl_egl_utility.cc

// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gl_egl_utility_qt.h"

#include "base/containers/contains.h"

#include <algorithm>
#include <vector>
#include <optional>

#if BUILDFLAG(IS_OZONE_X11)
#include "ui/base/x/x11_gl_egl_utility.h"
#include "ui/base/x/x11_util.h"
#include "ui/gfx/linux/gpu_memory_buffer_support_x11.h"
#include "ui/gl/gl_implementation.h"

extern void *GetQtXDisplay();
#endif

// From ANGLE's egl/eglext.h. Follows the same approach as in
// ui/gl/gl_surface_egl.cc
#ifndef EGL_ANGLE_platform_angle_device_type_swiftshader
#define EGL_ANGLE_platform_angle_device_type_swiftshader
#define EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE 0x3487
#endif /* EGL_ANGLE_platform_angle_device_type_swiftshader */

#ifndef EGL_ANGLE_platform_angle
#define EGL_ANGLE_platform_angle 1
#define EGL_PLATFORM_ANGLE_NATIVE_PLATFORM_TYPE_ANGLE 0x348F
#define EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE 0x3209
#endif /* EGL_ANGLE_platform_angle */

#ifndef EGL_ANGLE_platform_angle_vulkan
#define EGL_ANGLE_platform_angle_vulkan 1
#define EGL_PLATFORM_VULKAN_DISPLAY_MODE_HEADLESS_ANGLE 0x34A5
#endif /* EGL_ANGLE_platform_angle_vulkan */

#ifndef EGL_ANGLE_platform_angle_device_type_egl_angle
#define EGL_ANGLE_platform_angle_device_type_egl_angle
#define EGL_PLATFORM_ANGLE_DEVICE_TYPE_EGL_ANGLE 0x348E
#endif /* EGL_ANGLE_platform_angle_device_type_egl_angle */

#ifndef EGL_ANGLE_platform_angle_opengl
#define EGL_ANGLE_platform_angle_opengl 1
#define EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE 0x320D
#define EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE 0x320E
#endif /* EGL_ANGLE_platform_angle_opengl */

namespace ui {

GLEGLUtilityQt::GLEGLUtilityQt() = default;
GLEGLUtilityQt::~GLEGLUtilityQt() = default;

void GLEGLUtilityQt::GetAdditionalEGLAttributes(EGLenum platform_type,
                                                std::vector<EGLAttrib> *display_attributes)
{
#if BUILDFLAG(IS_OZONE_X11)
    if (GetQtXDisplay()) {
        GetPlatformExtraDisplayAttribs(platform_type, display_attributes);
        return;
    }
#endif

    if (base::Contains(*display_attributes, EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE)) {
        display_attributes->push_back(EGL_PLATFORM_ANGLE_NATIVE_PLATFORM_TYPE_ANGLE);
        display_attributes->push_back(EGL_PLATFORM_VULKAN_DISPLAY_MODE_HEADLESS_ANGLE);
        return;
    }

#if defined(WAYLAND_GBM)
    display_attributes->push_back(EGL_PLATFORM_ANGLE_NATIVE_PLATFORM_TYPE_ANGLE);
    display_attributes->push_back(EGL_PLATFORM_GBM_KHR);
#endif // defined(WAYLAND_GBM)

    if (std::find(display_attributes->begin(), display_attributes->end(),
                  EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
                != display_attributes->end()
        || std::find(display_attributes->begin(), display_attributes->end(),
                     EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE)
                != display_attributes->end()) {
        display_attributes->push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
        display_attributes->push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_EGL_ANGLE);
        return;
    }
}

void GLEGLUtilityQt::ChooseEGLAlphaAndBufferSize(EGLint *alpha_size, EGLint *buffer_size)
{
#if BUILDFLAG(IS_OZONE_X11)
    if (GetQtXDisplay())
        ChoosePlatformCustomAlphaAndBufferSize(alpha_size, buffer_size);
#endif
}

void GLEGLUtilityQt::CollectGpuExtraInfo(bool enable_native_gpu_memory_buffers,
                                         gfx::GpuExtraInfo &gpu_extra_info) const
{
#if BUILDFLAG(IS_OZONE_X11)
    if (GetQtXDisplay()) {
        // TODO(crbug.com/40110388): Enable by default.
        if (enable_native_gpu_memory_buffers) {
            gpu_extra_info.gpu_memory_buffer_support_x11 =
                    ui::GpuMemoryBufferSupportX11::GetInstance()->supported_configs();
        }

        if (gl::GetGLImplementation() == gl::kGLImplementationEGLANGLE) {
            // ANGLE does not yet support EGL_EXT_image_dma_buf_import[_modifiers].
            gpu_extra_info.gpu_memory_buffer_support_x11.clear();
        }
    }
#endif
}

bool GLEGLUtilityQt::HasVisualManager()
{
#if BUILDFLAG(IS_OZONE_X11)
    if (GetQtXDisplay())
        return true;
#endif

    return false;
}

std::optional<base::ScopedEnvironmentVariableOverride>
GLEGLUtilityQt::MaybeGetScopedDisplayUnsetForVulkan()
{
#if BUILDFLAG(IS_OZONE_X11)
    // Unset DISPLAY env, so the vulkan can be initialized successfully, if the
    // X server doesn't support Vulkan surface.
    if (GetQtXDisplay() && !ui::IsVulkanSurfaceSupported())
        return std::optional<base::ScopedEnvironmentVariableOverride>("DISPLAY");
#endif

    return std::nullopt;
}

} // namespace ui
