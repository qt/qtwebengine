// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QQmlApplicationEngine>

#include <QGuiApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char* argv[])
{
    QCoreApplication::setApplicationName("Qt Quick PDF Viewer Example");
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    QQmlApplicationEngine engine;

    QUrl toLoad = QUrl("qrc:/pdfviewer/resources/test.pdf");
    if (!parser.positionalArguments().isEmpty())
        toLoad = QUrl::fromLocalFile(parser.positionalArguments().constFirst());

    engine.setInitialProperties({{"source", toLoad}});

    engine.load(QUrl(QStringLiteral("qrc:///pdfviewer/viewer.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
