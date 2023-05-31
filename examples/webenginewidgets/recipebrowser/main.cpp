// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "stylesheetdialog.h"
#include <QApplication>
#include <QUrl>

int main(int argc, char *argv[])
{
    qRegisterMetaType<StyleSheet>("StyleSheet");

    QCoreApplication::setOrganizationName("QtExamples");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
