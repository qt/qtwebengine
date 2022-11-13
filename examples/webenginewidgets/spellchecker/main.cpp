// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "webview.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QApplication app(argc, argv);

    WebView view;
    view.setUrl(QUrl(QStringLiteral("qrc:/index.html")));
    view.resize(500, 640);
    view.show();

    return app.exec();
}
