// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSpacerItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWebEngineNotification>

#include <memory>

class NotificationPopup : public QWidget
{
    Q_OBJECT

    QLabel m_icon, m_title, m_message;
    std::unique_ptr<QWebEngineNotification> notification;

public:
    NotificationPopup(QWidget *parent) : QWidget(parent)
    {
        setWindowFlags(Qt::ToolTip);
        auto rootLayout = new QHBoxLayout(this);

        rootLayout->addWidget(&m_icon);

        auto bodyLayout = new QVBoxLayout;
        rootLayout->addLayout(bodyLayout);

        auto titleLayout = new QHBoxLayout;
        bodyLayout->addLayout(titleLayout);

        titleLayout->addWidget(&m_title);
        titleLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

        auto close = new QPushButton(tr("Close"));
        titleLayout->addWidget(close);
        connect(close, &QPushButton::clicked, this, &NotificationPopup::onClosed);

        bodyLayout->addWidget(&m_message);
        adjustSize();
    }

    void present(std::unique_ptr<QWebEngineNotification> &newNotification)
    {
        if (notification) {
            notification->close();
            notification.reset();
        }

        notification.swap(newNotification);

        m_title.setText("<b>" + notification->title() + "</b>");
        m_message.setText(notification->message());
        m_icon.setPixmap(QPixmap(":/icon.png").scaledToHeight(m_icon.height()));

        show();
        notification->show();

        connect(notification.get(), &QWebEngineNotification::closed, this,
                &NotificationPopup::onClosed);
        QTimer::singleShot(10000, notification.get(), [&]() { onClosed(); });

        // position our popup in the right corner of its parent widget
        move(parentWidget()->mapToGlobal(parentWidget()->rect().bottomRight()
                                         - QPoint(width() + 10, height() + 10)));
    }

protected slots:
    void onClosed()
    {
        hide();
        notification->close();
        notification.reset();
    }

protected:
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        QWidget::mouseReleaseEvent(event);
        if (notification && event->button() == Qt::LeftButton) {
            notification->click();
            onClosed();
        }
    }
};
