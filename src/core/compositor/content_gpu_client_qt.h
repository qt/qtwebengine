// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef CONTENT_GPU_CLIENT_QT_H
#define CONTENT_GPU_CLIENT_QT_H

#include "content/public/gpu/content_gpu_client.h"

namespace gl {
class GLShareGroup;
}

namespace QtWebEngineCore {
class ShareGroupQt;

class ContentGpuClientQt : public content::ContentGpuClient {
public:
    explicit ContentGpuClientQt();
    ~ContentGpuClientQt() override;

    // content::ContentGpuClient implementation.
    gl::GLShareGroup *GetInProcessGpuShareGroup() override;

private:
    scoped_refptr<ShareGroupQt> m_shareGroupQt;
};

}

#endif // CONTENT_GPU_CLIENT_QT_H
