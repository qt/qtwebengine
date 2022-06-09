// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QUrl>
#include <QWebEngineView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget *parent = nullptr;
//! [Using QWebEngineView]
    QWebEngineView *view = new QWebEngineView(parent);
    view->load(QUrl("http://qt-project.org/"));
    view->show();
//! [Using QWebEngineView]
    return app.exec();
}
