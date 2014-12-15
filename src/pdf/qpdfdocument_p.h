#ifndef QPDFDOCUMENT_P_H
#define QPDFDOCUMENT_P_H

#include "fpdfview.h"

#include <qiodevice.h>

class QPdfDocumentPrivate
{
public:
    QPdfDocumentPrivate();
    ~QPdfDocumentPrivate();

    FPDF_DOCUMENT doc;
    QScopedPointer<QIODevice> device;
    QByteArray password;
};

#endif // QPDFDOCUMENT_P_H

