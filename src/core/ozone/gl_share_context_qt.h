// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GL_SHARE_CONTEXT_QT
#define GL_SHARE_CONTEXT_QT

#include "ui/gl/gpu_timing.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_share_group.h"
#include "qtwebenginecoreglobal.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLContext)

namespace QtWebEngineCore {

class QtShareGLContext : public gl::GLContext
{

public:
    QtShareGLContext(QOpenGLContext *qtContext);
    void *GetHandle() override { return m_handle; }
    unsigned int CheckStickyGraphicsResetStatusImpl() override;
    // We don't care about the rest, this context shouldn't be used except for its handle.
    bool Initialize(gl::GLSurface *, const gl::GLContextAttribs &) override
    {
        Q_UNREACHABLE();
        return false;
    }
    bool MakeCurrentImpl(gl::GLSurface *) override
    {
        Q_UNREACHABLE();
        return false;
    }
    void ReleaseCurrent(gl::GLSurface *) override { Q_UNREACHABLE(); }
    bool IsCurrent(gl::GLSurface *) override
    {
        Q_UNREACHABLE();
        return false;
    }
    scoped_refptr<gl::GPUTimingClient> CreateGPUTimingClient() override { return nullptr; }
    const gfx::ExtensionSet &GetExtensions() override
    {
        static const gfx::ExtensionSet s_emptySet;
        return s_emptySet;
    }
    void ResetExtensions() override {}

private:
    void *m_handle;
};

class ShareGroupQt : public gl::GLShareGroup
{

public:
    gl::GLContext *GetContext() override { return m_shareContextQt.get(); }
    void AboutToAddFirstContext() override;

private:
    scoped_refptr<gl::GLContext> m_shareContextQt;
};
} // namespace
#endif
