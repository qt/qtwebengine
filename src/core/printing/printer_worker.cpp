// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "printer_worker.h"

#include "printing/pdfium_document_wrapper_qt.h"

#include <QPainter>
#include <QPagedPaintDevice>

namespace QtWebEngineCore {

PrinterWorker::PrinterWorker(QSharedPointer<QByteArray> data, QPagedPaintDevice *device)
    : m_data(data), m_device(device)
{
}

PrinterWorker::~PrinterWorker() { }

void PrinterWorker::print()
{
    if (!m_data->size()) {
        qWarning("Failed to print: Print result data is empty.");
        Q_EMIT resultReady(false);
        return;
    }

    PdfiumDocumentWrapperQt pdfiumWrapper(m_data->constData(), m_data->size());

    const int fromPage = m_firstPageFirst ? 0 : pdfiumWrapper.pageCount() - 1;
    const int toPage = m_firstPageFirst ? pdfiumWrapper.pageCount() : -1;

    int pageCopies = 1;
    if (m_collateCopies) {
        pageCopies = m_documentCopies;
        m_documentCopies = 1;
    }

    qreal resolution = m_deviceResolution / 72.0; // pdfium uses points so 1/72 inch

    QPainter painter;

    for (int printedDocuments = 0; printedDocuments < m_documentCopies; printedDocuments++) {
        if (printedDocuments > 0)
            m_device->newPage();

        for (int i = fromPage; i != toPage; m_firstPageFirst ? i++ : i--) {
            QSizeF documentSize = (pdfiumWrapper.pageSize(i) * resolution);
            bool isLandscape = documentSize.width() > documentSize.height();
            m_device->setPageOrientation(isLandscape ? QPageLayout::Landscape
                                                      : QPageLayout::Portrait);
            QRectF paintRect = m_device->pageLayout().paintRectPixels(m_deviceResolution);
            documentSize = documentSize.scaled(paintRect.size(), Qt::KeepAspectRatio);

            // setPageOrientation has to be called before qpainter.begin() or before
            // qprinter.newPage() so correct metrics is used, therefore call begin now for only
            // first page
            if (!painter.isActive() && !painter.begin(m_device)) {
                qWarning("Failure to print on device: Could not open printer for painting.");
                Q_EMIT resultReady(false);
                return;
            }

            if (i != fromPage)
                m_device->newPage();

            for (int printedPages = 0; printedPages < pageCopies; printedPages++) {
                if (printedPages > 0)
                    m_device->newPage();

                QImage currentImage =
                        pdfiumWrapper.pageAsQImage(i, documentSize.width(), documentSize.height());
                if (currentImage.isNull()) {
                    Q_EMIT resultReady(false);
                    return;
                }
                painter.drawImage(0, 0, currentImage);
            }
        }
    }
    painter.end();

    Q_EMIT resultReady(true);
    return;
}

} // namespace QtWebEngineCore
