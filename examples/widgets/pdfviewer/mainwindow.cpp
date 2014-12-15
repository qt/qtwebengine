#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPdfDocument>

Q_LOGGING_CATEGORY(lcExample, "qt.examples.pdfviewer")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_doc(new QPdfDocument(this))
{
    ui->setupUi(this);
    ui->pageWidget->setDocument(m_doc);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open(const QUrl &docLocation)
{
    if (docLocation.isLocalFile())
        m_doc->load(docLocation.toLocalFile());
    else {
        qCDebug(lcExample) << docLocation << "is not a valid local file";
        QMessageBox::critical(this, tr("Failed to open"), tr("%1 is not a valid local file").arg(docLocation.toString()));
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QUrl toOpen = QFileDialog::getOpenFileUrl(this);
    if (toOpen.isValid())
        open(toOpen);
}
