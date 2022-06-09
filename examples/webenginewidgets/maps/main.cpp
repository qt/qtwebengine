// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.resize(1024, 768);
    mainWindow.show();

    return app.exec();
}
