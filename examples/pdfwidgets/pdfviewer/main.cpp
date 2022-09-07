// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QUrl>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("Qt PDF Viewer");
    QCoreApplication::setOrganizationName("QtProject");

    QApplication a(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(a);

    MainWindow w;
    w.show();
    if (!parser.positionalArguments().isEmpty())
        w.open(QUrl::fromLocalFile(parser.positionalArguments().constFirst()));

    return a.exec();
}
