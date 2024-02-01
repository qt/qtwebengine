// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef TOUCHMOCKINGAPPLICATION_H
#define TOUCHMOCKINGAPPLICATION_H

#if defined(QUICK_TOUCHBROWSER)
#    include <QGuiApplication>
using Application = QGuiApplication;
#elif defined(WIDGET_TOUCHBROWSER)
#    include <QApplication>
using Application = QApplication;
#endif

QT_BEGIN_NAMESPACE
class QCursor;
QT_END_NAMESPACE

class TouchMockingApplication : public Application
{
    Q_OBJECT

public:
    TouchMockingApplication(int &argc, char **argv);
    ~TouchMockingApplication();

    virtual bool notify(QObject *, QEvent *) override;

private:
    void restoreCursor();

    QCursor *m_touchPoint;
};

#endif // TOUCHMOCKINGAPPLICATION_H
