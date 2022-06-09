// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QUOTA_REQUEST_CONTROLLER_IMPL_H
#define QUOTA_REQUEST_CONTROLLER_IMPL_H

#include "quota_permission_context_qt.h"
#include "quota_request_controller.h"

namespace QtWebEngineCore {

class QuotaRequestControllerImpl final : public QuotaRequestController {
public:
    QuotaRequestControllerImpl(
        QuotaPermissionContextQt *context,
        const content::StorageQuotaParams &params,
        content::QuotaPermissionContext::PermissionCallback callback);

    ~QuotaRequestControllerImpl();

protected:
    void accepted() override;
    void rejected() override;

private:
    scoped_refptr<QuotaPermissionContextQt> m_context;
    content::QuotaPermissionContext::PermissionCallback m_callback;
};

} // namespace QtWebEngineCore

#endif // QUOTA_REQUEST_CONTROLLER_IMPL_H
