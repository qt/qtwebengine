// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "web_engine_context.h"

#include "base/functional/bind.h"
#include "base/task/thread_pool.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread_restrictions.h"
#include "content/browser/gpu/gpu_main_thread_factory.h"
#include "content/browser/renderer_host/render_process_host_impl.h"
#include "content/browser/utility_process_host.h"
#include "content/child/child_process.h"
#include "content/gpu/gpu_child_thread.h"
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
        content::GetUIThreadTaskRunner({})->PostTask(
                FROM_HERE,
                base::BindOnce(&GpuThreadControllerQt::createGpuProcess, params, gpuPreferences));
    }
    ~GpuThreadControllerQt() override
    {
        content::GetUIThreadTaskRunner({})->PostTask(
                FROM_HERE,
                base::BindOnce(&GpuThreadControllerQt::destroyGpuProcess));
    }

    static void createGpuProcess(
            const content::InProcessChildThreadParams &params,
            const gpu::GpuPreferences &gpuPreferences)
    {
        DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
        if (s_gpuProcessDestroyed)
            return;

        s_gpuProcess = std::make_unique<content::ChildProcess>(base::ThreadType::kDefault);
        auto gpuInit = std::make_unique<gpu::GpuInit>();
        gpuInit->InitializeInProcess(base::CommandLine::ForCurrentProcess(), gpuPreferences);
        auto childThread = new content::GpuChildThread(params, std::move(gpuInit));
        childThread->Init(base::TimeTicks::Now());
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

    static std::unique_ptr<content::ChildProcess> s_gpuProcess;
    static bool s_gpuProcessDestroyed;
};

std::unique_ptr<content::ChildProcess> GpuThreadControllerQt::s_gpuProcess;
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
