/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
        m_icon.setPixmap(QPixmap::fromImage(notification->icon()).scaledToHeight(m_icon.height()));

        show();
        notification->show();

        connect(notification.get(), &QWebEngineNotification::closed, this, &NotificationPopup::onClosed);
        QTimer::singleShot(10000, notification.get(), [&] () { onClosed(); });

        // position our popup in the right corner of its parent widget
        move(parentWidget()->mapToGlobal(parentWidget()->rect().bottomRight() - QPoint(width() + 10, height() + 10)));
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

