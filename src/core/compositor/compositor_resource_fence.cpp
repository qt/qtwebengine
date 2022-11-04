// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "compositor_resource_fence.h"
#include "ozone/gl_surface_qt.h"
#include "ui/gl/gl_context.h"

#include <QtGui/qopenglcontext.h>

#ifndef GL_TIMEOUT_IGNORED
#define GL_TIMEOUT_IGNORED                0xFFFFFFFFFFFFFFFFull
#endif


#if QT_CONFIG(egl)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

namespace QtWebEngineCore {

void CompositorResourceFence::wait()
{
    if (!m_sync)
        return;

    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (!context)
        return;

    // Chromium uses its own GL bindings and stores in in thread local storage.
    // For that reason, let chromium_gpu_helper.cpp contain the producing code that will run in the Chromium
    // GPU thread, and put the sync consuming code here that will run in the QtQuick SG or GUI thread.
    switch (m_sync.type) {
    case gl::TransferableFence::NoSync:
        break;
    case gl::TransferableFence::EglSync:
#ifdef EGL_KHR_reusable_sync
    {
        static bool resolved = false;
        static PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR = 0;

        if (!resolved) {
            if (gl::GLSurfaceQt::HasEGLExtension("EGL_KHR_fence_sync"))
                eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC)context->getProcAddress("eglClientWaitSyncKHR");
            resolved = true;
        }

        if (eglClientWaitSyncKHR)
            // FIXME: Use the less wasteful eglWaitSyncKHR once we have a device that supports EGL_KHR_wait_sync.
            eglClientWaitSyncKHR(m_sync.egl.display, m_sync.egl.sync, 0, EGL_FOREVER_KHR);
    }
#endif
        break;
    case gl::TransferableFence::ArbSync:
        typedef void (QOPENGLF_APIENTRYP WaitSyncPtr)(GLsync sync, GLbitfield flags, GLuint64 timeout);
        static WaitSyncPtr glWaitSync_ = 0;
        if (!glWaitSync_) {
            glWaitSync_ = (WaitSyncPtr)context->getProcAddress("glWaitSync");
            Q_ASSERT(glWaitSync_);
        }
        glWaitSync_(m_sync.arb.sync, 0, GL_TIMEOUT_IGNORED);
        break;
    }

    release();
}

void CompositorResourceFence::release()
{
    if (!m_sync)
        return;

    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (!context)
        return;

    // Chromium uses its own GL bindings and stores in in thread local storage.
    // For that reason, let chromium_gpu_helper.cpp contain the producing code that will run in the Chromium
    // GPU thread, and put the sync consuming code here that will run in the QtQuick SG or GUI thread.
    switch (m_sync.type) {
    case gl::TransferableFence::NoSync:
        break;
    case gl::TransferableFence::EglSync:
#ifdef EGL_KHR_reusable_sync
    {
        static bool resolved = false;
        static PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR = 0;

        if (!resolved) {
            if (gl::GLSurfaceQt::HasEGLExtension("EGL_KHR_fence_sync"))
                eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC)context->getProcAddress("eglDestroySyncKHR");
            resolved = true;
        }

        if (eglDestroySyncKHR) {
            // FIXME: Use the less wasteful eglWaitSyncKHR once we have a device that supports EGL_KHR_wait_sync.
            eglDestroySyncKHR(m_sync.egl.display, m_sync.egl.sync);
            m_sync.reset();
        }
    }
#endif
        break;
    case gl::TransferableFence::ArbSync:
        typedef void (QOPENGLF_APIENTRYP DeleteSyncPtr)(GLsync sync);
        static DeleteSyncPtr glDeleteSync_ = 0;
        if (!glDeleteSync_) {
            glDeleteSync_ = (DeleteSyncPtr)context->getProcAddress("glDeleteSync");
            Q_ASSERT(glDeleteSync_);
        }
        glDeleteSync_(m_sync.arb.sync);
        m_sync.reset();
        break;
    }
    // If Chromium was able to create a sync, we should have been able to handle its type here too.
    Q_ASSERT(!m_sync);
}

// static
scoped_refptr<CompositorResourceFence> CompositorResourceFence::create(std::unique_ptr<gl::GLFence> glFence)
{
    if (!glFence && gl::GLContext::GetCurrent() && gl::GLFence::IsSupported())
        glFence = gl::GLFence::Create();
    if (glFence)
        return base::MakeRefCounted<CompositorResourceFence>(glFence->Transfer());
    return nullptr;
}

} // namespace QtWebEngineCore
