// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle(QStringLiteral("Material"));

    QQmlApplicationEngine engine;

    bool isEmbedded = false;
#ifdef QTWEBENGINE_RECIPE_BROWSER_EMBEDDED
    isEmbedded = true;
#endif
    engine.rootContext()->setContextProperty(QStringLiteral("isEmbedded"), isEmbedded);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
