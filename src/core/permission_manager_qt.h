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

#ifndef PERMISSION_MANAGER_QT_H
#define PERMISSION_MANAGER_QT_H

#include "base/callback.h"
#include "content/public/browser/permission_manager.h"
#include "browser_context_adapter.h"

#include <QHash>
#include <QList>

namespace QtWebEngineCore {

class PermissionManagerQt : public content::PermissionManager {

public:
    PermissionManagerQt();
    ~PermissionManagerQt();
    typedef BrowserContextAdapter::PermissionType PermissionType;

    void permissionRequestReply(const QUrl &origin, PermissionType type, bool reply);
    bool checkPermission(const QUrl &origin, PermissionType type);

    // content::PermissionManager implementation:
    void RequestPermission(
        content::PermissionType permission,
        content::RenderFrameHost* render_frame_host,
        int request_id,
        const GURL& requesting_origin,
        bool user_gesture,
        const base::Callback<void(content::PermissionStatus)>& callback) override;

    void CancelPermissionRequest(
        content::PermissionType permission,
        content::RenderFrameHost* render_frame_host,
        int request_id,
        const GURL& requesting_origin) override;

    content::PermissionStatus GetPermissionStatus(
        content::PermissionType permission,
        const GURL& requesting_origin,
        const GURL& embedding_origin) override;

    void ResetPermission(
        content::PermissionType permission,
        const GURL& requesting_origin,
        const GURL& embedding_origin) override;

    void RegisterPermissionUsage(
        content::PermissionType permission,
        const GURL& requesting_origin,
        const GURL& embedding_origin) override;

    int SubscribePermissionStatusChange(
        content::PermissionType permission,
        const GURL& requesting_origin,
        const GURL& embedding_origin,
        const base::Callback<void(content::PermissionStatus)>& callback) override;

    void UnsubscribePermissionStatusChange(int subscription_id) override;

private:
    QHash<QPair<QUrl, PermissionType>, bool> m_permissions;
    struct Request {
        int id;
        PermissionType type;
        QUrl origin;
        base::Callback<void(content::PermissionStatus)> callback;
    };
    QVector<Request> m_requests;
    struct Subscriber {
        int id;
        PermissionType type;
        QUrl origin;
        base::Callback<void(content::PermissionStatus)> callback;
    };
    int m_subscriberCount;
    QVector<Subscriber> m_subscribers;

};

} // namespace QtWebEngineCore

#endif // PERMISSION_MANAGER_QT_H
