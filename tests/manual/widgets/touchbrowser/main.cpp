// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "touchmockingapplication.h"
#include "utils.h"

#include <QApplication>
#include <QLineEdit>
#include <QMainWindow>
#include <QToolBar>
#include <QWebEngineView>


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
    TouchMockingApplication app(argc, argv);
    app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

    QMainWindow window;
    QWebEngineView view(&window);
    QToolBar addressBar("AddressBar", &window);
    QLineEdit lineEdit(&addressBar);

    view.setAttribute(Qt::WA_AcceptTouchEvents, true);
    view.setUrl(startupUrl());
    window.resize(1024, 750);
    window.setCentralWidget(&view);

    addressBar.setAttribute(Qt::WA_AcceptTouchEvents, true);
    addressBar.setMovable(false);
    addressBar.toggleViewAction()->setEnabled(false);

    lineEdit.setAttribute(Qt::WA_AcceptTouchEvents, true);
    lineEdit.setClearButtonEnabled(true);

    addressBar.addWidget(&lineEdit);
    QObject::connect(&lineEdit, &QLineEdit::returnPressed, [&]() {
        QUrl url = Utils::fromUserInput(lineEdit.text());
        lineEdit.setText(url.toDisplayString());
        view.setUrl(url);
    });

    window.addToolBar(&addressBar);
    window.show();

    return app.exec();
}
