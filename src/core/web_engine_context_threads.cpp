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

#include "web_engine_context.h"

#include "base/bind.h"
#include "base/task/post_task.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread_restrictions.h"
#include "content/browser/gpu/gpu_main_thread_factory.h"
#include "content/browser/renderer_host/render_process_host_impl.h"
#include "content/browser/utility_process_host.h"
#include "content/gpu/gpu_child_thread.h"
#include "content/gpu/gpu_process.h"
#include "content/gpu/in_process_gpu_thread.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/renderer/in_process_renderer_thread.h"
#include "content/utility/in_process_utility_thread.h"
#include "gpu/ipc/service/gpu_init.h"

#include <memory>

#include <QEventLoop>

namespace QtWebEngineCore {

struct GpuThreadControllerQt : content::GpuThreadController
{
    GpuThreadControllerQt(const content::InProcessChildThreadParams &params, const gpu::GpuPreferences &gpuPreferences)
    {
        base::PostTask(
                FROM_HERE, { content::BrowserThread::UI },
                base::BindOnce(&GpuThreadControllerQt::createGpuProcess, params, gpuPreferences));
    }
    ~GpuThreadControllerQt() override
    {
        base::PostTask(
                FROM_HERE, { content::BrowserThread::UI },
                base::BindOnce(&GpuThreadControllerQt::destroyGpuProcess));
    }

    static void createGpuProcess(
            const content::InProcessChildThreadParams &params,
            const gpu::GpuPreferences &gpuPreferences)
    {
        DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
        if (s_gpuProcessDestroyed)
            return;

        s_gpuProcess = std::make_unique<content::GpuProcess>(base::ThreadPriority::DISPLAY);
        auto gpuInit = std::make_unique<gpu::GpuInit>();
        gpuInit->InitializeInProcess(base::CommandLine::ForCurrentProcess(), gpuPreferences);
        auto childThread = new content::GpuChildThread(params, std::move(gpuInit));
        childThread->Init(base::Time::Now());
        s_gpuProcess->set_main_thread(childThread);
    }

    static void destroyGpuProcess()
    {
        DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
        if (s_gpuProcessDestroyed)
            return;

        // viz::GpuServiceImpl::~GpuServiceImpl waits for io task.
        base::ScopedAllowBaseSyncPrimitivesForTesting allow;
        s_gpuProcess.reset();
        s_gpuProcessDestroyed = true;
    }

    static std::unique_ptr<content::GpuProcess> s_gpuProcess;
    static bool s_gpuProcessDestroyed;
};

std::unique_ptr<content::GpuProcess> GpuThreadControllerQt::s_gpuProcess;
bool GpuThreadControllerQt::s_gpuProcessDestroyed = false;

static std::unique_ptr<content::GpuThreadController> createGpuThreadController(
        const content::InProcessChildThreadParams &params,
        const gpu::GpuPreferences &gpuPreferences)
{
    return std::make_unique<GpuThreadControllerQt>(params, gpuPreferences);
}

// static
void WebEngineContext::destroyGpuProcess()
{
    GpuThreadControllerQt::destroyGpuProcess();
}

// static
void WebEngineContext::registerMainThreadFactories()
{
    content::UtilityProcessHost::RegisterUtilityMainThreadFactory(content::CreateInProcessUtilityThread);
    content::RenderProcessHostImpl::RegisterRendererMainThreadFactory(content::CreateInProcessRendererThread);
    if (!isGpuServiceOnUIThread())
        content::RegisterGpuMainThreadFactory(content::CreateInProcessGpuThread);
    else
        content::RegisterGpuMainThreadFactory(createGpuThreadController);
}

} // namespace QtWebEngineCore
