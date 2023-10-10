// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DESKTOP_NOTIFICATION_CONTROLLER_H
#define DESKTOP_NOTIFICATION_CONTROLLER_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qsharedpointer.h>
#include <QtCore/qurl.h>
#include <QtGui/qimage.h>

class GURL;

namespace blink {
    struct NotificationResources;
    struct PlatformNotificationData;
}

namespace QtWebEngineCore {

class UserNotificationControllerPrivate;

// Works as an accessor and owner of chromium objects related to showing desktop notifications.
class Q_WEBENGINECORE_EXPORT UserNotificationController : public QEnableSharedFromThis<UserNotificationController> {
public:
    struct Delegate {
        virtual ~Delegate() { }
        virtual void shown() = 0;
        virtual void clicked() = 0;
        virtual void closed(bool byUser) = 0;
    };

    UserNotificationController(const blink::PlatformNotificationData &params,
                                  const blink::NotificationResources &resources,
                                  const GURL &origin,
                                  Delegate *delegate);
    ~UserNotificationController();

    // The notification was shown.
    void notificationDisplayed();

    // The notification was closed.
    void notificationClosed();

    // The user clicked on the notification.
    void notificationClicked();

    // Chromium requests to close the notification.
    void closeNotification();

    QUrl origin() const;
    QImage icon() const;
    QImage image() const;
    QImage badge() const;
    QString title() const;
    QString body() const;
    QString tag() const;
    QString language() const;
    Qt::LayoutDirection direction() const;

    bool isShown() const;

    class Client {
    public:
        virtual ~Client() { }
        virtual void notificationClosed(const UserNotificationController *) = 0;
    };
    void setClient(Client *client);
    Client* client();

private:
    UserNotificationControllerPrivate *d;
};

} // namespace QtWebEngineCore

#endif // DESKTOP_NOTIFICATION_CONTROLLER_H
