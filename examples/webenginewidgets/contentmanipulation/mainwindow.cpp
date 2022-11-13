// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>
#include <QtWebEngineWidgets>
#include "mainwindow.h"

MainWindow::MainWindow(const QUrl& url)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    progress = 0;

    QFile file;
    file.setFileName(":/jquery.min.js");
    file.open(QIODevice::ReadOnly);
    jQuery = file.readAll();
    jQuery.append("\nvar qt = { 'jQuery': jQuery.noConflict(true) };");
    file.close();

    view = new QWebEngineView(this);
    view->load(url);
    connect(view, &QWebEngineView::loadFinished, this, &MainWindow::adjustLocation);
    connect(view, &QWebEngineView::titleChanged, this, &MainWindow::adjustTitle);
    connect(view, &QWebEngineView::loadProgress, this, &MainWindow::setProgress);
    connect(view, &QWebEngineView::loadFinished, this, &MainWindow::finishLoading);

    locationEdit = new QLineEdit(this);
    locationEdit->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());
    connect(locationEdit, &QLineEdit::returnPressed, this, &MainWindow::changeLocation);

    QToolBar *toolBar = addToolBar(tr("Navigation"));
    toolBar->addAction(view->pageAction(QWebEnginePage::Back));
    toolBar->addAction(view->pageAction(QWebEnginePage::Forward));
    toolBar->addAction(view->pageAction(QWebEnginePage::Reload));
    toolBar->addAction(view->pageAction(QWebEnginePage::Stop));
    toolBar->addWidget(locationEdit);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    QAction *viewSourceAction = new QAction(tr("Page Source"), this);
    connect(viewSourceAction, &QAction::triggered, this, &MainWindow::viewSource);
    viewMenu->addAction(viewSourceAction);

    QMenu *effectMenu = menuBar()->addMenu(tr("&Effect"));
    effectMenu->addAction(tr("Highlight all links"), this, &MainWindow::highlightAllLinks);

    rotateAction = new QAction(this);
    rotateAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    rotateAction->setCheckable(true);
    rotateAction->setText(tr("Turn images upside down"));
    connect(rotateAction, &QAction::toggled, this, &MainWindow::rotateImages);
    effectMenu->addAction(rotateAction);

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(tr("Remove GIF images"), this, &MainWindow::removeGifImages);
    toolsMenu->addAction(tr("Remove all inline frames"), this, &MainWindow::removeInlineFrames);
    toolsMenu->addAction(tr("Remove all object elements"), this, &MainWindow::removeObjectElements);
    toolsMenu->addAction(tr("Remove all embedded elements"), this, &MainWindow::removeEmbeddedElements);

    setCentralWidget(view);
}

void MainWindow::viewSource()
{
    QTextEdit *textEdit = new QTextEdit(nullptr);
    textEdit->setAttribute(Qt::WA_DeleteOnClose);
    textEdit->adjustSize();
    textEdit->move(this->geometry().center() - textEdit->rect().center());
    textEdit->show();

    view->page()->toHtml([textEdit](const QString &html){
        textEdit->setPlainText(html);
    });
}

void MainWindow::adjustLocation()
{
    locationEdit->setText(view->url().toString());
}

void MainWindow::changeLocation()
{
    QUrl url = QUrl::fromUserInput(locationEdit->text());
    view->load(url);
    view->setFocus();
}

void MainWindow::adjustTitle()
{
    if (progress <= 0 || progress >= 100)
        setWindowTitle(view->title());
    else
        setWindowTitle(QStringLiteral("%1 (%2%)").arg(view->title()).arg(progress));
}

void MainWindow::setProgress(int p)
{
    progress = p;
    adjustTitle();
}

void MainWindow::finishLoading(bool)
{
    progress = 100;
    adjustTitle();
    view->page()->runJavaScript(jQuery);

    rotateImages(rotateAction->isChecked());
}

void MainWindow::highlightAllLinks()
{
    QString code = QStringLiteral("qt.jQuery('a').each( function () { qt.jQuery(this).css('background-color', 'yellow') } )");
    view->page()->runJavaScript(code);
}

void MainWindow::rotateImages(bool invert)
{
    QString code;

    if (invert)
        code = QStringLiteral("qt.jQuery('img').each( function () { qt.jQuery(this).css('transition', 'transform 2s'); qt.jQuery(this).css('transform', 'rotate(180deg)') } )");
    else
        code = QStringLiteral("qt.jQuery('img').each( function () { qt.jQuery(this).css('transition', 'transform 2s'); qt.jQuery(this).css('transform', 'rotate(0deg)') } )");
    view->page()->runJavaScript(code);
}

void MainWindow::removeGifImages()
{
    QString code = QStringLiteral("qt.jQuery('[src*=gif]').remove()");
    view->page()->runJavaScript(code);
}

void MainWindow::removeInlineFrames()
{
    QString code = QStringLiteral("qt.jQuery('iframe').remove()");
    view->page()->runJavaScript(code);
}

void MainWindow::removeObjectElements()
{
    QString code = QStringLiteral("qt.jQuery('object').remove()");
    view->page()->runJavaScript(code);
}

void MainWindow::removeEmbeddedElements()
{
    QString code = QStringLiteral("qt.jQuery('embed').remove()");
    view->page()->runJavaScript(code);
}
