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

#ifndef COMPOSITOR_RESOURCE_TRACKER_H
#define COMPOSITOR_RESOURCE_TRACKER_H

#include "compositor_resource.h"
#include "locked_ptr.h"

#include <base/callback.h>
#include <base/containers/flat_set.h>

#include <atomic>
#include <vector>

namespace viz {
class CompositorFrame;
struct ReturnedResource;
} // namespace viz

namespace gpu {
struct MailboxHolder;
} // namespace gpu

namespace QtWebEngineCore {

// Ensures resources are not used before they are ready.
//
// The life cycle of a frame's resources:
//
//   Step 1. A new CompositorFrame is received and given to submitResources().
//   The frame's resources will extracted and initialized to a pending state.
//
//   Step 2. Once the new resources are ready to be committed,
//   CompositorResourceTracker will notify the client by running the callback
//   given to submitResources().
//
//   Step 3. Once the client is ready to render, commitResources() should be
//   called. This will commit all the pending resources, making them available
//   via findResource().
//
//   Step 4. Once all the resources have been used (via findResource()),
//   returnResources() may be called to return a list of all the resources which
//   were *not* used since the last commitResources(). Go to step 1.
class CompositorResourceTracker final
{
public:
    CompositorResourceTracker();
    ~CompositorResourceTracker();

    void submitResources(const viz::CompositorFrame &frame, base::OnceClosure callback);
    void commitResources();
    std::vector<viz::ReturnedResource> returnResources();

    // The returned pointer is invalidated by the next call to commitFrame() or
    // returnResources(). It should therefore not be stored in data structures
    // but used immediately.
    //
    // Do not ask for resources which do not exist.
    const CompositorResource *findResource(CompositorResourceId id) const;

private:
    void updateBitmap(CompositorResource *resource);

    quint32 consumeMailbox(const gpu::MailboxHolder &mailboxHolder);

    bool scheduleUpdateMailbox(CompositorResource *resource);
    void updateMailbox(CompositorResource *resource);

    void scheduleUpdateMailboxes(std::vector<CompositorResource *> resources);
    void updateMailboxes(std::vector<CompositorResource *> resources);

    void scheduleRunSubmitCallback();
    void runSubmitCallback();

    base::flat_set<CompositorResource> m_committedResources;
    std::vector<CompositorResource> m_pendingResources;
    std::vector<CompositorResource *> m_pendingImports;
    base::OnceClosure m_submitCallback;
    std::atomic<size_t> m_pendingResourceUpdates{0};
    quint32 m_committedFrameId = 0;

    base::LockedPtrFactory<CompositorResourceTracker> m_weakPtrFactory{this};

    DISALLOW_COPY_AND_ASSIGN(CompositorResourceTracker);
};

} // namespace QtWebEngineCore

#endif // !COMPOSITOR_RESOURCE_TRACKER_H
