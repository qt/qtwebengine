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

#include "compositor.h"

#include "compositor_resource_tracker.h"
#include "delegated_frame_node.h"

#include "base/task/post_task.h"
#include "components/viz/common/resources/returned_resource.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "services/viz/public/interfaces/compositing/compositor_frame_sink.mojom.h"

namespace QtWebEngineCore {

Compositor::Compositor(content::RenderWidgetHost *host)
    : m_resourceTracker(new CompositorResourceTracker)
    , m_host(host)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    m_taskRunner = base::CreateSingleThreadTaskRunnerWithTraits({content::BrowserThread::UI, base::TaskPriority::USER_VISIBLE});
    m_beginFrameSource =
        std::make_unique<viz::DelayBasedBeginFrameSource>(
            std::make_unique<viz::DelayBasedTimeSource>(m_taskRunner.get()),
            viz::BeginFrameSource::kNotRestartableId);
}

Compositor::~Compositor()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

void Compositor::setFrameSinkClient(viz::mojom::CompositorFrameSinkClient *frameSinkClient)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    if (m_frameSinkClient == frameSinkClient)
        return;

    // Accumulated resources belong to the old RendererCompositorFrameSink and
    // should not be returned.
    //
    // TODO(juvaldma): Can there be a pending frame from the old client?
    m_resourceTracker->returnResources();
    m_frameSinkClient = frameSinkClient;
}

void Compositor::setNeedsBeginFrames(bool needsBeginFrames)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    if (m_needsBeginFrames == needsBeginFrames)
        return;

    if (needsBeginFrames)
        m_beginFrameSource->AddObserver(this);
    else
        m_beginFrameSource->RemoveObserver(this);

    m_needsBeginFrames = needsBeginFrames;
}

void Compositor::submitFrame(viz::CompositorFrame frame, base::OnceClosure callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    DCHECK(!m_submitCallback);

    m_pendingFrame = std::move(frame);
    m_submitCallback = std::move(callback);
    m_resourceTracker->submitResources(
        m_pendingFrame,
        base::BindOnce(&Compositor::runSubmitCallback, base::Unretained(this)));
}

QSGNode *Compositor::updatePaintNode(QSGNode *oldNode, RenderWidgetHostViewQtDelegate *viewDelegate)
{
    // DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    //
    // This might be called from a Qt Quick render thread, but the UI thread
    // will still be blocked for the duration of this call.

    DelegatedFrameNode *frameNode = static_cast<DelegatedFrameNode *>(oldNode);
    if (!frameNode)
        frameNode = new DelegatedFrameNode;

    if (!m_updatePaintNodeShouldCommit) {
        frameNode->commit(m_committedFrame, viz::CompositorFrame(), m_resourceTracker.get(), viewDelegate);
        return frameNode;
    }
    m_updatePaintNodeShouldCommit = false;

    gfx::PresentationFeedback dummyFeedback(base::TimeTicks::Now(), base::TimeDelta(), gfx::PresentationFeedback::Flags::kVSync);
    m_presentations.insert({m_committedFrame.metadata.frame_token, dummyFeedback});

    m_resourceTracker->commitResources();
    frameNode->commit(m_pendingFrame, m_committedFrame, m_resourceTracker.get(), viewDelegate);
    m_committedFrame = std::move(m_pendingFrame);
    m_pendingFrame = viz::CompositorFrame();

    m_taskRunner->PostTask(FROM_HERE,
                           base::BindOnce(&Compositor::notifyFrameCommitted, m_weakPtrFactory.GetWeakPtr()));

    return frameNode;
}

void Compositor::runSubmitCallback()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    m_updatePaintNodeShouldCommit = true;
    std::move(m_submitCallback).Run();
}

void Compositor::notifyFrameCommitted()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    m_beginFrameSource->DidFinishFrame(this);
    if (m_frameSinkClient)
        m_frameSinkClient->DidReceiveCompositorFrameAck(m_resourceTracker->returnResources());
}

void Compositor::sendPresentationFeedback(uint frame_token)
{
    gfx::PresentationFeedback dummyFeedback(base::TimeTicks::Now(), base::TimeDelta(), gfx::PresentationFeedback::Flags::kVSync);
    m_presentations.insert({frame_token, dummyFeedback});
}

bool Compositor::OnBeginFrameDerivedImpl(const viz::BeginFrameArgs &args)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    ProgressFlingIfNeeded(m_host, args.frame_time);
    m_beginFrameSource->OnUpdateVSyncParameters(args.frame_time, args.interval);
    if (m_frameSinkClient) {
        m_frameSinkClient->OnBeginFrame(args, m_presentations);
        m_presentations.clear();
    }

    return true;
}

void Compositor::OnBeginFrameSourcePausedChanged(bool)
{
    // Ignored for now.  If the begin frame source is paused, the renderer
    // doesn't need to be informed about it and will just not receive more
    // begin frames.
}

} // namespace QtWebEngineCore
