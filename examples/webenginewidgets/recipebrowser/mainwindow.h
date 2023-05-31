// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "document.h"
#include <QMainWindow>
#include <QSettings>
#include <QWebEngineScriptCollection>
#include <QWebEngineView>
#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void insertStyleSheet(const QString &name, const QString &source, bool immediately);
    void removeStyleSheet(const QString &name, bool immediately);
    bool hasStyleSheet(const QString &name);
    void loadDefaultStyleSheets();

private slots:
    void showStyleSheetsDialog();
    void toggleEditView();

private:
    Ui::MainWindow *ui;

    bool m_isEditMode;
    Document m_content;
};

#endif // MAINWINDOW_H
