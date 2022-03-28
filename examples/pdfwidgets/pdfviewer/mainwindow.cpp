/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "zoomselector.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QPdfBookmarkModel>
#include <QPdfDocument>
#include <QPdfNavigationStack>
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
    auto nav = ui->pdfView->pageNavigation();
    connect(nav, &QPdfNavigationStack::currentPageChanged, m_pageSelector, &QSpinBox::setValue);
    connect(nav, &QPdfNavigationStack::backAvailableChanged, ui->actionBack, &QAction::setEnabled);
    connect(nav, &QPdfNavigationStack::forwardAvailableChanged, ui->actionForward, &QAction::setEnabled);

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
        const auto documentTitle = m_document->metaData(QPdfDocument::Title).toString();
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

    const int page = index.data(QPdfBookmarkModel::PageNumberRole).toInt();
    const qreal zoomLevel = index.data(QPdfBookmarkModel::LevelRole).toReal();
    ui->pdfView->pageNavigation()->jump(page, {}, zoomLevel);
}

void MainWindow::pageSelected(int page)
{
    auto nav = ui->pdfView->pageNavigation();
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
    auto nav = ui->pdfView->pageNavigation();
    nav->jump(nav->currentPage() - 1, {}, nav->currentZoom());
}

void MainWindow::on_actionNext_Page_triggered()
{
    auto nav = ui->pdfView->pageNavigation();
    nav->jump(nav->currentPage() + 1, {}, nav->currentZoom());
}

void MainWindow::on_actionContinuous_triggered()
{
    ui->pdfView->setPageMode(ui->actionContinuous->isChecked() ? QPdfView::MultiPage : QPdfView::SinglePage);
}

void MainWindow::on_actionBack_triggered()
{
    ui->pdfView->pageNavigation()->back();
}

void MainWindow::on_actionForward_triggered()
{
    ui->pdfView->pageNavigation()->forward();
}
