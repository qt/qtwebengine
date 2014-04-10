/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromium_gpu_helper.h"

#include "content/common/gpu/gpu_channel_manager.h"
#include "content/common/gpu/sync_point_manager.h"
#include "content/gpu/gpu_child_thread.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/texture_manager.h"

static void addSyncPointCallbackDelegate(content::SyncPointManager *syncPointManager, uint32 sync_point, const base::Closure& callback)
{
    syncPointManager->AddSyncPointCallback(sync_point, callback);
}

FenceSync createFence()
{
    FenceSync ret;
    // Logic taken from chromium/ui/gl/gl_fence.cc
#if !defined(OS_MACOSX)
    if (gfx::g_driver_egl.ext.b_EGL_KHR_fence_sync) {
        ret.type = FenceSync::EglSync;
        ret.egl.display = eglGetCurrentDisplay();
        ret.egl.sync = eglCreateSyncKHR(ret.egl.display, EGL_SYNC_FENCE_KHR, NULL);
    } else
#endif
    if (gfx::g_driver_gl.ext.b_GL_ARB_sync) {
        ret.type = FenceSync::ArbSync;
        ret.arb.sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    // glFlush is necessary to make sure that our fence creation reaches the GL server
    // before we try waiting on it from a different context, which could deadlock.
    // In cases where no fence extension is available, this also serves as flushing
    // Chromium's GL context command stream before yielding to the SG thread.
    glFlush();
    return ret;
}

base::MessageLoop *gpu_message_loop()
{
    return content::GpuChildThread::instance()->message_loop();
}

content::SyncPointManager *sync_point_manager()
{
    content::GpuChannelManager *gpuChannelManager = content::GpuChildThread::instance()->ChannelManager();
    return gpuChannelManager->sync_point_manager();
}

void AddSyncPointCallbackOnGpuThread(base::MessageLoop *gpuMessageLoop, content::SyncPointManager *syncPointManager, uint32 sync_point, const base::Closure& callback)
{
    // We need to set our callback from the GPU thread, where the SyncPointManager lives.
    gpuMessageLoop->PostTask(FROM_HERE, base::Bind(&addSyncPointCallbackDelegate, make_scoped_refptr(syncPointManager), sync_point, callback));
}

gpu::gles2::MailboxManager *mailbox_manager()
{
    content::GpuChannelManager *gpuChannelManager = content::GpuChildThread::instance()->ChannelManager();
    return gpuChannelManager->mailbox_manager();
}

gpu::gles2::Texture* ConsumeTexture(gpu::gles2::MailboxManager *mailboxManager, unsigned target, const gpu::gles2::MailboxName& name)
{
    return mailboxManager->ConsumeTexture(target, name);
}

unsigned int service_id(gpu::gles2::Texture *tex)
{
    return tex->service_id();
}
