/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#if defined(DESKTOP_BUILD)
#include "touchmockingapplication.h"
#endif
#include "utils.h"

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickView>
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
    return QUrl(QStringLiteral("https://www.qt.io/"));
}

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // We use touch mocking on desktop and apply all the mobile switches.
    QByteArrayList args = QByteArrayList()
            << QByteArrayLiteral("--enable-embedded-switches")
            << QByteArrayLiteral("--log-level=0");
    const int count = args.size() + argc;
    QList<char*> qargv(count);

    qargv[0] = argv[0];
    for (int i = 0; i < args.size(); ++i)
        qargv[i + 1] = args[i].data();
    for (int i = args.size() + 1; i < count; ++i)
        qargv[i] = argv[i - args.size()];

    int qAppArgCount = qargv.size();

    QtWebEngineQuick::initialize();

#if defined(DESKTOP_BUILD)
    TouchMockingApplication app(qAppArgCount, qargv.data());
#else
    QGuiApplication app(qAppArgCount, qargv.data());
#endif

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
