#ifndef QPDFDOCUMENT_P_H
#define QPDFDOCUMENT_P_H

#include "fpdfview.h"
#include "fpdf_dataavail.h"
#include "qpdfdocument.h"

#include <qiodevice.h>

class QPdfDocumentPrivate: public FPDF_FILEACCESS
{
public:
    QPdfDocumentPrivate();
    ~QPdfDocumentPrivate();

    FPDF_DOCUMENT doc;

    QIODevice *device;
    QScopedPointer<QIODevice> ownDevice;
    QByteArray password;

    QPdfDocument::Error load(QIODevice *device, bool ownDevice, const QString &documentPassword);

    static bool fpdf_IsDataAvail(struct _FX_FILEAVAIL* pThis, size_t offset, size_t size);
    static int fpdf_GetBlock(void* param, unsigned long position, unsigned char* pBuf, unsigned long size);
};

#endif // QPDFDOCUMENT_P_H

