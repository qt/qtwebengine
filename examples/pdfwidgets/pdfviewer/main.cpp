// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include <QApplication>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QStringList args = a.arguments();
    w.show();
    if (args.length() > 1)
        w.open(QUrl::fromLocalFile(args[1]));

    return a.exec();
}
