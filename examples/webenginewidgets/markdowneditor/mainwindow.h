// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "document.h"

#include <QMainWindow>
#include <QString>

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

    void openFile(const QString &path);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();

private:
    bool isModified() const;

    Ui::MainWindow *ui;
    QString m_filePath;
    Document m_content;
};

#endif // MAINWINDOW_H
