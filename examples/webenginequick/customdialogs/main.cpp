// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "server.h"
#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <QNetworkProxy>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QtGui/QGuiApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    Server *server = new Server(&engine);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    QTimer::singleShot(0, server, &Server::run);

    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("localhost");
    proxy.setPort(5555);
    QNetworkProxy::setApplicationProxy(proxy);

    return app.exec();
}

