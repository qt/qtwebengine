// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "platform_notification_service_qt.h"

#include "chrome/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_event_dispatcher.h"
#include "ui/message_center/public/cpp/notification_delegate.h"

#include "profile_adapter.h"
#include "profile_adapter_client.h"
#include "profile_qt.h"
#include "user_notification_controller.h"
#include "type_conversion.h"

#include <QSharedPointer>

namespace QtWebEngineCore {

struct NonPersistentNotificationDelegate : UserNotificationController::Delegate {
    NonPersistentNotificationDelegate(const std::string &id) : notification_id(id) { }

    virtual void shown() override {
        Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
        if (auto inst = content::NotificationEventDispatcher::GetInstance())
            inst->DispatchNonPersistentShowEvent(notification_id);
    }

    virtual void clicked() override {
        Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
        if (auto inst = content::NotificationEventDispatcher::GetInstance())
            inst->DispatchNonPersistentClickEvent(notification_id, base::DoNothing());
    }

    virtual void closed(bool /*by_user*/) override {
        Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
        if (auto inst = content::NotificationEventDispatcher::GetInstance())
            inst->DispatchNonPersistentCloseEvent(notification_id, base::DoNothing());
    }

    const std::string notification_id;
};

struct PersistentNotificationDelegate : UserNotificationController::Delegate {
    PersistentNotificationDelegate(content::BrowserContext *context, const std::string &id, const GURL &origin)
        : browser_context(context), notification_id(id), origin(origin) { }

    virtual void shown() override { }

    virtual void clicked() override {
        Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
        if (auto inst = content::NotificationEventDispatcher::GetInstance())
            inst->DispatchNotificationClickEvent(browser_context, notification_id, origin, absl::nullopt, absl::nullopt, base::DoNothing());
    }

    virtual void closed(bool by_user) override {
        Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
        if (auto inst = content::NotificationEventDispatcher::GetInstance())
            inst->DispatchNotificationCloseEvent(browser_context, notification_id, origin, by_user, base::DoNothing());
    }

    content::BrowserContext *browser_context;
    const std::string notification_id;
    const GURL origin;
};


PlatformNotificationServiceQt::PlatformNotificationServiceQt(content::BrowserContext *browserContext)
        : browser_context(browserContext)
{}

PlatformNotificationServiceQt::~PlatformNotificationServiceQt() {}

void PlatformNotificationServiceQt::DisplayNotification(
        const std::string &notification_id,
        const GURL &origin,
        const GURL &document_url,
        const blink::PlatformNotificationData &notificationData,
        const blink::NotificationResources &notificationResources)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    ProfileQt *profile = static_cast<ProfileQt*>(browser_context);

    auto delegate = new NonPersistentNotificationDelegate(notification_id);
    QSharedPointer<UserNotificationController> controller(
                new UserNotificationController(notificationData, notificationResources, origin, delegate));

    profile->profileAdapter()->ephemeralNotifications().insert(QByteArray::fromStdString(notification_id), controller);

    const QList<ProfileAdapterClient *> clients = profile->profileAdapter()->clients();
    for (ProfileAdapterClient *client : clients)
        client->showNotification(controller);
}

void PlatformNotificationServiceQt::DisplayPersistentNotification(
        const std::string &notification_id,
        const GURL &service_worker_origin,
        const GURL &origin,
        const blink::PlatformNotificationData &notificationData,
        const blink::NotificationResources &notificationResources)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    ProfileQt * profile = static_cast<ProfileQt*>(browser_context);

    auto delegate = new PersistentNotificationDelegate(profile, notification_id, service_worker_origin);
    QSharedPointer<UserNotificationController> controller(
                new UserNotificationController(notificationData, notificationResources, service_worker_origin, delegate));

    profile->profileAdapter()->persistentNotifications().insert(QByteArray::fromStdString(notification_id), controller);
    const QList<ProfileAdapterClient *> clients = profile->profileAdapter()->clients();
    for (ProfileAdapterClient *client : clients)
        client->showNotification(controller);
}

void PlatformNotificationServiceQt::CloseNotification(const std::string &notification_id)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    ProfileQt *profile = static_cast<ProfileQt*>(browser_context);

    QSharedPointer<UserNotificationController> notificationController =
            profile->profileAdapter()->ephemeralNotifications().take(QByteArray::fromStdString(notification_id)).lock();
    if (notificationController)
        notificationController->closeNotification();
}

void PlatformNotificationServiceQt::ClosePersistentNotification(const std::string &notification_id)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    ProfileQt *profile = static_cast<ProfileQt*>(browser_context);

    QSharedPointer<UserNotificationController> notificationController =
            profile->profileAdapter()->persistentNotifications().take(QByteArray::fromStdString(notification_id));
    if (notificationController)
        notificationController->closeNotification();
}

void PlatformNotificationServiceQt::GetDisplayedNotifications(DisplayedNotificationsCallback callback)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    ProfileQt *profile = static_cast<ProfileQt *>(browser_context);

    std::set<std::string> movableStdStringSet;
    auto it = profile->profileAdapter()->persistentNotifications().constBegin();
    const auto end = profile->profileAdapter()->persistentNotifications().constEnd();
    while (it != end) {
        if (it.value()->isShown())
            movableStdStringSet.insert(it.key().toStdString());
        ++it;
    }

    std::move(callback).Run(std::move(movableStdStringSet), true /* supports_synchronization */);
}

int64_t PlatformNotificationServiceQt::ReadNextPersistentNotificationId()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    auto prefs = static_cast<ProfileQt *>(browser_context)->GetPrefs();
    int64_t nextId = prefs->GetInteger(prefs::kNotificationNextPersistentId) + 1;
    prefs->SetInteger(prefs::kNotificationNextPersistentId, nextId);
    return nextId;
}

void PlatformNotificationServiceQt::ScheduleTrigger(base::Time /*timestamp*/)
{
    QT_NOT_YET_IMPLEMENTED
}

base::Time PlatformNotificationServiceQt::ReadNextTriggerTimestamp()
{
    return base::Time::Max();
}

} // namespace QtWebEngineCore
