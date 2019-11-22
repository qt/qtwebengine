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

    qreal resolution = m_printer->resolution() / 72.0; // pdfium uses points so 1/72 inch

    QPainter painter;

    for (int printedDocuments = 0; printedDocuments < documentCopies; printedDocuments++) {
        if (printedDocuments > 0)
            m_printer->newPage();

        int currentPageIndex = fromPage;

        for (int i = 0; true; i++) {
            QSizeF documentSize = (pdfiumWrapper.pageSize(currentPageIndex - 1) * resolution);
            bool isLandscape = documentSize.width() > documentSize.height();
            m_printer->setPageOrientation(isLandscape ? QPageLayout::Landscape : QPageLayout::Portrait);
            QRectF pageRect = m_printer->pageRect(QPrinter::DevicePixel);
            documentSize = documentSize.scaled(pageRect.size(), Qt::KeepAspectRatio);

            // setPageOrientation has to be called before qpainter.begin() or before qprinter.newPage() so correct metrics is used,
            // therefore call begin now for only first page
            if (!painter.isActive() && !painter.begin(m_printer)) {
                qWarning("Failure to print on printer %ls: Could not open printer for painting.",
                          qUtf16Printable(m_printer->printerName()));
                Q_EMIT resultReady(false);
                return;
            }

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

                QImage currentImage = pdfiumWrapper.pageAsQImage(currentPageIndex - 1,documentSize.width(),documentSize.height());
                if (currentImage.isNull()) {
                    Q_EMIT resultReady(false);
                    return;
                }
                painter.drawImage(0,0, currentImage);
            }

            if (currentPageIndex == toPage)
                break;

            if (ascendingOrder)
                currentPageIndex++;
            else
                currentPageIndex--;
        }
    }
    painter.end();

    Q_EMIT resultReady(true);
    return;
}

} // namespace QtWebEngineCore
