// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "user_notification_controller.h"

#include "type_conversion.h"

#include "third_party/blink/public/mojom/notifications/notification.mojom-shared.h"
#include "third_party/blink/public/common/notifications/notification_resources.h"
#include "third_party/blink/public/common/notifications/platform_notification_data.h"
#include "ui/message_center/public/cpp/notification_delegate.h"

#include <memory>

namespace QtWebEngineCore {

static Qt::LayoutDirection toDirection(blink::mojom::NotificationDirection direction)
{
    switch (direction) {
        case blink::mojom::NotificationDirection::LEFT_TO_RIGHT:
            return Qt::LeftToRight;
        case blink::mojom::NotificationDirection::RIGHT_TO_LEFT:
            return Qt::RightToLeft;
        case blink::mojom::NotificationDirection::AUTO:
        default:
            break;
    }
    return Qt::LayoutDirectionAuto;
}

class UserNotificationControllerPrivate  {
public:
    UserNotificationControllerPrivate(const blink::PlatformNotificationData &params,
                                         const blink::NotificationResources &resources,
                                         const GURL &origin)
        : m_params(params)
        , m_origin(origin)
        , m_delegate(nullptr)
        , m_resources(resources)
        , m_client(nullptr)
        , m_iconGenerated(false)
        , m_imageGenerated(false)
        , m_badgeGenerated(false)
        , m_shown(false)
    { }

    blink::PlatformNotificationData m_params;
    GURL m_origin;
    std::unique_ptr<UserNotificationController::Delegate> m_delegate;
    blink::NotificationResources m_resources;
    UserNotificationController::Client *m_client;
    QImage m_icon;
    QImage m_image;
    QImage m_badge;
    bool m_iconGenerated;
    bool m_imageGenerated;
    bool m_badgeGenerated;
    bool m_shown;
};


UserNotificationController::UserNotificationController(const blink::PlatformNotificationData &params,
                                                             const blink::NotificationResources &resources,
                                                             const GURL &origin,
                                                             Delegate *delegate)
        : d(new UserNotificationControllerPrivate(params, resources, origin))
{
    d->m_delegate.reset(delegate);
}

UserNotificationController::~UserNotificationController()
{
    delete d;
    d = nullptr;
}

void UserNotificationController::notificationDisplayed()
{
    if (!d->m_shown) {
        d->m_shown = true;
        if (d->m_delegate)
            d->m_delegate->shown();
    }
}

void UserNotificationController::notificationClosed()
{
    d->m_shown = false;
    if (d->m_delegate)
        d->m_delegate->closed(true);
}

void UserNotificationController::notificationClicked()
{
    if (d->m_delegate)
        d->m_delegate->clicked();
}

void UserNotificationController::closeNotification()
{
    d->m_shown = false;
    if (d->m_client)
        d->m_client->notificationClosed(this);
}

void UserNotificationController::setClient(UserNotificationController::Client* client)
{
    d->m_client = client;
}

UserNotificationController::Client* UserNotificationController::client()
{
    return d->m_client;
}

QUrl UserNotificationController::origin() const
{
    return toQt(d->m_origin);
}

QImage UserNotificationController::icon() const
{
    if (!d->m_iconGenerated) {
        d->m_iconGenerated = true;
        if (!d->m_resources.notification_icon.isNull())
            d->m_icon = toQImage(d->m_resources.notification_icon);
    }
    return d->m_icon;
}

QImage UserNotificationController::image() const
{
    if (d->m_imageGenerated)
        return d->m_image;
    d->m_image = toQImage(d->m_resources.image);
    d->m_imageGenerated = true;
    return d->m_image;
}

QImage UserNotificationController::badge() const
{
    if (d->m_badgeGenerated)
        return d->m_badge;
    d->m_badge = toQImage(d->m_resources.badge);
    d->m_badgeGenerated = true;
    return d->m_badge;
}

QString UserNotificationController::title() const
{
    return toQt(d->m_params.title);
}

QString UserNotificationController::body() const
{
    return toQt(d->m_params.body);
}

QString UserNotificationController::tag() const
{
    return toQt(d->m_params.tag);
}

QString UserNotificationController::language() const
{
    return toQt(d->m_params.lang);
}

Qt::LayoutDirection UserNotificationController::direction() const
{
    return toDirection(d->m_params.direction);
}

bool UserNotificationController::isShown() const
{
    return d->m_shown;
}

} // namespace QtWebEngineCore
