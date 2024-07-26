// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include <QApplication>
#include <QLoggingCategory>
#include <QUrl>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(QString(QLatin1StringView(":AppLogoColor.png"))));
    MainWindow window(QUrl("qrc:/landing.html"));
    window.resize(1024, 768);
    window.show();
    return app.exec();
}
