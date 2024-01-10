// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "touchmockingapplication.h"
#include "utils.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QtWebEngineQuick>

static QUrl startupUrl()
{
    QUrl ret;
    QStringList args(qApp->arguments());
    args.takeFirst();
    for (const QString &arg : std::as_const(args)) {
        if (arg.startsWith(QLatin1Char('-')))
             continue;
        ret = Utils::fromUserInput(arg);
        if (ret.isValid())
            return ret;
    }
    return QUrl(QStringLiteral("https://www.qt.io/"));
}

int main(int argc, char **argv)
{
    QtWebEngineQuick::initialize();

    TouchMockingApplication app(argc, argv);
    app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

    QQuickView view;
    Utils utils;
    view.rootContext()->setContextProperty("utils", &utils);

    view.setTitle("Touch Browser");
    view.setFlags(Qt::Window | Qt::WindowTitleHint);
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/main.qml"));

    QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));

    view.show();
    if (view.size().isEmpty())
        view.setGeometry(0, 0, 800, 600);
    QMetaObject::invokeMethod(reinterpret_cast<QObject *>(view.rootObject()), "load", Q_ARG(QVariant, startupUrl()));

    return app.exec();
}
