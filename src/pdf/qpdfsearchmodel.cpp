/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qpdfsearchmodel.h"
#include "qpdfsearchmodel_p.h"
#include "qpdfdocument_p.h"

#include "third_party/pdfium/public/fpdf_doc.h"
#include "third_party/pdfium/public/fpdf_text.h"

#include <QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcS, "qt.pdf.search")

QPdfSearchModel::QPdfSearchModel(QObject *parent)
    : QObject(parent),
      d(new QPdfSearchModelPrivate())
{
}

QPdfSearchModel::~QPdfSearchModel() {}

QVector<QRectF> QPdfSearchModel::matches(int page, const QString &searchString)
{
    const QPdfMutexLocker lock;
    FPDF_PAGE pdfPage = FPDF_LoadPage(d->document->d->doc, page);
    if (!pdfPage) {
        qWarning() << "failed to load page" << page;
        return {};
    }
    double pageHeight = FPDF_GetPageHeight(pdfPage);
    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(pdfPage);
    if (!textPage) {
        qWarning() << "failed to load text of page" << page;
        FPDF_ClosePage(pdfPage);
        return {};
    }
    QVector<QRectF> ret;
    if (searchString.isEmpty())
        return ret;
    FPDF_SCHHANDLE sh = FPDFText_FindStart(textPage, searchString.utf16(), 0, 0);
    while (FPDFText_FindNext(sh)) {
        int idx = FPDFText_GetSchResultIndex(sh);
        int count = FPDFText_GetSchCount(sh);
        int rectCount = FPDFText_CountRects(textPage, idx, count);
        qCDebug(qLcS) << searchString << ": matched" << count << "chars @" << idx << "across" << rectCount << "rects";
        for (int r = 0; r < rectCount; ++r) {
            double left, top, right, bottom;
            FPDFText_GetRect(textPage, r, &left, &top, &right, &bottom);
            ret << QRectF(left, pageHeight - top, right - left, top - bottom);
            qCDebug(qLcS) << ret.last();
        }
    }
    FPDFText_FindClose(sh);
    FPDFText_ClosePage(textPage);
    FPDF_ClosePage(pdfPage);

    return ret;
}

QPdfDocument *QPdfSearchModel::document() const
{
    return d->document;
}

void QPdfSearchModel::setDocument(QPdfDocument *document)
{
    if (d->document == document)
        return;
    d->document = document;
    emit documentChanged();
}

QPdfSearchModelPrivate::QPdfSearchModelPrivate()
{
}

QT_END_NAMESPACE

#include "moc_qpdfsearchmodel.cpp"
