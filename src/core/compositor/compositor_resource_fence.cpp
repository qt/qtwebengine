/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "compositor_resource_fence.h"
#include "ozone/gl_surface_qt.h"
#include "ui/gl/gl_context.h"

#include <QtGui/private/qtguiglobal_p.h>
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
    Q_ASSERT(context);

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
