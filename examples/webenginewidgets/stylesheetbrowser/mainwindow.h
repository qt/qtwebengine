// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QWebEngineScriptCollection>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QUrl &url);
    ~MainWindow();

    void insertStyleSheet(const QString &name, const QString &source, bool immediately);
    void removeStyleSheet(const QString &name, bool immediately);
    bool hasStyleSheet(const QString &name);
    void loadDefaultStyleSheets();

private slots:
    void urlEntered();
    void urlChanged(const QUrl &url);
    void showStyleSheetsDialog();
    void reloadRequested();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
