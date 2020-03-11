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
            controller->setClient(0);
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
