// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
    QCoreApplication::setApplicationName("Qt Quick PDF Viewer Example");
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///pdfviewer/viewer.qml")));
    if (app.arguments().count() > 1) {
        QUrl toLoad = QUrl::fromUserInput(app.arguments().at(1));
        engine.rootObjects().first()->setProperty("source", toLoad);
    } else {
        engine.rootObjects().first()->setProperty("source", QStringLiteral("resources/test.pdf"));
    }

    return app.exec();
}
