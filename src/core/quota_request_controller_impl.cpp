// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "quota_request_controller_impl.h"

#include "type_conversion.h"

namespace QtWebEngineCore {

QuotaRequestControllerImpl::QuotaRequestControllerImpl(QuotaPermissionContextQt *context,
    const content::StorageQuotaParams &params,
    content::QuotaPermissionContext::PermissionCallback callback)
    : QuotaRequestController(
        toQt(params.origin_url),
        params.requested_size)
    , m_context(context)
    , m_callback(std::move(callback))
{}

QuotaRequestControllerImpl::~QuotaRequestControllerImpl()
{
    if (m_callback)
        m_context->dispatchCallbackOnIOThread(std::move(m_callback), content::QuotaPermissionContext::QUOTA_PERMISSION_RESPONSE_CANCELLED);
}

void QuotaRequestControllerImpl::accepted()
{
    m_context->dispatchCallbackOnIOThread(std::move(m_callback), content::QuotaPermissionContext::QUOTA_PERMISSION_RESPONSE_ALLOW);
}

void QuotaRequestControllerImpl::rejected()
{
    m_context->dispatchCallbackOnIOThread(std::move(m_callback), content::QuotaPermissionContext::QUOTA_PERMISSION_RESPONSE_DISALLOW);
}

} // namespace QtWebEngineCore
