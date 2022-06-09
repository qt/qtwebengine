// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "utils.h"

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>

static QUrl startupUrl()
{
    QUrl ret;
    QStringList args(qApp->arguments());
    args.takeFirst();
    for (const QString &arg : qAsConst(args)) {
        if (arg.startsWith(QLatin1Char('-')))
             continue;
        ret = Utils::fromUserInput(arg);
        if (ret.isValid())
            return ret;
    }
    return QUrl(QStringLiteral("https://www.qt.io"));
}

int main(int argc, char **argv)
{
    QCoreApplication::setOrganizationName("QtExamples");
    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine appEngine;
    Utils utils;
    appEngine.rootContext()->setContextProperty("utils", &utils);
    appEngine.load(QUrl("qrc:/ApplicationRoot.qml"));
    if (!appEngine.rootObjects().isEmpty())
        QMetaObject::invokeMethod(appEngine.rootObjects().first(), "load", Q_ARG(QVariant, startupUrl()));
    else
        qFatal("Failed to load sources");

    return app.exec();
}
