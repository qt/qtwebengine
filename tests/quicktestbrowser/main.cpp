// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "utils.h"

#ifndef QT_NO_WIDGETS
#include <QtWidgets/QApplication>
typedef QApplication Application;
#else
#include <QtGui/QGuiApplication>
typedef QGuiApplication Application;
#endif
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlComponent>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <QtWebEngineQuick/QQuickWebEngineProfile>
#include <QtWebEngineCore/qwebenginecookiestore.h>

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
    return QUrl(QStringLiteral("http://qt.io/"));
}

int main(int argc, char **argv)
{
    Application app(argc, argv);

    // Enable dev tools by default for the test browser
    if (qgetenv("QTWEBENGINE_REMOTE_DEBUGGING").isNull())
        qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "1337");
    QtWebEngineQuick::initialize();

    QQmlApplicationEngine appEngine;
    Utils utils;
    appEngine.rootContext()->setContextProperty("utils", &utils);
    appEngine.load(QUrl("qrc:/ApplicationRoot.qml"));

    QObject *rootObject = appEngine.rootObjects().first();

    QQuickWebEngineProfile *profile = new QQuickWebEngineProfile(rootObject);

    const QMetaObject *rootMeta = rootObject->metaObject();
    int index = rootMeta->indexOfProperty("thirdPartyCookiesEnabled");
    Q_ASSERT(index != -1);
    QMetaProperty thirdPartyCookiesProperty = rootMeta->property(index);
    profile->cookieStore()->setCookieFilter(
            [rootObject,&thirdPartyCookiesProperty](const QWebEngineCookieStore::FilterRequest &request)
            {
                return !request.thirdParty || thirdPartyCookiesProperty.read(rootObject).toBool();
            });

    index = rootMeta->indexOfProperty("testProfile");
    Q_ASSERT(index != -1);
    QMetaProperty profileProperty = rootMeta->property(index);
    profileProperty.write(rootObject, QVariant::fromValue(profile));

    QMetaObject::invokeMethod(rootObject, "load", Q_ARG(QVariant, startupUrl()));

    return app.exec();
}
