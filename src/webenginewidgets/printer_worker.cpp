/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "printer_worker.h"

#include "printing/pdfium_document_wrapper_qt.h"

#include <QPainter>
#include <QPrinter>

namespace QtWebEngineCore {

PrinterWorker::PrinterWorker(QSharedPointer<QByteArray> data, QPrinter *printer)
    : m_data(data)
    , m_printer(printer)
{
}

PrinterWorker::~PrinterWorker()
{
}

void PrinterWorker::print()
{
    if (!m_data->size()) {
        qWarning("Failure to print on printer %ls: Print result data is empty.",
                 qUtf16Printable(m_printer->printerName()));
        Q_EMIT resultReady(false);
        return;
    }

    PdfiumDocumentWrapperQt pdfiumWrapper(m_data->constData(), m_data->size());

    int toPage = m_printer->toPage();
    int fromPage = m_printer->fromPage();
    bool ascendingOrder = true;

    if (fromPage == 0 && toPage == 0) {
        fromPage = 1;
        toPage = pdfiumWrapper.pageCount();
    }
    fromPage = qMax(1, fromPage);
    toPage = qMin(pdfiumWrapper.pageCount(), toPage);

    if (m_printer->pageOrder() == QPrinter::LastPageFirst) {
        qSwap(fromPage, toPage);
        ascendingOrder = false;
    }

    int pageCopies = 1;
    int documentCopies = 1;

    if (!m_printer->supportsMultipleCopies())
        documentCopies = m_printer->copyCount();

    if (m_printer->collateCopies()) {
        pageCopies = documentCopies;
        documentCopies = 1;
    }

    bool isLandscape = pdfiumWrapper.pageIsLandscape(0);
    QPageLayout::Orientation prevOrientation = m_printer->pageLayout().orientation();
    m_printer->setPageOrientation(isLandscape ? QPageLayout::Landscape : QPageLayout::Portrait);

    QPainter painter;
    if (!painter.begin(m_printer)) {
        qWarning("Failure to print on printer %ls: Could not open printer for painting.",
                  qUtf16Printable(m_printer->printerName()));
        Q_EMIT resultReady(false);
        return;
    }

    for (int printedDocuments = 0; printedDocuments < documentCopies; printedDocuments++) {
        if (printedDocuments > 0)
            m_printer->newPage();

        int currentPageIndex = fromPage;

        for (int i = 0; true; i++) {
            prevOrientation = m_printer->pageLayout().orientation();
            isLandscape = pdfiumWrapper.pageIsLandscape(currentPageIndex - 1);
            m_printer->setPageOrientation(isLandscape ? QPageLayout::Landscape : QPageLayout::Portrait);

            QSize pageSize = m_printer->pageRect().size();

            if (i > 0)
                m_printer->newPage();

            for (int printedPages = 0; printedPages < pageCopies; printedPages++) {
                if (m_printer->printerState() == QPrinter::Aborted
                        || m_printer->printerState() == QPrinter::Error) {
                    Q_EMIT resultReady(false);
                    return;
                }

                if (printedPages > 0)
                    m_printer->newPage();

                QImage currentImage = pdfiumWrapper.pageAsQImage(currentPageIndex - 1);
                if (currentImage.isNull()) {
                    Q_EMIT resultReady(false);
                    return;
                }

                QRect targetRect = currentImage.rect();
                // Scale down currentImage by both width and height to fit into the drawable area of the page.
                float scaleFactor = (float)pageSize.width() / (float)targetRect.width();
                targetRect = QRect(0, 0, targetRect.width() * scaleFactor, targetRect.height() * scaleFactor);
                scaleFactor = (float)pageSize.height() / (float)targetRect.height();
                targetRect = QRect(0, 0, targetRect.width() * scaleFactor, targetRect.height() * scaleFactor);

                // Painting operations are automatically clipped to the bounds of the drawable part of the page.
                painter.drawImage(targetRect, currentImage, currentImage.rect());
            }

            if (currentPageIndex == toPage)
                break;

            if (ascendingOrder)
                currentPageIndex++;
            else
                currentPageIndex--;

            m_printer->setPageOrientation(prevOrientation);
        }
    }
    painter.end();

    Q_EMIT resultReady(true);
    return;
}

} // namespace QtWebEngineCore
