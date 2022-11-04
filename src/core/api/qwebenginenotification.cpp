// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginenotification.h"

#include "user_notification_controller.h"

#include <QExplicitlySharedDataPointer>

QT_BEGIN_NAMESPACE

using QtWebEngineCore::UserNotificationController;

/*!
    \qmltype WebEngineNotification
    \instantiates QWebEngineNotification
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.9
    \brief Encapsulates the data of an HTML5 web notification.

    This type contains the information and API for HTML5 desktop and push notifications.

    Web engine notifications are passed to the user in the
    \l WebEngineProfile::presentNotification() signal.

    For more information about how to handle web notification, see the
    \l{WebEngine Notifications Example}{Notification Example}.
*/

/*!
    \class QWebEngineNotification
    \brief The QWebEngineNotification class encapsulates the data of an HTML5 web notification.
    \since 5.13

    \inmodule QtWebEngineCore

    This class contains the information and API for HTML5 desktop and push notifications.

    Web engine notifications are passed to the user through the custom handler
    provided with the \l QWebEngineProfile::setNotificationPresenter() call.

    For more information about how to handle web notification, see the
    \l{WebEngine Notifications Example}{Notification Example}.
*/

class QWebEngineNotificationPrivate : public UserNotificationController::Client {
public:
    QWebEngineNotificationPrivate(QWebEngineNotification *q, const QSharedPointer<UserNotificationController> &controller)
        : controller(controller)
        , q(q)
    {
        controller->setClient(this);
    }
    ~QWebEngineNotificationPrivate() override
    {
        if (controller->client() == this)
            controller->setClient(nullptr);
    }

    // UserNotificationController::Client:
    virtual void notificationClosed(const UserNotificationController *) override
    {
        Q_EMIT q->closed();
    }

    QSharedPointer<UserNotificationController> controller;
    QWebEngineNotification *q;
};

/*! \internal
*/
QWebEngineNotification::QWebEngineNotification(const QSharedPointer<UserNotificationController> &controller)
    : d_ptr(new QWebEngineNotificationPrivate(this, controller))
{}

/*! \internal
*/
QWebEngineNotification::~QWebEngineNotification() {}

/*!
    Returns \c true if the two notifications belong to the same message chain.
    That is, if their tag() and origin() are the same. This means one is
    a replacement or an update of the \a other.

    \sa tag(), origin()
*/
bool QWebEngineNotification::matches(const QWebEngineNotification *other) const
{
    if (!other)
        return false;
    if (!d_ptr)
        return !other->d_ptr;
    if (!other->d_ptr)
        return false;
    return tag() == other->tag() && origin() == other->origin();
}

/*!
    \qmlproperty string WebEngineNotification::title
    \brief The title of the notification.
*/
/*!
    \property QWebEngineNotification::title
    \brief The title of the notification.
    \sa message()
*/
QString QWebEngineNotification::title() const
{
    Q_D(const QWebEngineNotification);
    return d ? d->controller->title() : QString();
}

/*!
    \qmlproperty string WebEngineNotification::message
    \brief The body of the notification message.
*/
/*!
    \property QWebEngineNotification::message
    \brief The body of the notification message.
    \sa title()
*/

QString QWebEngineNotification::message() const
{
    Q_D(const QWebEngineNotification);
    return d ? d->controller->body() : QString();
}

/*!
    \qmlproperty string WebEngineNotification::tag
    \brief The tag of the notification message.

    New notifications that have the same tag and origin URL as an existing
    one should replace or update the old notification with the same tag.
*/
/*!
    \property QWebEngineNotification::tag
    \brief The tag of the notification message.

    New notifications that have the same tag and origin URL as an existing
    one should replace or update the old notification with the same tag.

    \sa matches()
*/
QString QWebEngineNotification::tag() const
{
    Q_D(const QWebEngineNotification);
    return d ? d->controller->tag() : QString();
}

/*!
    \qmlproperty url WebEngineNotification::origin
    \brief The URL of the page sending the notification.
*/
/*!
    \property QWebEngineNotification::origin
    \brief The URL of the page sending the notification.
*/

QUrl QWebEngineNotification::origin() const
{
    Q_D(const QWebEngineNotification);
    return d ? d->controller->origin() : QUrl();
}

/*!
    Returns the icon to be shown with the notification.

    If no icon is set by the sender, a null QImage is returned.
*/
QImage QWebEngineNotification::icon() const
{
    Q_D(const QWebEngineNotification);
    return d ? d->controller->icon() : QImage();
}

/*!
    \qmlproperty string WebEngineNotification::language
    \brief The primary language for the notification's title and body.

    Its value is a valid BCP 47 language tag, or the empty string.
*/
/*!
    \property QWebEngineNotification::language
    \brief The primary language for the notification's title and body.

    Its value is a valid BCP 47 language tag, or the empty string.

    \sa title(), message()
*/
QString QWebEngineNotification::language() const
{
    Q_D(const QWebEngineNotification);
    return d ? d->controller->language() : QString();
}

/*!
    \qmlproperty enumeration WebEngineNotification::direction
    \brief The text direction for the notification's title and body.

    \value Qt.LeftToRight Items are laid out from left to right.
    \value Qt.RightToLeft Items are laid out from right to left.
    \value Qt.LayoutDirectionAuto The direction to lay out items is determined automatically.
*/
/*!
    \property QWebEngineNotification::direction
    \brief The text direction for the notification's title and body.
    \sa title(), message()
*/
Qt::LayoutDirection QWebEngineNotification::direction() const
{
    Q_D(const QWebEngineNotification);
    return d ? d->controller->direction() : Qt::LayoutDirectionAuto;
}

/*!
    \qmlmethod void WebEngineNotification::show()
    Creates and dispatches a JavaScript \e {show event} on notification.

    Should be called by the notification platform when the notification has been shown to user.
*/
/*!
    Creates and dispatches a JavaScript \e {show event} on notification.

    Should be called by the notification platform when the notification has been shown to user.
*/
void QWebEngineNotification::show() const
{
    Q_D(const QWebEngineNotification);
    if (d)
        d->controller->notificationDisplayed();
}

/*!
    \qmlmethod void WebEngineNotification::click()
    Creates and dispatches a JavaScript \e {click event} on notification.

    Should be called by the notification platform when the notification is activated by the user.
*/
/*!
    Creates and dispatches a JavaScript \e {click event} on notification.

    Should be called by the notification platform when the notification is activated by the user.
*/
void QWebEngineNotification::click() const
{
    Q_D(const QWebEngineNotification);
    if (d)
        d->controller->notificationClicked();
}

/*!
    \qmlmethod void WebEngineNotification::close()
    Creates and dispatches a JavaScript \e {close event} on notification.

    Should be called by the notification platform when the notification is closed,
    either by the underlying platform or by the user.
*/
/*!
    Creates and dispatches a JavaScript \e {close event} on notification.

    Should be called by the notification platform when the notification is closed,
    either by the underlying platform or by the user.
*/
void QWebEngineNotification::close() const
{
    Q_D(const QWebEngineNotification);
    if (d)
        d->controller->notificationClosed();
}

/*!
    \qmlsignal WebEngineNotification::closed()

    This signal is emitted when the web page calls close steps for the notification,
    and it no longer needs to be shown.
*/
/*!
    \fn void QWebEngineNotification::closed()

    This signal is emitted when the web page calls close steps for the notification,
    and it no longer needs to be shown.
*/

QT_END_NAMESPACE

#include "moc_qwebenginenotification.cpp"
