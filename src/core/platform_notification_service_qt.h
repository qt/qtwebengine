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

#ifndef PLATFORM_NOTIFICATION_SERVICE_QT_H
#define PLATFORM_NOTIFICATION_SERVICE_QT_H

#include "content/public/browser/platform_notification_service.h"

namespace content {
class BrowserContext;
}

namespace QtWebEngineCore {

class PlatformNotificationServiceQt : public content::PlatformNotificationService {
public:
    PlatformNotificationServiceQt(content::BrowserContext *browserContext);
    ~PlatformNotificationServiceQt() override;

    // Displays the notification described in |notification_data| to the user. A
    // closure through which the notification can be closed will be stored in the
    // |cancel_callback| argument. This method must be called on the UI thread.
    void DisplayNotification(const std::string& notification_id,
                             const GURL& origin,
                             const blink::PlatformNotificationData& notificationData,
                             const blink::NotificationResources& notificationResources) override;

    // Displays the persistent notification described in |notification_data| to
    // the user. This method must be called on the UI thread.
    void DisplayPersistentNotification(const std::string& notification_id,
                                       const GURL& service_worker_origin,
                                       const GURL& origin,
                                       const blink::PlatformNotificationData& notification_data,
                                       const blink::NotificationResources& notification_resources) override;

    // Closes the notification identified by |notification_id|.
    // This method must be called on the UI thread.
    void CloseNotification(const std::string& notification_id) override;

    // Closes the persistent notification identified by |persistent_notification_id|.
    // This method must be called on the UI thread.
    void ClosePersistentNotification(const std::string& notification_id) override;

    // Retrieves the ids of all currently displaying notifications and
    // posts |callback| with the result.
    void GetDisplayedNotifications(DisplayedNotificationsCallback callback) override;

    // Reads the value of the next persistent notification ID from the profile and
    // increments the value, as it is called once per notification write.
    int64_t ReadNextPersistentNotificationId() override;

    void ScheduleTrigger(base::Time timestamp) override;
    base::Time ReadNextTriggerTimestamp() override;

    // Records a given notification to UKM.
    void RecordNotificationUkmEvent(const content::NotificationDatabaseData&) override { }

    content::BrowserContext *browser_context;
};

} // namespace QtWebEngineCore

#endif // PLATFORM_NOTIFICATION_SERVICE_QT_H
