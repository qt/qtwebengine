// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "browser.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include <QApplication>
#include <QLoggingCategory>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

using namespace Qt::StringLiterals;

QUrl commandLineUrlArgument()
{
    const QStringList args = QCoreApplication::arguments();
    for (const QString &arg : args.mid(1)) {
        if (!arg.startsWith(u'-'))
            return QUrl::fromUserInput(arg);
    }
    return QUrl(u"chrome://qt"_s);
}

int main(int argc, char **argv)
{
    QCoreApplication::setOrganizationName("QtExamples");

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(u":AppLogoColor.png"_s));
    QLoggingCategory::setFilterRules(u"qt.webenginecontext.debug=true"_s);

    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);

    QUrl url = commandLineUrlArgument();

    Browser browser;
    BrowserWindow *window = browser.createHiddenWindow();
    window->tabWidget()->setUrl(url);
    window->show();
    return app.exec();
}
