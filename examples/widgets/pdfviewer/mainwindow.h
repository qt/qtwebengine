/******************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt PDF Module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcExample)

namespace Ui {
class MainWindow;
}

class QLineEdit;
class SequentialPageWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void open(const QUrl &docLocation);

private slots:
    void showingPageRange(int start, int end);
    void zoomChanged(qreal factor);
    void zoomEdited();

    // action handlers
    void on_actionOpen_triggered();
    void on_actionQuit_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionGo_triggered();
    void on_actionPrevious_Page_triggered();
    void on_actionNext_Page_triggered();

private:
    Ui::MainWindow *ui;
    SequentialPageWidget *m_pageWidget;
    QLineEdit *m_zoomEdit;
    QLineEdit *m_pageEdit;
};

#endif // MAINWINDOW_H
