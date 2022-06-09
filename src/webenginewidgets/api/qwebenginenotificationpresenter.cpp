// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginenotificationpresenter_p.h"

#include <QApplication>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE

DefaultNotificationPresenter::DefaultNotificationPresenter(QObject *parent) : QObject(parent)
{
#if QT_CONFIG(systemtrayicon)
    m_systemTrayIcon = new QSystemTrayIcon(this);
    connect(m_systemTrayIcon, &QSystemTrayIcon::messageClicked, this, &DefaultNotificationPresenter::messageClicked);
#endif
}

DefaultNotificationPresenter::~DefaultNotificationPresenter()
{
}

void DefaultNotificationPresenter::show(std::unique_ptr<QWebEngineNotification> notification)
{
    Q_ASSERT(notification);
    if (m_activeNotification) {
        m_activeNotification->close();
        m_activeNotification->disconnect(this);
    }

    m_activeNotification = std::move(notification);

#if QT_CONFIG(systemtrayicon)
    if (m_activeNotification && m_systemTrayIcon) {
        m_systemTrayIcon->setIcon(qApp->windowIcon());
        m_systemTrayIcon->show();
        QImage notificationIconImage = m_activeNotification->icon();
        m_notificationIcon = QIcon(QPixmap::fromImage(std::move(notificationIconImage), Qt::NoFormatConversion));
        if (!m_notificationIcon.isNull())
            m_systemTrayIcon->showMessage(m_activeNotification->title(), m_activeNotification->message(), m_notificationIcon);
        else
            m_systemTrayIcon->showMessage(m_activeNotification->title(), m_activeNotification->message());
        m_activeNotification->show();
        connect(m_activeNotification.get(), &QWebEngineNotification::closed, this, &DefaultNotificationPresenter::closeNotification);
    }
#endif
}

void DefaultNotificationPresenter::messageClicked()
{
    if (m_activeNotification)
        m_activeNotification->click();
}

void DefaultNotificationPresenter::closeNotification()
{
#if QT_CONFIG(systemtrayicon)
    const QWebEngineNotification *canceled = static_cast<const QWebEngineNotification *>(QObject::sender());
    if (m_systemTrayIcon && canceled->matches(m_activeNotification.get()))
        m_systemTrayIcon->hide();
#endif
}

void defaultNotificationPresenter(std::unique_ptr<QWebEngineNotification> notification)
{
    static DefaultNotificationPresenter *presenter = nullptr;
    if (!presenter)
        presenter = new DefaultNotificationPresenter();
    presenter->show(std::move(notification));
}


QT_END_NAMESPACE

#include "moc_qwebenginenotificationpresenter_p.cpp"
