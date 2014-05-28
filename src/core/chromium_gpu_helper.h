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

#ifndef CHROMIUM_GPU_HELPER_H
#define CHROMIUM_GPU_HELPER_H

#include <QtGlobal> // We need this for the Q_OS_QNX define.

#include "base/callback.h"

namespace base {
class MessageLoop;
}

namespace content {
class SyncPointManager;
}

namespace gpu {
namespace gles2 {
class MailboxManager;
struct MailboxName;
class Texture;
}
}

typedef void *EGLDisplay;
typedef void *EGLSyncKHR;
typedef struct __GLsync *GLsync;

union FenceSync {
    enum SyncType {
        NoSync,
        EglSync,
        ArbSync
    };
    SyncType type;
    struct {
        SyncType type;
        EGLDisplay display;
        EGLSyncKHR sync;
    } egl;
    struct {
        SyncType type;
        GLsync sync;
    } arb;

    FenceSync() : type(NoSync) { }
    operator bool() { return type != NoSync; }
    void reset() { type = NoSync; }
};

// These functions wrap code that needs to include headers that are
// incompatible with Qt GL headers.
// From the outside, types from incompatible headers referenced in these
// functions should only be forward-declared and considered as opaque types.

FenceSync createFence();
base::MessageLoop *gpu_message_loop();
content::SyncPointManager *sync_point_manager();
gpu::gles2::MailboxManager *mailbox_manager();

void AddSyncPointCallbackOnGpuThread(base::MessageLoop *gpuMessageLoop, content::SyncPointManager *syncPointManager, uint32 sync_point, const base::Closure& callback);
gpu::gles2::Texture* ConsumeTexture(gpu::gles2::MailboxManager *mailboxManager, unsigned target, const gpu::gles2::MailboxName& name);
unsigned int service_id(gpu::gles2::Texture *tex);

#ifdef Q_OS_QNX
typedef void* EGLDisplay;
typedef void* EGLStreamKHR;

struct EGLStreamData {
    EGLDisplay egl_display;
    EGLStreamKHR egl_str_handle;

    EGLStreamData(): egl_display(NULL), egl_str_handle(NULL) {}
};

EGLStreamData eglstream_connect_consumer(gpu::gles2::Texture *tex);
#endif

#endif // CHROMIUM_GPU_HELPER_H
