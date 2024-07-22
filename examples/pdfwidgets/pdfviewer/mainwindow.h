// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLoggingCategory>
#include <QMainWindow>

Q_DECLARE_LOGGING_CATEGORY(lcExample)

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}

class QFileDialog;
class QLineEdit;
class QPdfDocument;
class QPdfPageSelector;
class QPdfSearchModel;
class QPdfView;
class QSpinBox;
QT_END_NAMESPACE

class ZoomSelector;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void open(const QUrl &docLocation);

private slots:
    void bookmarkSelected(const QModelIndex &index);
    void pageSelected(int page);
    void searchResultSelected(const QModelIndex &current, const QModelIndex &previous);

    // action handlers
    void on_actionOpen_triggered();
    void on_actionQuit_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionPrevious_Page_triggered();
    void on_actionNext_Page_triggered();
    void on_thumbnailsView_activated(const QModelIndex &index);
    void on_actionContinuous_triggered();
    void on_actionBack_triggered();
    void on_actionForward_triggered();
    void on_actionFindNext_triggered();
    void on_actionFindPrevious_triggered();

private:
    Ui::MainWindow *ui;
    ZoomSelector *m_zoomSelector;
    QPdfPageSelector *m_pageSelector;
    QPdfSearchModel *m_searchModel;
    QLineEdit *m_searchField;
    QFileDialog *m_fileDialog = nullptr;

    QPdfDocument *m_document;
};

#endif // MAINWINDOW_H
