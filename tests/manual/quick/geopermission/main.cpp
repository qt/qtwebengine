// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWebEngineQuick/qtwebenginequickglobal.h>

#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QQuickView>

#include <QtGui/QGuiApplication>

#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtCore/QLoggingCategory>

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName("Geopermission test");
    QCoreApplication::setOrganizationName("QtProject");

    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);

    QQuickView view;

    view.setTitle("Touch Browser");
    view.setFlags(Qt::Window | Qt::WindowTitleHint);
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/tst_geopermission.qml"));

    QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));

    view.show();
    if (view.size().isEmpty())
        view.setGeometry(0, 0, 800, 600);

    return app.exec();
}
