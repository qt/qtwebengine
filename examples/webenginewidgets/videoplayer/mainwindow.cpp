// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"

#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebEngineFullScreenRequest>

using namespace Qt::StringLiterals;

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

    const QUrl baseUrl = QUrl(u"https://www.qt.io"_s);
    const QString videoUrl = u"https://www.youtube.com/embed/CjyjEUFn_FI"_s;
    const QString html = u"<!doctype html>"
                         "<html lang='en'>"
                         "     <head>"
                         "         <meta charset='utf-8'>"
                         "         <style type='text/css'>"
                         "             #ytplayer {"
                         "                 position: absolute;"
                         "                 top: 0;"
                         "                 left: 0;"
                         "                 width: 100%;"
                         "                 height: 100%;"
                         "             }"
                         "         </style>"
                         "     </head>"
                         "     <body>"
                         "         <iframe"
                         "             id='ytplayer'"
                         "             src='%1'"
                         "             frameborder='0'"
                         "             allowfullscreen>"
                         "         </iframe>"
                         "     </body>"
                         "</html>"_s.arg(videoUrl);
    m_view->setHtml(html, baseUrl);
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
