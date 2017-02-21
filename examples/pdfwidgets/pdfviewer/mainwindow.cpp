/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "sequentialpagewidget.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPdfBookmarkModel>
#include <QPdfDocument>
#include <QScroller>
#include <QtMath>

const qreal zoomMultiplier = qSqrt(2.0);

Q_LOGGING_CATEGORY(lcExample, "qt.examples.pdfviewer")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pageWidget(new SequentialPageWidget(this))
    , m_zoomEdit(new QLineEdit(this))
    , m_pageEdit(new QLineEdit(this))
    , m_document(new QPdfDocument(this))
{
    ui->setupUi(this);
    ui->scrollArea->setWidget(m_pageWidget);
    m_zoomEdit->setMaximumWidth(50);
    m_zoomEdit->setAlignment(Qt::AlignHCenter);
    ui->mainToolBar->insertWidget(ui->actionZoom_In, m_zoomEdit);
    m_pageEdit->setMaximumWidth(50);
    m_pageEdit->setAlignment(Qt::AlignHCenter);
    ui->mainToolBar->insertWidget(ui->actionGo, m_pageEdit);
    connect(m_pageWidget, SIGNAL(showingPageRange(int,int)),
            this, SLOT(showingPageRange(int,int)), Qt::QueuedConnection);
    connect(m_pageWidget, SIGNAL(zoomChanged(qreal)),
            this, SLOT(zoomChanged(qreal)));
    connect(m_zoomEdit, SIGNAL(returnPressed()), this, SLOT(zoomEdited()));
    connect(m_pageEdit, SIGNAL(returnPressed()), this, SLOT(on_actionGo_triggered()));

    QScroller::grabGesture(ui->scrollArea);

    QPdfBookmarkModel *bookmarkModel = new QPdfBookmarkModel(this);
    bookmarkModel->setDocument(m_document);

    ui->bookmarkView->setModel(bookmarkModel);
    connect(ui->bookmarkView, SIGNAL(activated(QModelIndex)), this, SLOT(bookmarkSelected(QModelIndex)));

    ui->tabWidget->setTabEnabled(1, false); // disable 'Pages' tab for now

    m_pageWidget->setDocument(m_document);
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
    } else {
        qCDebug(lcExample) << docLocation << "is not a valid local file";
        QMessageBox::critical(this, tr("Failed to open"), tr("%1 is not a valid local file").arg(docLocation.toString()));
    }
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
    m_pageEdit->setText(QString::number(end));
}

void MainWindow::zoomChanged(qreal factor)
{
    m_zoomEdit->setText(tr("%1%").arg(factor * 100., 0, 'f', 0));
}

void MainWindow::zoomEdited()
{
    bool ok = false;
    qreal factor = m_zoomEdit->text().remove(QChar('%')).toDouble(&ok);
    if (ok)
        m_pageWidget->setZoom(factor / 100.);
}

void MainWindow::bookmarkSelected(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const int page = index.data(QPdfBookmarkModel::PageNumberRole).toInt();
    ui->scrollArea->ensureVisible(0, m_pageWidget->yForPage(page));
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
    m_pageWidget->setZoom(m_pageWidget->zoom() * zoomMultiplier);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    m_pageWidget->setZoom(m_pageWidget->zoom() / zoomMultiplier);
}

void MainWindow::on_actionGo_triggered()
{
    bool ok = false;
    int page = m_pageEdit->text().toInt(&ok);
    if (!ok) return;
    ui->scrollArea->ensureVisible(0, m_pageWidget->yForPage(page));
}

void MainWindow::on_actionPrevious_Page_triggered()
{
    ui->scrollArea->ensureVisible(0, m_pageWidget->yForPage(m_pageWidget->topPageShowing() - 1));
}

void MainWindow::on_actionNext_Page_triggered()
{
    ui->scrollArea->ensureVisible(0, m_pageWidget->yForPage(m_pageWidget->bottomPageShowing() + 1));
}
