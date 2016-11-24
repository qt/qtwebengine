/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "pdfium_printing_wrapper_qt.h"

#include <QtCore/qhash.h>
#include <QtGui/qimage.h>
#include <QtGui/qpainter.h>
#include <QtPrintSupport/qprinter.h>

#include "third_party/pdfium/public/fpdf_doc.h"
#include "third_party/pdfium/public/fpdfview.h"

namespace QtWebEngineCore {
int PdfiumPrintingWrapperQt::m_libraryUsers = 0;

class PDFiumPageWrapper {
public:
    PDFiumPageWrapper(void *data, int pageIndex, int targetWidth, int targetHeight)
        : m_pageData(FPDF_LoadPage(data, pageIndex))
        , m_width(FPDF_GetPageWidth(m_pageData))
        , m_height(FPDF_GetPageHeight(m_pageData))
        , m_index(pageIndex)
        , m_image(createImage(targetWidth, targetHeight))
    {
    }

    PDFiumPageWrapper()
        : m_pageData(nullptr)
        , m_width(-1)
        , m_height(-1)
        , m_index(-1)
        , m_image(QImage())
    {
    }

    virtual ~PDFiumPageWrapper()
    {
        FPDF_ClosePage(m_pageData);
    }

    QImage image()
    {
        return m_image;
    }

private:
    QImage createImage(int targetWidth, int targetHeight)
    {
        Q_ASSERT(m_pageData);
        if (targetWidth <= 0)
            targetWidth = m_width;

        if (targetHeight <= 0)
            targetHeight = m_height;

        QImage image(targetWidth, targetHeight, QImage::Format_RGBA8888);
        Q_ASSERT(!image.isNull());
        image.fill(0xFFFFFFFF);

        FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(image.width(), image.height(),
                                                 FPDFBitmap_BGRA,
                                                 image.scanLine(0), image.bytesPerLine());
        Q_ASSERT(bitmap);

        FPDF_RenderPageBitmap(bitmap, m_pageData,
                              0, 0, image.width(), image.height(),
                              0, 0);
        FPDFBitmap_Destroy(bitmap);
        bitmap = nullptr;

        // Map BGRA to RGBA as PDFium currently does not support RGBA bitmaps directly
        for (int i = 0; i < image.height(); i++) {
            uchar *pixels = image.scanLine(i);
            for (int j = 0; j < image.width(); j++) {
                qSwap(pixels[0], pixels[2]);
                pixels += 4;
            }
        }
        return image;
    }

private:
    void *m_pageData;
    int m_width;
    int m_height;
    int m_index;
    QImage m_image;
};


PdfiumPrintingWrapperQt::PdfiumPrintingWrapperQt(const void *pdfData, size_t size, const char *password)
{
   Q_ASSERT(pdfData);
   Q_ASSERT(size);
   if (m_libraryUsers++ == 0)
       FPDF_InitLibrary();

   m_documentHandle = FPDF_LoadMemDocument(pdfData, static_cast<int>(size), password);
   m_pageCount = FPDF_GetPageCount(m_documentHandle);
}

bool PdfiumPrintingWrapperQt::printOnPrinter(QPrinter &printer)
{
    if (!m_documentHandle || !m_pageCount) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
        qWarning("Failure to print on printer %ls: invalid document.\n", qUtf16Printable(printer.printerName()));
#endif
        return false;
    }

    int toPage = printer.toPage();
    int fromPage = printer.fromPage();
    bool ascendingOrder = true;

    if (fromPage == 0 && toPage == 0) {
        fromPage = 1;
        toPage = m_pageCount;
    }
    fromPage = qMax(1, fromPage);
    toPage = qMin(m_pageCount, toPage);

    if (printer.pageOrder() == QPrinter::LastPageFirst) {
        qSwap(fromPage, toPage);
        ascendingOrder = false;
    }

    int documentCopies = printer.copyCount();
    int pageCopies = 1;
    if (printer.collateCopies()) {
        pageCopies = documentCopies;
        documentCopies = 1;
    }

    QRect printerPageRect = printer.pageRect();
    int doubledPrinterWidth = 2 * printerPageRect.width();
    int doubledPrinterHeight = 2 * printerPageRect.height();

    QPainter painter;
    if (!painter.begin(&printer)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
        qWarning("Failure to print on printer %ls: Could not open printer for painting.\n", qUtf16Printable(printer.printerName()));
#endif
        return false;
    }

    QHash<int, PDFiumPageWrapper*> cachedPages;
    for (int printedDocuments = 0; printedDocuments < documentCopies; printedDocuments++) {
        int currentPageIndex = fromPage;
        while (true) {
            for (int printedPages = 0; printedPages < pageCopies; printedPages++) {
                if (printer.printerState() == QPrinter::Aborted
                        || printer.printerState() == QPrinter::Error)
                    return false;

                PDFiumPageWrapper *currentPageWrapper;
                if (!cachedPages.contains(currentPageIndex - 1)) {
                    currentPageWrapper
                            = new PDFiumPageWrapper(m_documentHandle, currentPageIndex - 1
                                                    , doubledPrinterWidth, doubledPrinterHeight);
                    cachedPages.insert(currentPageIndex - 1, currentPageWrapper);
                } else {
                    currentPageWrapper = cachedPages.value(currentPageIndex - 1);
                }

                QImage currentImage = currentPageWrapper->image();
                painter.drawImage(printerPageRect, currentImage, currentImage.rect());
                if (printedPages < pageCopies - 1)
                    printer.newPage();
            }

            if (currentPageIndex == toPage)
                break;

            if (ascendingOrder)
                currentPageIndex++;
            else
                currentPageIndex--;

            printer.newPage();
        }
        if (printedDocuments < documentCopies - 1)
            printer.newPage();
    }
    painter.end();

    qDeleteAll(cachedPages);

    return true;
}

PdfiumPrintingWrapperQt::~PdfiumPrintingWrapperQt()
{
    FPDF_CloseDocument(m_documentHandle);
    if (--m_libraryUsers == 0)
        FPDF_DestroyLibrary();
}

}
