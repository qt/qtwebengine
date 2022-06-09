// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
                             const GURL& document_url,
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
