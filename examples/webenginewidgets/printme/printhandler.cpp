// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "printhandler.h"
#include <QPrintDialog>
#include <QPainter>
#include <QPrintPreviewDialog>
#include <QWebEngineView>

PrintHandler::PrintHandler(QObject *parent)
    : QObject(parent)
{
    m_printer.setResolution(300);
}

void PrintHandler::setView(QWebEngineView *view)
{
    Q_ASSERT(!m_view);
    m_view = view;
    connect(view, &QWebEngineView::printRequested, this, &PrintHandler::printPreview);
    connect(view, &QWebEngineView::printFinished, this, &PrintHandler::printFinished);
}

void PrintHandler::print()
{
    QPrintDialog dialog(&m_printer, m_view);
    if (dialog.exec() != QDialog::Accepted)
        return;
    printDocument(&m_printer);
}

void PrintHandler::printDocument(QPrinter *printer)
{
    m_view->print(printer);
    m_waitForResult.exec();
}

void PrintHandler::printFinished(bool success)
{
    if (!success) {
        QPainter painter;
        if (painter.begin(&m_printer)) {
            QFont font = painter.font();
            font.setPixelSize(20);
            painter.setFont(font);
            painter.drawText(QPointF(10,25),
                             QStringLiteral("Could not generate print preview."));
            painter.end();
        }
    }
    m_waitForResult.quit();
}

void PrintHandler::printPreview()
{
    if (!m_view)
        return;
    if (m_inPrintPreview)
        return;
    m_inPrintPreview = true;
    QPrintPreviewDialog preview(&m_printer, m_view);
    connect(&preview, &QPrintPreviewDialog::paintRequested,
            this, &PrintHandler::printDocument);
    preview.exec();
    m_inPrintPreview = false;
}
