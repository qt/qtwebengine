// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#if 0
#pragma qt_no_master_include
#endif

#include <QResizeEvent>
#include <QScopedPointer>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>

// TestWindow: Utility class to ignore QQuickView details.
class TestWindow : public QQuickView {
public:
    inline TestWindow(QQuickItem *webEngineView);
    QScopedPointer<QQuickItem> webEngineView;

protected:
    inline void resizeEvent(QResizeEvent *) override;
};

inline TestWindow::TestWindow(QQuickItem *webEngineView)
    : webEngineView(webEngineView)
{
    Q_ASSERT(webEngineView);
    webEngineView->setParentItem(contentItem());
    resize(300, 400);
}

inline void TestWindow::resizeEvent(QResizeEvent *event)
{
    QQuickView::resizeEvent(event);
    webEngineView->setX(0);
    webEngineView->setY(0);
    webEngineView->setWidth(event->size().width());
    webEngineView->setHeight(event->size().height());
}

#endif /* TESTWINDOW_H */
