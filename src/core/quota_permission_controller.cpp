/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "quota_permission_controller.h"
#include "quota_permission_controller_p.h"

#include "quota_permission_context_qt.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

QuotaPermissionControllerPrivate::QuotaPermissionControllerPrivate(QuotaPermissionContextQt *context,
                                                                   const content::StorageQuotaParams &params,
                                                                   const content::QuotaPermissionContext::PermissionCallback &callback)
    : m_context(context),
      m_originUrl(toQt(params.origin_url)),
      m_requestedSize(params.requested_size),
      m_callback(callback)
{
}

QuotaPermissionControllerPrivate::~QuotaPermissionControllerPrivate()
{
}

QuotaPermissionController::QuotaPermissionController(QuotaPermissionControllerPrivate *controllerPrivate)
    : d(controllerPrivate)
    , m_answered(false)
{
}

QuotaPermissionController::~QuotaPermissionController()
{
}

void QuotaPermissionController::accept()
{
    if (!m_answered) {
        d->m_context->dispatchCallbackOnIOThread(d->m_callback, QuotaPermissionContextQt::QUOTA_PERMISSION_RESPONSE_ALLOW);
        m_answered = true;
    }
}

void QuotaPermissionController::cancel()
{
    if (!m_answered) {
        d->m_context->dispatchCallbackOnIOThread(d->m_callback, QuotaPermissionContextQt::QUOTA_PERMISSION_RESPONSE_DISALLOW);
        m_answered = true;
    }
}

QUrl QuotaPermissionController::origin()
{
    return d->m_originUrl;
}

qint64 QuotaPermissionController::requestedSize()
{
    return d->m_requestedSize;
}

} // namespace QtWebEngineCore
