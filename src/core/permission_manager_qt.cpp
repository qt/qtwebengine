/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "permission_manager_qt.h"

#include "content/browser/renderer_host/render_view_host_delegate.h"
#include "content/public/browser/permission_type.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

#include "type_conversion.h"
#include "web_contents_delegate_qt.h"

namespace QtWebEngineCore {

BrowserContextAdapter::PermissionType toQt(content::PermissionType type)
{
    switch (type) {
    case content::PermissionType::GEOLOCATION:
        return BrowserContextAdapter::GeolocationPermission;
    case content::PermissionType::NOTIFICATIONS:
    case content::PermissionType::MIDI_SYSEX:
    case content::PermissionType::PUSH_MESSAGING:
    case content::PermissionType::PROTECTED_MEDIA_IDENTIFIER:
    case content::PermissionType::NUM:
        break;
    }
    return BrowserContextAdapter::UnsupportedPermission;
}

PermissionManagerQt::PermissionManagerQt()
    : m_subscriberCount(0)
{
}

PermissionManagerQt::~PermissionManagerQt()
{
}

void PermissionManagerQt::permissionRequestReply(const QUrl &origin, BrowserContextAdapter::PermissionType type, bool reply)
{
    QPair<QUrl, BrowserContextAdapter::PermissionType> key(origin, type);
    m_permissions[key] = reply;
    content::PermissionStatus status = reply ? content::PERMISSION_STATUS_GRANTED : content::PERMISSION_STATUS_DENIED;
    auto it = m_requests.begin();
    while (it != m_requests.end()) {
        if (it->origin == origin && it->type == type) {
            it->callback.Run(status);
            it = m_requests.erase(it);
        } else
            ++it;
    }
    Q_FOREACH (const Subscriber &subscriber, m_subscribers) {
        if (subscriber.origin == origin && subscriber.type == type)
            subscriber.callback.Run(status);
    }
}

bool PermissionManagerQt::checkPermission(const QUrl &origin, BrowserContextAdapter::PermissionType type)
{
    QPair<QUrl, BrowserContextAdapter::PermissionType> key(origin, type);
    return m_permissions.contains(key) && m_permissions[key];
}

void PermissionManagerQt::RequestPermission(content::PermissionType permission,
                                            content::RenderFrameHost *frameHost,
                                            int request_id,
                                            const GURL& requesting_origin,
                                            bool user_gesture,
                                            const base::Callback<void(content::PermissionStatus)>& callback)
{
    Q_UNUSED(user_gesture);
    BrowserContextAdapter::PermissionType permissionType = toQt(permission);
    if (permissionType == BrowserContextAdapter::UnsupportedPermission) {
        callback.Run(content::PERMISSION_STATUS_DENIED);
        return;
    }

    content::WebContents *webContents = frameHost->GetRenderViewHost()->GetDelegate()->GetAsWebContents();
    WebContentsDelegateQt* contentsDelegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());
    Q_ASSERT(contentsDelegate);
    Request request = {
        request_id,
        permissionType,
        toQt(requesting_origin),
        callback
    };
    m_requests.append(request);
    if (permissionType == BrowserContextAdapter::GeolocationPermission)
        contentsDelegate->requestGeolocationPermission(request.origin);
}

void PermissionManagerQt::CancelPermissionRequest(content::PermissionType permission,
                                                  content::RenderFrameHost *frameHost,
                                                  int request_id,
                                                  const GURL& requesting_origin)
{
    Q_UNUSED(frameHost);
    const BrowserContextAdapter::PermissionType permissionType = toQt(permission);
    if (permissionType == BrowserContextAdapter::UnsupportedPermission)
        return;

    // Should we add API to cancel permissions in the UI level?
    const QUrl origin = toQt(requesting_origin);
    auto it = m_requests.begin();
    while (it != m_requests.end()) {
        if (it->id == request_id && it->type == permissionType && it->origin == origin) {
            m_requests.erase(it);
            return;
        }
    }
    qWarning() << "PermissionManagerQt::CancelPermissionRequest called on unknown request" << request_id << origin << permissionType;
}

content::PermissionStatus PermissionManagerQt::GetPermissionStatus(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/)
{
    const BrowserContextAdapter::PermissionType permissionType = toQt(permission);
    if (permissionType == BrowserContextAdapter::UnsupportedPermission)
        return content::PERMISSION_STATUS_DENIED;

    QPair<QUrl, BrowserContextAdapter::PermissionType> key(toQt(requesting_origin), permissionType);
    if (!m_permissions.contains(key))
        return content::PERMISSION_STATUS_ASK;
    if (m_permissions[key])
        return content::PERMISSION_STATUS_GRANTED;
    return content::PERMISSION_STATUS_DENIED;
}

void PermissionManagerQt::ResetPermission(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/)
{
    const BrowserContextAdapter::PermissionType permissionType = toQt(permission);
    if (permissionType == BrowserContextAdapter::UnsupportedPermission)
        return;

    QPair<QUrl, BrowserContextAdapter::PermissionType> key(toQt(requesting_origin), permissionType);
    m_permissions.remove(key);
}

void PermissionManagerQt::RegisterPermissionUsage(
    content::PermissionType /*permission*/,
    const GURL& /*requesting_origin*/,
    const GURL& /*embedding_origin*/)
{
    // We do not currently track which permissions are used.
}

int PermissionManagerQt::SubscribePermissionStatusChange(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/,
    const base::Callback<void(content::PermissionStatus)>& callback)
{
    Subscriber subscriber = {
        m_subscriberCount++,
        toQt(permission),
        toQt(requesting_origin),
        callback
    };
    m_subscribers.append(subscriber);
    return subscriber.id;
}

void PermissionManagerQt::UnsubscribePermissionStatusChange(int subscription_id)
{
    for (int i = 0; i < m_subscribers.count(); i++) {
        if (m_subscribers[i].id == subscription_id) {
            m_subscribers.removeAt(i);
            return;
        }
    }
    qWarning() << "PermissionManagerQt::UnsubscribePermissionStatusChange called on unknown subscription id" << subscription_id;
}

} // namespace QtWebEngineCore
