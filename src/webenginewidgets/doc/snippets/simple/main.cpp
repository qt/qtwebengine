// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [Minimal Example]
#include <QApplication>
#include <QWebEngineView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
//! [Using QWebEngineView]
    QWebEngineView view;
    view.load(QUrl("https://qt-project.org/"));
    view.resize(1024, 750);
    view.show();
//! [Using QWebEngineView]
    return app.exec();
}
//! [Minimal Example]
