// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#define QT_PDF_BUILD_REMOVED_API

#include "qtpdfglobal.h"

QT_USE_NAMESPACE

#if QT_PDF_REMOVED_SINCE(6, 12)

#include "qpdfdocument.h"

QImage QPdfDocument::render(int page, QSize imageSize, QPdfDocumentRenderOptions renderOptions)
{
    return render(page, imageSize, renderOptions, QImage::Format_ARGB32, Qt::transparent);
}


// #include "qotherheader.h"
// implement removed functions from qotherheader.h
// order sections alphabetically

#endif // QT_PDF_REMOVED_SINCE(6, 12)
