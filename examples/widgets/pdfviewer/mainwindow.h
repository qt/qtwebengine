#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcExample)

namespace Ui {
class MainWindow;
}

class QPdfDocument;
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
    void on_actionOpen_triggered();

private:
    Ui::MainWindow *ui;
    QPdfDocument *m_doc;
    SequentialPageWidget *m_pageWidget;
};

#endif // MAINWINDOW_H
