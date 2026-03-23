// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef GL_EGL_UTILITY_QT_H
#define GL_EGL_UTILITY_QT_H

#include "ui/ozone/public/platform_gl_egl_utility.h"

namespace ui {

class GLEGLUtilityQt : public PlatformGLEGLUtility
{
public:
    GLEGLUtilityQt();
    ~GLEGLUtilityQt() override;
    GLEGLUtilityQt(const GLEGLUtilityQt &util) = delete;
    GLEGLUtilityQt &operator=(const GLEGLUtilityQt &util) = delete;

    // PlatformGLEGLUtility overrides:
    void GetAdditionalEGLAttributes(EGLenum platform_type,
                                    std::vector<EGLAttrib> *display_attributes) override;
    void ChooseEGLAlphaAndBufferSize(EGLint *alpha_size, EGLint *buffer_size) override;
    void CollectGpuExtraInfo(bool enable_native_gpu_memory_buffers,
                             gfx::GpuExtraInfo &gpu_extra_info) const override;
    bool HasVisualManager() override;
    std::optional<base::ScopedEnvironmentVariableOverride>
    MaybeGetScopedDisplayUnsetForVulkan() override;
};

} // namespace ui

#endif // GL_EGL_UTILITY_QT_H
