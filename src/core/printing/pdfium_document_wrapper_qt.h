// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef PDFIUM_DOCUMENT_WRAPPER_QT_H
#define PDFIUM_DOCUMENT_WRAPPER_QT_H

#include "qtwebenginecoreglobal_p.h"

#include <QtGui/qimage.h>

namespace QtWebEngineCore {

class Q_WEBENGINECORE_PRIVATE_EXPORT PdfiumDocumentWrapperQt
{
public:
    PdfiumDocumentWrapperQt(const void *pdfData, size_t size, const char *password = nullptr);
    virtual ~PdfiumDocumentWrapperQt();
    QImage pageAsQImage(size_t index, int width , int height);
    QSizeF pageSize(size_t index);
    int pageCount() const { return m_pageCount; }

private:
    static int m_libraryUsers;
    int m_pageCount;
    void *m_documentHandle;
};

} // namespace QtWebEngineCore
#endif // PDFIUM_DOCUMENT_WRAPPER_QT_H
