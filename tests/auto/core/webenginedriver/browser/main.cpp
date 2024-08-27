// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QtWidgets/qapplication.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWebEngineView view;
    QObject::connect(view.page(), &QWebEnginePage::windowCloseRequested, &app, &QApplication::quit);
    QObject::connect(&app, &QApplication::aboutToQuit,
                     []() { fprintf(stderr, "Test browser is about to quit.\n"); });

    view.resize(100, 100);
    view.show();

    return app.exec();
}
