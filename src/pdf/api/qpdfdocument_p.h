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

#ifndef QPDFDOCUMENT_P_H
#define QPDFDOCUMENT_P_H

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

#include "qpdfdocument.h"

#include "third_party/pdfium/public/fpdfview.h"
#include "third_party/pdfium/public/fpdf_dataavail.h"

#include <QtCore/qbuffer.h>
#include <QtCore/qmutex.h>
#include <QtCore/qpointer.h>
#include <QtNetwork/qnetworkreply.h>

QT_BEGIN_NAMESPACE

class QPdfMutexLocker : public QMutexLocker
{
public:
    QPdfMutexLocker();
};

class Q_PDF_PRIVATE_EXPORT QPdfDocumentPrivate: public FPDF_FILEACCESS, public FX_FILEAVAIL, public FX_DOWNLOADHINTS
{
public:
    QPdfDocumentPrivate();
    ~QPdfDocumentPrivate();

    QPdfDocument *q;

    FPDF_AVAIL avail;
    FPDF_DOCUMENT doc;
    bool loadComplete;

    QPointer<QIODevice> device;
    QScopedPointer<QIODevice> ownDevice;
    QBuffer asyncBuffer;
    QPointer<QIODevice> sequentialSourceDevice;
    QByteArray password;

    QPdfDocument::Status status;
    QPdfDocument::DocumentError lastError;
    int pageCount;

    void clear();

    void load(QIODevice *device, bool ownDevice);
    void loadAsync(QIODevice *device);

    void _q_tryLoadingWithSizeFromContentHeader();
    void initiateAsyncLoadWithTotalSizeKnown(quint64 totalSize);
    void _q_copyFromSequentialSourceDevice();
    void tryLoadDocument();
    void checkComplete();
    bool checkPageComplete(int page);
    void setStatus(QPdfDocument::Status status);

    static FPDF_BOOL fpdf_IsDataAvail(struct _FX_FILEAVAIL* pThis, size_t offset, size_t size);
    static int fpdf_GetBlock(void* param, unsigned long position, unsigned char* pBuf, unsigned long size);
    static void fpdf_AddSegment(struct _FX_DOWNLOADHINTS* pThis, size_t offset, size_t size);
    void updateLastError();
    QString getText(FPDF_TEXTPAGE textPage, int startIndex, int count);
    QPointF getCharPosition(FPDF_TEXTPAGE textPage, double pageHeight, int charIndex);
    QRectF getCharBox(FPDF_TEXTPAGE textPage, double pageHeight, int charIndex);

    struct TextPosition {
        QPointF position;
        qreal height = 0;
        int charIndex = -1;
    };
    TextPosition hitTest(int page, QPointF position);
};

QT_END_NAMESPACE

#endif // QPDFDOCUMENT_P_H
