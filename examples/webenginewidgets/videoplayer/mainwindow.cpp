// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"

#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebEngineFullScreenRequest>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_view(new QWebEngineView(this))
{
    setCentralWidget(m_view);
    m_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    connect(m_view->page(),
            &QWebEnginePage::fullScreenRequested,
            this,
            &MainWindow::fullScreenRequested);
    m_view->load(QUrl(QStringLiteral("qrc:/index.html")));
}

void MainWindow::fullScreenRequested(QWebEngineFullScreenRequest request)
{
    if (request.toggleOn()) {
        if (m_fullScreenWindow)
            return;
        request.accept();
        m_fullScreenWindow.reset(new FullScreenWindow(m_view));
    } else {
        if (!m_fullScreenWindow)
            return;
        request.accept();
        m_fullScreenWindow.reset();
    }
}
