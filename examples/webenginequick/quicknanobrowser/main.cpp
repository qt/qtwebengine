// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "utils.h"

#include <QtWebEngineQuick/qtwebenginequickglobal.h>

#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include <QtGui/QGuiApplication>

#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtCore/QLoggingCategory>

static QUrl startupUrl(const QCommandLineParser &parser)
{
    if (!parser.positionalArguments().isEmpty()) {
        const QUrl url = Utils::fromUserInput(parser.positionalArguments().constFirst());
        if (url.isValid())
            return url;
    }
    return QUrl(QStringLiteral("chrome://qt"));
}

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName("Quick Nano Browser");
    QCoreApplication::setOrganizationName("QtProject");

    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);
    QLoggingCategory::setFilterRules(QStringLiteral("qt.webenginecontext.debug=true"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("url", "The URL to open.");
    parser.process(app);

    QQmlApplicationEngine appEngine;
    appEngine.load(QUrl("qrc:/ApplicationRoot.qml"));
    if (appEngine.rootObjects().isEmpty())
        qFatal("Failed to load sources");

    const QUrl url = startupUrl(parser);
    QMetaObject::invokeMethod(appEngine.rootObjects().constFirst(),
                              "load", Q_ARG(QVariant, url));

    return app.exec();
}
