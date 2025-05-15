// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef GL_OZONE_QT_H
#define GL_OZONE_QT_H

#include "ui/ozone/common/gl_ozone_egl.h"

namespace ui {

// Based on //ui/ozone/platform/x11/x11_surface_factory.cc
enum class NativePixmapSupportType {
    // Importing native pixmaps not supported.
    kNone,

    // Native pixmaps are imported directly into EGL using the
    // EGL_EXT_image_dma_buf_import extension.
    kDMABuf,

    // Native pixmaps are first imported as X11 pixmaps using DRI3 and then into
    // EGL.
    kX11Pixmap,
};

class GLOzoneQt : public GLOzoneEGL
{
public:
    static NativePixmapSupportType getNativePixmapSupportType();

    bool InitializeStaticGLBindings(const gl::GLImplementationParts &implementation) override;
    bool InitializeExtensionSettingsOneOffPlatform(gl::GLDisplay *display) override;
    scoped_refptr<gl::GLSurface> CreateViewGLSurface(gl::GLDisplay *display,
                                                     gfx::AcceleratedWidget window) override;
    scoped_refptr<gl::GLSurface> CreateOffscreenGLSurface(gl::GLDisplay *display,
                                                          const gfx::Size &size) override;
    bool CanImportNativePixmap(gfx::BufferFormat format) override;
    std::unique_ptr<NativePixmapGLBinding>
    ImportNativePixmap(scoped_refptr<gfx::NativePixmap> pixmap, gfx::BufferFormat plane_format,
                       gfx::BufferPlane plane, gfx::Size plane_size,
                       const gfx::ColorSpace &color_space, GLenum target,
                       GLuint texture_id) override;

protected:
    // Returns native platform display handle. This is used to obtain the EGL
    // display connection for the native display.
    gl::EGLDisplayPlatform GetNativeDisplay() override;

    // Sets up GL bindings for the native surface.
    bool LoadGLES2Bindings(const gl::GLImplementationParts &implementation) override;
};

class GLOzoneANGLEQt : public GLOzoneQt
{
protected:
    bool LoadGLES2Bindings(const gl::GLImplementationParts &implementation) override;
};

class GLOzoneEGLQt : public GLOzoneQt
{
public:
    void ShutdownGL(gl::GLDisplay *display) override;

protected:
    bool LoadGLES2Bindings(const gl::GLImplementationParts &implementation) override;

private:
    void *m_nativeEGLHandle = nullptr;
};

} // namespace ui

#endif // GL_OZONE_QT_H
