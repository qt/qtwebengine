#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPdfDocument>
#include "sequentialpagewidget.h"

Q_LOGGING_CATEGORY(lcExample, "qt.examples.pdfviewer")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_doc(new QPdfDocument(this))
    , m_pageWidget(new SequentialPageWidget(this))
{
    ui->setupUi(this);
    m_pageWidget->setDocument(m_doc);
    ui->scrollArea->setWidget(m_pageWidget);
    connect(m_pageWidget, SIGNAL(showingPageRange(int,int)),
            this, SLOT(showingPageRange(int,int)), Qt::QueuedConnection);
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
    // TODO: connect to signal from document
    m_pageWidget->invalidate();
    qCDebug(lcExample) << docLocation;
    ui->scrollArea->ensureVisible(0, 0, 0, 0);
}

void MainWindow::showingPageRange(int start, int end)
{
    ui->statusBar->clearMessage();
    if (start == end)
        ui->statusBar->showMessage(tr("showing page %1").arg(start));
    else
        ui->statusBar->showMessage(tr("showing pages %1 to %2").arg(start).arg(end));
}

void MainWindow::on_actionOpen_triggered()
{
    QUrl toOpen = QFileDialog::getOpenFileUrl(this, tr("Choose a PDF"), QUrl(), "Portable Documents (*.pdf)");
    if (toOpen.isValid())
        open(toOpen);
}
