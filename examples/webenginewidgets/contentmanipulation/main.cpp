// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>
#include "mainwindow.h"
#include <qtwebenginewidgetsglobal.h>

int main(int argc, char * argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QApplication app(argc, argv);

    QUrl url;
    if (argc > 1)
        url = QUrl::fromUserInput(argv[1]);
    else
        url = QUrl("http://www.google.com/ncr");
    MainWindow *browser = new MainWindow(url);
    browser->resize(1024, 768);
    browser->show();
    return app.exec();
}
