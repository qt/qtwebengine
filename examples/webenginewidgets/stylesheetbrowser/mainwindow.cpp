/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "stylesheetdialog.h"

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
    for (auto name : qAsConst(styleSheets))
        insertStyleSheet(name, settings.value(name, QString()).toString(), false);
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

    QWebEngineScript script = ui->webEngineView->page()->scripts().findScript(name);
    ui->webEngineView->page()->scripts().remove(script);
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
