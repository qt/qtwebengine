// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QUOTA_PERMISSION_CONTEXT_QT_H
#define QUOTA_PERMISSION_CONTEXT_QT_H

#include "content/public/browser/quota_permission_context.h"

namespace QtWebEngineCore {

class QuotaPermissionContextQt : public content::QuotaPermissionContext {
public:
    void RequestQuotaPermission(const content::StorageQuotaParams &params,
                                int render_process_id,
                                PermissionCallback callback) override;

    void dispatchCallbackOnIOThread(PermissionCallback callback,
                                    QuotaPermissionContext::QuotaPermissionResponse response);
};

} // namespace QtWebEngineCore

#endif // QUOTA_PERMISSION_CONTEXT_QT_H
