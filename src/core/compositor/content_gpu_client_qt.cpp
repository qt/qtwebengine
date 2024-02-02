// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "content_gpu_client_qt.h"
#include "ozone/gl_share_context_qt.h"

namespace QtWebEngineCore {

ContentGpuClientQt::ContentGpuClientQt()
{
}

ContentGpuClientQt::~ContentGpuClientQt()
{
}

gl::GLShareGroup *ContentGpuClientQt::GetInProcessGpuShareGroup()
{
    if (!m_shareGroupQt.get())
        m_shareGroupQt = new ShareGroupQt;
    return m_shareGroupQt.get();
}

} // namespace
