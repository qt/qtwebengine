// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "pdfium_document_wrapper_qt.h"
#include <QtCore/qhash.h>
#include <QtGui/qimage.h>
#include <QtGui/qpainter.h>

#include "third_party/pdfium/public/fpdf_doc.h"
#include "third_party/pdfium/public/fpdfview.h"

namespace QtWebEngineCore {
int PdfiumDocumentWrapperQt::m_libraryUsers = 0;

PdfiumDocumentWrapperQt::PdfiumDocumentWrapperQt(const void *pdfData, size_t size,
                                                 const char *password)
{
    Q_ASSERT(pdfData);
    Q_ASSERT(size);
    if (m_libraryUsers++ == 0)
        FPDF_InitLibrary();

    m_documentHandle = (void *)FPDF_LoadMemDocument(pdfData, static_cast<int>(size), password);
    m_pageCount = FPDF_GetPageCount((FPDF_DOCUMENT)m_documentHandle);
}

QImage PdfiumDocumentWrapperQt::pageAsQImage(size_t pageIndex,int width , int height)
{
    if (!m_documentHandle || !m_pageCount) {
        qWarning("Failure to generate QImage from invalid or empty PDF document.");
        return QImage();
    }

    if (static_cast<int>(pageIndex) >= m_pageCount) {
        qWarning("Failure to generate QImage from PDF data: index out of bounds.");
        return QImage();
    }

    FPDF_PAGE pageData(FPDF_LoadPage((FPDF_DOCUMENT)m_documentHandle, pageIndex));
    QImage image(width, height, QImage::Format_ARGB32);
    Q_ASSERT(!image.isNull());
    image.fill(0xFFFFFFFF);

    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(width, height,
                                             FPDFBitmap_BGRA,
                                             image.scanLine(0), image.bytesPerLine());
    Q_ASSERT(bitmap);
    FPDF_RenderPageBitmap(bitmap, pageData,
                          0, 0, width, height,
                          0, 0);
    FPDFBitmap_Destroy(bitmap);
    bitmap = nullptr;
    FPDF_ClosePage(pageData);
    return image;
}

QSizeF PdfiumDocumentWrapperQt::pageSize(size_t index)
{
    QSizeF size;
    FPDF_GetPageSizeByIndex((FPDF_DOCUMENT)m_documentHandle, index, &size.rwidth(), &size.rheight());
    return size;
}

PdfiumDocumentWrapperQt::~PdfiumDocumentWrapperQt()
{
    FPDF_CloseDocument((FPDF_DOCUMENT)m_documentHandle);
    if (--m_libraryUsers == 0)
        FPDF_DestroyLibrary();
}
}
