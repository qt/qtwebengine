/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
