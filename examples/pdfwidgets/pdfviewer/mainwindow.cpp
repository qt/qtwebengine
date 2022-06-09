// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "zoomselector.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QPdfBookmarkModel>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QtMath>

const qreal zoomMultiplier = qSqrt(2.0);

Q_LOGGING_CATEGORY(lcExample, "qt.examples.pdfviewer")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_zoomSelector(new ZoomSelector(this))
    , m_pageSelector(new QSpinBox(this))
    , m_document(new QPdfDocument(this))
{
    ui->setupUi(this);

    m_zoomSelector->setMaximumWidth(150);
    ui->mainToolBar->insertWidget(ui->actionZoom_In, m_zoomSelector);

    ui->mainToolBar->insertWidget(ui->actionForward, m_pageSelector);
    connect(m_pageSelector, &QSpinBox::valueChanged, this, &MainWindow::pageSelected);
    auto nav = ui->pdfView->pageNavigator();
    connect(nav, &QPdfPageNavigator::currentPageChanged, m_pageSelector, &QSpinBox::setValue);
    connect(nav, &QPdfPageNavigator::backAvailableChanged, ui->actionBack, &QAction::setEnabled);
    connect(nav, &QPdfPageNavigator::forwardAvailableChanged, ui->actionForward, &QAction::setEnabled);

    connect(m_zoomSelector, &ZoomSelector::zoomModeChanged, ui->pdfView, &QPdfView::setZoomMode);
    connect(m_zoomSelector, &ZoomSelector::zoomFactorChanged, ui->pdfView, &QPdfView::setZoomFactor);
    m_zoomSelector->reset();

    QPdfBookmarkModel *bookmarkModel = new QPdfBookmarkModel(this);
    bookmarkModel->setDocument(m_document);

    ui->bookmarkView->setModel(bookmarkModel);
    connect(ui->bookmarkView, SIGNAL(activated(QModelIndex)), this, SLOT(bookmarkSelected(QModelIndex)));

    ui->tabWidget->setTabEnabled(1, false); // disable 'Pages' tab for now

    ui->pdfView->setDocument(m_document);

    connect(ui->pdfView, &QPdfView::zoomFactorChanged,
            m_zoomSelector, &ZoomSelector::setZoomFactor);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open(const QUrl &docLocation)
{
    if (docLocation.isLocalFile()) {
        m_document->load(docLocation.toLocalFile());
        const auto documentTitle = m_document->metaData(QPdfDocument::MetaDataField::Title).toString();
        setWindowTitle(!documentTitle.isEmpty() ? documentTitle : QStringLiteral("PDF Viewer"));
        pageSelected(0);
        m_pageSelector->setMaximum(m_document->pageCount() - 1);
    } else {
        qCDebug(lcExample) << docLocation << "is not a valid local file";
        QMessageBox::critical(this, tr("Failed to open"), tr("%1 is not a valid local file").arg(docLocation.toString()));
    }
    qCDebug(lcExample) << docLocation;
}

void MainWindow::bookmarkSelected(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const int page = index.data(int(QPdfBookmarkModel::Role::Page)).toInt();
    const qreal zoomLevel = index.data(int(QPdfBookmarkModel::Role::Level)).toReal();
    ui->pdfView->pageNavigator()->jump(page, {}, zoomLevel);
}

void MainWindow::pageSelected(int page)
{
    auto nav = ui->pdfView->pageNavigator();
    nav->jump(page, {}, nav->currentZoom());
}

void MainWindow::on_actionOpen_triggered()
{
    QUrl toOpen = QFileDialog::getOpenFileUrl(this, tr("Choose a PDF"), QUrl(), "Portable Documents (*.pdf)");
    if (toOpen.isValid())
        open(toOpen);
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About PdfViewer"),
        tr("An example using QPdfDocument"));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionZoom_In_triggered()
{
    ui->pdfView->setZoomFactor(ui->pdfView->zoomFactor() * zoomMultiplier);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    ui->pdfView->setZoomFactor(ui->pdfView->zoomFactor() / zoomMultiplier);
}

void MainWindow::on_actionPrevious_Page_triggered()
{
    auto nav = ui->pdfView->pageNavigator();
    nav->jump(nav->currentPage() - 1, {}, nav->currentZoom());
}

void MainWindow::on_actionNext_Page_triggered()
{
    auto nav = ui->pdfView->pageNavigator();
    nav->jump(nav->currentPage() + 1, {}, nav->currentZoom());
}

void MainWindow::on_actionContinuous_triggered()
{
    ui->pdfView->setPageMode(ui->actionContinuous->isChecked() ?
                                 QPdfView::PageMode::MultiPage :
                                 QPdfView::PageMode::SinglePage);
}

void MainWindow::on_actionBack_triggered()
{
    ui->pdfView->pageNavigator()->back();
}

void MainWindow::on_actionForward_triggered()
{
    ui->pdfView->pageNavigator()->forward();
}
