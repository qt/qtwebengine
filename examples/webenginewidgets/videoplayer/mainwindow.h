// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fullscreenwindow.h"

#include <QMainWindow>
#include <QWebEngineView>
#include <QWebEngineFullScreenRequest>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void fullScreenRequested(QWebEngineFullScreenRequest request);

private:
    QWebEngineView *m_view;
    QScopedPointer<FullScreenWindow> m_fullScreenWindow;
};

#endif // MAINWINDOW_H
