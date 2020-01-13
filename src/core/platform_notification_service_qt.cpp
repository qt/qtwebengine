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

#include "platform_notification_service_qt.h"

#include "chrome/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/permission_type.h"
#include "content/public/browser/notification_event_dispatcher.h"
#include "ui/message_center/public/cpp/notification_delegate.h"

#include "profile_adapter.h"
#include "profile_adapter_client.h"
#include "profile_qt.h"
#include "user_notification_controller.h"
#include "resource_context_qt.h"
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
            inst->DispatchNotificationClickEvent(browser_context, notification_id, origin, base::nullopt, base::nullopt, base::DoNothing());
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
