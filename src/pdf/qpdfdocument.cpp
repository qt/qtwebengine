#include "qpdfdocument.h"

#include "qpdfdocument_p.h"

#include <QFile>
#include <QIODevice>
#include <QMutex>

static int libraryRefCount;
static QMutex libraryInitializerMutex;

// PDFium stores the error code when loading a document in a global
// variable, but that is only set from the FPDF_Load*Document functions.
// Therefore this mutex serializes access to the loading.
static QMutex documentLoadMutex;

QPdfDocumentPrivate::QPdfDocumentPrivate()
    : doc(0)
{
    {
        QMutexLocker lock(&libraryInitializerMutex);
        if (libraryRefCount == 0)
            FPDF_InitLibrary();
        ++libraryRefCount;
    }
}

QPdfDocumentPrivate::~QPdfDocumentPrivate()
{
    if (doc)
        FPDF_CloseDocument(doc);
    doc = 0;

    {
        QMutexLocker lock(&libraryInitializerMutex);
        if (!--libraryRefCount)
            FPDF_DestroyLibrary();
    }
}

QPdfDocument::QPdfDocument(QObject *parent)
    : QObject(parent)
    , d(new QPdfDocumentPrivate)
{
}

QPdfDocument::~QPdfDocument()
{
}

static int fpdf_GetBlock(void* param, unsigned long position, unsigned char* pBuf, unsigned long size)
{
    QIODevice *dev = reinterpret_cast<QIODevice*>(param);
    dev->seek(position);
    return dev->read(reinterpret_cast<char *>(pBuf), size);
}

QPdfDocument::Error QPdfDocument::load(const QString &fileName, const QString &password)
{
    if (d->doc)
        FPDF_CloseDocument(d->doc);

    QFile *file = new QFile(fileName);
    d->device.reset(file);

    if (!d->device->open(QIODevice::ReadOnly))
        return FileNotFoundError;

    FPDF_FILEACCESS access;
    access.m_FileLen = file->size();
    access.m_GetBlock = fpdf_GetBlock;
    access.m_Param = d->device.data();

    d->password = password.toUtf8();

    QMutexLocker loadLocker(&documentLoadMutex);
    d->doc = FPDF_LoadCustomDocument(&access, d->password.constData());
    switch (FPDF_GetLastError()) {
    case FPDF_ERR_SUCCESS: return NoError;
    case FPDF_ERR_UNKNOWN: return UnknownError;
    case FPDF_ERR_FILE: return FileNotFoundError;
    case FPDF_ERR_FORMAT: return InvalidFileFormatError;
    case FPDF_ERR_PASSWORD: return IncorrectPasswordError;
    case FPDF_ERR_SECURITY: return UnsupportedSecuritySchemeError;
    default:
        Q_UNREACHABLE();
    }
    return UnknownError;
}

int QPdfDocument::pageCount() const
{
    if (!d->doc)
        return 0;
    return FPDF_GetPageCount(d->doc);
}
