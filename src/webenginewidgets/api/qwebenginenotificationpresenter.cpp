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

#include "qwebenginenotificationpresenter_p.h"

#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE

DefaultNotificationPresenter::DefaultNotificationPresenter(QObject *parent) : QObject(parent)
{
#ifndef QT_NO_SYSTEMTRAYICON
    m_systemTrayIcon = new QSystemTrayIcon(this);
    connect(m_systemTrayIcon, &QSystemTrayIcon::messageClicked, this, &DefaultNotificationPresenter::messageClicked);
#endif
}

DefaultNotificationPresenter::~DefaultNotificationPresenter()
{
}

void DefaultNotificationPresenter::show(const QWebEngineNotification &notification)
{
    if (!m_activeNotification.isNull())
        m_activeNotification.close();
    m_activeNotification = notification;
#ifndef QT_NO_SYSTEMTRAYICON
    if (m_systemTrayIcon) {
        m_systemTrayIcon->show();
        QIcon icon = notification.icon();
        if (!icon.isNull())
            m_systemTrayIcon->showMessage(notification.title(), notification.message(), icon);
        else
            m_systemTrayIcon->showMessage(notification.title(), notification.message());
        notification.show();
        connect(&m_activeNotification, &QWebEngineNotification::closed, this, &DefaultNotificationPresenter::closeNotification);
    }
#endif
}

void DefaultNotificationPresenter::messageClicked()
{
    if (!m_activeNotification.isNull())
        m_activeNotification.click();
}

void DefaultNotificationPresenter::closeNotification()
{
#ifndef QT_NO_SYSTEMTRAYICON
    const QWebEngineNotification *canceled = static_cast<const QWebEngineNotification *>(QObject::sender());
    if (m_systemTrayIcon && canceled->matches(m_activeNotification))
        m_systemTrayIcon->hide();
#endif
}

void defaultNotificationPresenter(const QWebEngineNotification &notification)
{
    static DefaultNotificationPresenter *presenter = nullptr;
    if (!presenter)
        presenter = new DefaultNotificationPresenter();
    presenter->show(notification);
}


QT_END_NAMESPACE
