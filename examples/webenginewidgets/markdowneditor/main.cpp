// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "document.h"
#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QApplication a(argc, argv);

    MainWindow window;
    window.show();

    return a.exec();
}
