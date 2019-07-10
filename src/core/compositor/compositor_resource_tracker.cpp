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

#include "compositor_resource_tracker.h"

#include "chromium_gpu_helper.h"
#include "render_widget_host_view_qt_delegate.h"
#include "web_engine_context.h"

#include "base/message_loop/message_loop.h"
#include "base/task/post_task.h"
#include "components/viz/common/quads/compositor_frame.h"
#include "components/viz/common/resources/returned_resource.h"
#include "components/viz/service/display_embedder/server_shared_bitmap_manager.h"
#include "content/browser/browser_main_loop.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/gpu/content_gpu_client.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/sync_point_manager.h"

namespace QtWebEngineCore {

CompositorResourceTracker::CompositorResourceTracker()
{}

CompositorResourceTracker::~CompositorResourceTracker()
{}

void CompositorResourceTracker::submitResources(const viz::CompositorFrame &frame, base::OnceClosure callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    DCHECK(!m_submitCallback);
    DCHECK(m_pendingResources.empty());
    DCHECK(m_pendingImports.empty());
    DCHECK(m_pendingResourceUpdates == 0);

    m_submitCallback = std::move(callback);

    m_pendingResources.reserve(frame.resource_list.size());
    m_pendingImports.reserve(frame.resource_list.size());

    for (const viz::TransferableResource &transferableResource : frame.resource_list) {
        auto it = m_committedResources.find(transferableResource.id);
        if (it != m_committedResources.end())
            m_pendingImports.push_back(&*it);
        else
            m_pendingResources.emplace_back(transferableResource);
    }

    if (m_pendingResources.empty()) {
        scheduleRunSubmitCallback();
        return;
    }

    m_pendingResourceUpdates = m_pendingResources.size();

    std::vector<CompositorResource *> batch;
    batch.reserve(m_pendingResources.size());

    for (CompositorResource &resource : m_pendingResources) {
        if (resource.is_software)
            updateBitmap(&resource);
        else if (!scheduleUpdateMailbox(&resource))
            batch.push_back(&resource);
    }

    if (!batch.empty())
        scheduleUpdateMailboxes(std::move(batch));
}

void CompositorResourceTracker::commitResources()
{
    // DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    //
    // This might be called from a Qt Quick render thread, but the UI thread
    // will still be blocked for the duration of this call.

    DCHECK(m_pendingResourceUpdates == 0);

    for (CompositorResource *resource : m_pendingImports)
        resource->import_count++;
    m_pendingImports.clear();

    m_committedResources.insert(std::make_move_iterator(m_pendingResources.begin()),
                                std::make_move_iterator(m_pendingResources.end()));
    m_pendingResources.clear();

    ++m_committedFrameId;
}

std::vector<viz::ReturnedResource> CompositorResourceTracker::returnResources()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    std::vector<viz::ReturnedResource> returnedResources;
    base::EraseIf(m_committedResources, [&](const CompositorResource &resource) {
        if (resource.last_used_for_frame != m_committedFrameId) {
            viz::ReturnedResource returnedResource;
            returnedResource.id = resource.id;
            returnedResource.count = resource.import_count;
            returnedResources.push_back(std::move(returnedResource));
            return true;
        }
        return false;
    });
    return returnedResources;
}

const CompositorResource *CompositorResourceTracker::findResource(CompositorResourceId id) const
{
    auto it = m_committedResources.find(id);
    DCHECK(it != m_committedResources.end());

    const_cast<CompositorResource &>(*it).last_used_for_frame = m_committedFrameId;

    return &*it;
}

void CompositorResourceTracker::updateBitmap(CompositorResource *resource)
{
    content::BrowserMainLoop *browserMainLoop = content::BrowserMainLoop::GetInstance();
    viz::ServerSharedBitmapManager *bitmapManager = browserMainLoop->GetServerSharedBitmapManager();

    resource->bitmap = bitmapManager->GetSharedBitmapFromId(
        resource->size,
        viz::BGRA_8888,
        resource->mailbox_holder.mailbox);

    if (--m_pendingResourceUpdates == 0)
        scheduleRunSubmitCallback();
}

quint32 CompositorResourceTracker::consumeMailbox(const gpu::MailboxHolder &mailboxHolder)
{
#if QT_CONFIG(opengl)
    gpu::MailboxManager *mailboxManager = mailbox_manager();
    DCHECK(mailboxManager);
    if (mailboxHolder.sync_token.HasData())
        mailboxManager->PullTextureUpdates(mailboxHolder.sync_token);
    gpu::TextureBase *tex = mailboxManager->ConsumeTexture(mailboxHolder.mailbox);
    return tex ? service_id(tex) : 0;
#else
    NOTREACHED();
#endif // QT_CONFIG(OPENGL)
}

bool CompositorResourceTracker::scheduleUpdateMailbox(CompositorResource *resource)
{
#if QT_CONFIG(opengl)
    gpu::SyncPointManager *syncPointManager = WebEngineContext::syncPointManager();
    DCHECK(syncPointManager);
    return syncPointManager->WaitOutOfOrder(
        resource->mailbox_holder.sync_token,
        base::BindOnce(&CompositorResourceTracker::updateMailbox,
                       m_weakPtrFactory.GetWeakPtr(),
                       resource));
#else
    NOTREACHED();
#endif // QT_CONFIG(OPENGL)
}

void CompositorResourceTracker::updateMailbox(CompositorResource *resource)
{
#if QT_CONFIG(opengl)
    resource->texture_id = consumeMailbox(resource->mailbox_holder);
    resource->texture_fence = CompositorResourceFence::create();

    if (--m_pendingResourceUpdates == 0)
        scheduleRunSubmitCallback();
#else
    NOTREACHED();
#endif // QT_CONFIG(OPENGL)
}

void CompositorResourceTracker::scheduleUpdateMailboxes(std::vector<CompositorResource *> resources)
{
#if QT_CONFIG(opengl)
    scoped_refptr<base::SingleThreadTaskRunner> gpuTaskRunner = gpu_task_runner();
    DCHECK(gpuTaskRunner);
    thread_local bool currentThreadIsGpu = gpuTaskRunner->BelongsToCurrentThread();
    if (currentThreadIsGpu)
        return updateMailboxes(std::move(resources));
    gpuTaskRunner->PostTask(
        FROM_HERE,
        base::BindOnce(&CompositorResourceTracker::updateMailboxes,
                       m_weakPtrFactory.GetWeakPtr(),
                       std::move(resources)));
#else
    NOTREACHED();
#endif // QT_CONFIG(OPENGL)
}

void CompositorResourceTracker::updateMailboxes(std::vector<CompositorResource *> resources)
{
#if QT_CONFIG(opengl)
    for (CompositorResource *resource : resources)
        resource->texture_id = consumeMailbox(resource->mailbox_holder);

    scoped_refptr<CompositorResourceFence> fence = CompositorResourceFence::create();

    for (CompositorResource *resource : resources)
        resource->texture_fence = fence;

    if ((m_pendingResourceUpdates -= resources.size()) == 0)
        scheduleRunSubmitCallback();
#else
    NOTREACHED();
#endif // QT_CONFIG(OPENGL)
}

void CompositorResourceTracker::scheduleRunSubmitCallback()
{
    thread_local bool currentThreadIsUi = content::BrowserThread::CurrentlyOn(content::BrowserThread::UI);
    if (currentThreadIsUi)
        return runSubmitCallback();
    base::PostTaskWithTraits(
        FROM_HERE, { content::BrowserThread::UI, base::TaskPriority::USER_VISIBLE },
        base::BindOnce(&CompositorResourceTracker::runSubmitCallback,
                       m_weakPtrFactory.GetWeakPtr()));
}

void CompositorResourceTracker::runSubmitCallback()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    std::move(m_submitCallback).Run();
}

} // namespace QtWebEngineCore
