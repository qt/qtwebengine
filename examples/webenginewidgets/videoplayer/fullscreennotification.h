// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef FULLSCREENNOTIFICATION_H
#define FULLSCREENNOTIFICATION_H

#include <QLabel>

class FullScreenNotification : public QLabel
{
    Q_OBJECT
public:
    FullScreenNotification(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

signals:
    void shown();

private:
    bool m_previouslyVisible;
};

#endif // FULLSCREENNOTIFICATION_H
