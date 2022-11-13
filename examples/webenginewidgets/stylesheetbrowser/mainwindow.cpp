// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "stylesheetdialog.h"
#include "ui_mainwindow.h"

static QMap<QString, QString> defaultStyleSheets = {
    {"Upside down", "body { -webkit-transform: rotate(180deg); }"}
};

MainWindow::MainWindow(const QUrl &url) :
    QMainWindow(),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->urlBar, &QLineEdit::returnPressed, this, &MainWindow::urlEntered);
    connect(ui->webEngineView, &QWebEngineView::urlChanged, this, &MainWindow::urlChanged);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::showStyleSheetsDialog);
    connect(ui->reloadButton, &QPushButton::clicked, this, &MainWindow::reloadRequested);

    QSettings settings;
    settings.beginGroup("styleSheets");
    QStringList styleSheets = settings.allKeys();
    if (styleSheets.empty()) {
        // Add back default style sheets if the user cleared them out
        loadDefaultStyleSheets();
    } else {
        for (auto name : qAsConst(styleSheets)) {
            StyleSheet styleSheet = settings.value(name).value<StyleSheet>();
            if (styleSheet.second)
                insertStyleSheet(name, styleSheet.first, false);
        }
    }
    settings.endGroup();

    ui->webEngineView->setUrl(url);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::insertStyleSheet(const QString &name, const QString &source, bool immediately)
{
    QWebEngineScript script;
    QString s = QString::fromLatin1("(function() {"\
                                    "    css = document.createElement('style');"\
                                    "    css.type = 'text/css';"\
                                    "    css.id = '%1';"\
                                    "    document.head.appendChild(css);"\
                                    "    css.innerText = '%2';"\
                                    "})()").arg(name).arg(source.simplified());
    if (immediately)
        ui->webEngineView->page()->runJavaScript(s, QWebEngineScript::ApplicationWorld);

    script.setName(name);
    script.setSourceCode(s);
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setRunsOnSubFrames(true);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    ui->webEngineView->page()->scripts().insert(script);
}

void MainWindow::removeStyleSheet(const QString &name, bool immediately)
{
    QString s = QString::fromLatin1("(function() {"\
                                    "    var element = document.getElementById('%1');"\
                                    "    element.outerHTML = '';"\
                                    "    delete element;"\
                                    "})()").arg(name);
    if (immediately)
        ui->webEngineView->page()->runJavaScript(s, QWebEngineScript::ApplicationWorld);

    const QList<QWebEngineScript> scripts = ui->webEngineView->page()->scripts().find(name);
    if (!scripts.isEmpty())
        ui->webEngineView->page()->scripts().remove(scripts.first());
}

bool MainWindow::hasStyleSheet(const QString &name)
{
    const QList<QWebEngineScript> scripts = ui->webEngineView->page()->scripts().find(name);
    return !scripts.isEmpty();
}

void MainWindow::loadDefaultStyleSheets()
{
    QSettings settings;
    settings.beginGroup("styleSheets");

    auto it = defaultStyleSheets.constBegin();
    while (it != defaultStyleSheets.constEnd()) {
        settings.setValue(it.key(), QVariant::fromValue(qMakePair(it.value(), true)));
        insertStyleSheet(it.key(), it.value(), false);
        ++it;
    }

    settings.endGroup();
}

void MainWindow::urlEntered()
{
    ui->webEngineView->setUrl(QUrl::fromUserInput(ui->urlBar->text()));
}

void MainWindow::urlChanged(const QUrl &url)
{
    ui->urlBar->setText(url.toString());
}

void MainWindow::showStyleSheetsDialog()
{
    StylesheetDialog *dialog = new StylesheetDialog(this);
    dialog->show();
}

void MainWindow::reloadRequested()
{
    ui->webEngineView->reload();
}
