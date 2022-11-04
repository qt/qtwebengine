// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "webuihandler.h"

#include <QApplication>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");

    WebUiHandler::registerUrlScheme();

    QApplication app(argc, argv);

    QWebEngineProfile profile;

    WebUiHandler handler;
    profile.installUrlSchemeHandler(WebUiHandler::schemeName, &handler);

    QWebEnginePage page(&profile);
    QWebEngineView view;
    view.setPage(&page);
    page.load(WebUiHandler::aboutUrl);
    view.setContextMenuPolicy(Qt::NoContextMenu);
    view.resize(500, 600);
    view.show();

    return app.exec();
}
