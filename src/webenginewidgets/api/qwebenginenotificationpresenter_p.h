// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINENOTIFICATIONPRESENTER_P_H
#define QWEBENGINENOTIFICATIONPRESENTER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWebEngineCore/QWebEngineNotification>

#include <QtCore/QObject>
#include <QtGui/QIcon>

#include <memory>

QT_BEGIN_NAMESPACE

class QSystemTrayIcon;

class DefaultNotificationPresenter : public QObject
{
    Q_OBJECT
public:
    DefaultNotificationPresenter(QObject *parent = nullptr);
    virtual ~DefaultNotificationPresenter();

    void show(std::unique_ptr<QWebEngineNotification> notification);

private Q_SLOTS:
    void messageClicked();
    void closeNotification();

private:
    QSystemTrayIcon *m_systemTrayIcon;
    QIcon m_notificationIcon;
    std::unique_ptr<QWebEngineNotification> m_activeNotification;
};

void defaultNotificationPresenter(std::unique_ptr<QWebEngineNotification> notification);

QT_END_NAMESPACE

#endif // QWEBENGINENOTIFICATIONPRESENTER_P_H
