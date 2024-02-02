// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
#include "private/qtpdfexports_p.h"

#include "third_party/pdfium/public/fpdfview.h"
#include "third_party/pdfium/public/fpdf_dataavail.h"

#include <QtCore/qbuffer.h>
#include <QtCore/qmutex.h>
#include <QtCore/qpointer.h>
#include <QtNetwork/qnetworkreply.h>

#include <mutex>

QT_BEGIN_NAMESPACE

class QPdfMutexLocker : public std::unique_lock<QRecursiveMutex>
{
public:
    QPdfMutexLocker();
};

class QPdfPageModel;

class Q_PDF_PRIVATE_EXPORT QPdfDocumentPrivate: public FPDF_FILEACCESS, public FX_FILEAVAIL, public FX_DOWNLOADHINTS
{
public:
    QPdfDocumentPrivate();
    ~QPdfDocumentPrivate();

    QPdfDocument *q;
    QPdfPageModel *pageModel = nullptr;

    FPDF_AVAIL avail;
    FPDF_DOCUMENT doc;
    bool loadComplete;

    QPointer<QIODevice> device;
    QScopedPointer<QIODevice> ownDevice;
    QBuffer asyncBuffer;
    QPointer<QIODevice> sequentialSourceDevice;
    QByteArray password;

    QPdfDocument::Status status;
    QPdfDocument::Error lastError;
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

    // FPDF takes the rotation parameter as an int.
    // This enum is mapping the int values defined in fpdfview.h:956.
    // (not using enum class to ensure int convertability)
    enum QFPDFRotation {
        Normal = 0,
        ClockWise90 = 1,
        ClockWise180 = 2,
        CounterClockWise90 = 3
    };

    static constexpr QFPDFRotation toFPDFRotation(QPdfDocumentRenderOptions::Rotation rotation)
    {
        switch (rotation) {
        case QPdfDocumentRenderOptions::Rotation::None:
            return QFPDFRotation::Normal;
        case QPdfDocumentRenderOptions::Rotation::Clockwise90:
            return QFPDFRotation::ClockWise90;
        case QPdfDocumentRenderOptions::Rotation::Clockwise180:
            return QFPDFRotation::ClockWise180;
        case QPdfDocumentRenderOptions::Rotation::Clockwise270:
            return QFPDFRotation::CounterClockWise90;
        }
        Q_UNREACHABLE();
    }

    struct TextPosition {
        QPointF position;
        qreal height = 0;
        int charIndex = -1;
    };
    TextPosition hitTest(int page, QPointF position);
};

QT_END_NAMESPACE

#endif // QPDFDOCUMENT_P_H
