// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef COMPOSITOR_RESOURCE_FENCE_H
#define COMPOSITOR_RESOURCE_FENCE_H

#include <base/memory/ref_counted.h>
#include <ui/gl/gl_fence.h>

namespace QtWebEngineCore {

// Sync object created on GPU thread and consumed on render thread.
class CompositorResourceFence final : public base::RefCountedThreadSafe<CompositorResourceFence>
{
public:
    REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE();

    CompositorResourceFence() {}
    CompositorResourceFence(const gl::TransferableFence &sync) : m_sync(sync) {}
    ~CompositorResourceFence() { release(); }

    // May be used only by Qt Quick render thread.
    void wait();
    void release();

    // May be used only by GPU thread.
    static scoped_refptr<CompositorResourceFence> create(std::unique_ptr<gl::GLFence> glFence = nullptr);

private:
    gl::TransferableFence m_sync;
};

} // namespace QtWebEngineCore

#endif // !COMPOSITOR_RESOURCE_FENCE_H
