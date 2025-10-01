// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef CONTENT_GPU_CLIENT_QT_H
#define CONTENT_GPU_CLIENT_QT_H

#include "base/memory/scoped_refptr.h"
#include "base/task/single_thread_task_runner.h"
#include "content/public/gpu/content_gpu_client.h"
#include "gpu/config/gpu_preferences.h"

#include <QtCore/qscopedpointer.h>

namespace gpu {
class GpuDriverBugWorkarounds;
}

namespace mojo {
class BinderMap;
}

namespace QtWebEngineCore {

class GpuObserver;

class ContentGpuClientQt : public content::ContentGpuClient
{
public:
    ContentGpuClientQt();
    ~ContentGpuClientQt();

    gpu::GpuPreferences gpuPreferences() const { return m_gpuPreferences; }
    scoped_refptr<base::SingleThreadTaskRunner> gpuTaskRunner() const { return m_gpuTaskRunner; }

    // Overridden from content::ContentGpuClient:
    void GpuServiceInitialized() override;
    void ExposeInterfacesToBrowser(viz::GpuServiceImpl *gpu_service,
                                   const gpu::GpuPreferences &gpu_preferences,
                                   const gpu::GpuDriverBugWorkarounds &gpu_workarounds,
                                   mojo::BinderMap *binders) override;

private:
    QScopedPointer<GpuObserver> m_gpuObserver;
    gpu::GpuPreferences m_gpuPreferences;
    scoped_refptr<base::SingleThreadTaskRunner> m_gpuTaskRunner;
};

} // namespace QtWebEngineCore

#endif // CONTENT_GPU_CLIENT_QT_H
