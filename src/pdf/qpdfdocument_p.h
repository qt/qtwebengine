#ifndef QPDFDOCUMENT_P_H
#define QPDFDOCUMENT_P_H

#include "qpdfdocument.h"

#include "public/fpdfview.h"
#include "public/fpdf_dataavail.h"

#include <qbuffer.h>
#include <qnetworkreply.h>
#include <qpointer.h>

QT_BEGIN_NAMESPACE

class QPdfDocumentPrivate: public FPDF_FILEACCESS, public FX_FILEAVAIL, public FX_DOWNLOADHINTS
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

    QPdfDocument::Error lastError;

    void clear();

    void load(QIODevice *device, bool ownDevice);
    void loadAsync(QIODevice *device);

    void _q_tryLoadingWithSizeFromContentHeader();
    void initiateAsyncLoadWithTotalSizeKnown(quint64 totalSize);
    void _q_copyFromSequentialSourceDevice();
    void tryLoadDocument();
    void checkComplete();

    static FPDF_BOOL fpdf_IsDataAvail(struct _FX_FILEAVAIL* pThis, size_t offset, size_t size);
    static int fpdf_GetBlock(void* param, unsigned long position, unsigned char* pBuf, unsigned long size);
    static void fpdf_AddSegment(struct _FX_DOWNLOADHINTS* pThis, size_t offset, size_t size);
    void updateLastError();
};

QT_END_NAMESPACE

#endif // QPDFDOCUMENT_P_H
