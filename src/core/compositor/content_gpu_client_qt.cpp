// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "content_gpu_client_qt.h"

#include "web_engine_context.h"

namespace QtWebEngineCore {

ContentGpuClientQt::ContentGpuClientQt()
{
}

ContentGpuClientQt::~ContentGpuClientQt()
{
}

gpu::SyncPointManager *ContentGpuClientQt::GetSyncPointManager()
{
    return WebEngineContext::syncPointManager();
}

} // namespace
