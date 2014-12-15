#include "qpdfdocument.h"

#include "qpdfdocument_p.h"

#include <QFile>
#include <QIODevice>
#include <QMutex>

// The library is not thread-safe at all, it has a lot of global variables.
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, pdfMutex, (QMutex::Recursive));
static int libraryRefCount;

QPdfDocumentPrivate::QPdfDocumentPrivate()
    : doc(0)
{
    QMutexLocker lock(pdfMutex());
    if (libraryRefCount == 0)
        FPDF_InitLibrary();
    ++libraryRefCount;

    // FPDF_FILEACCESS setup
    m_Param = this;
    m_GetBlock = fpdf_GetBlock;
}

QPdfDocumentPrivate::~QPdfDocumentPrivate()
{
    QMutexLocker lock(pdfMutex());
    if (doc)
        FPDF_CloseDocument(doc);
    doc = 0;

    if (!--libraryRefCount)
        FPDF_DestroyLibrary();
}

QPdfDocument::Error QPdfDocumentPrivate::load(QIODevice *newDevice, bool transferDeviceOwnership, const QString &documentPassword)
{
    QMutexLocker lock(pdfMutex());

    if (doc)
        FPDF_CloseDocument(doc);

    if (transferDeviceOwnership)
        ownDevice.reset(newDevice);
    else
        ownDevice.reset();
    device = newDevice;

    if (!device->isOpen() && !device->open(QIODevice::ReadOnly))
        return QPdfDocument::FileNotFoundError;

    // FPDF_FILEACCESS setup
    m_FileLen = device->size();

    password = documentPassword.toUtf8();

    doc = FPDF_LoadCustomDocument(this, password.constData());
    switch (FPDF_GetLastError()) {
    case FPDF_ERR_SUCCESS: return QPdfDocument::NoError;
    case FPDF_ERR_UNKNOWN: return QPdfDocument::UnknownError;
    case FPDF_ERR_FILE: return QPdfDocument::FileNotFoundError;
    case FPDF_ERR_FORMAT: return QPdfDocument::InvalidFileFormatError;
    case FPDF_ERR_PASSWORD: return QPdfDocument::IncorrectPasswordError;
    case FPDF_ERR_SECURITY: return QPdfDocument::UnsupportedSecuritySchemeError;
    default:
        Q_UNREACHABLE();
    }
    return QPdfDocument::UnknownError;
}

int QPdfDocumentPrivate::fpdf_GetBlock(void *param, unsigned long position, unsigned char *pBuf, unsigned long size)
{
    QPdfDocumentPrivate *d = static_cast<QPdfDocumentPrivate*>(reinterpret_cast<FPDF_FILEACCESS*>(param));
    d->device->seek(position);
    return qMax(qint64(0), d->device->read(reinterpret_cast<char *>(pBuf), size));

}

QPdfDocument::QPdfDocument(QObject *parent)
    : QObject(parent)
    , d(new QPdfDocumentPrivate)
{
}

QPdfDocument::~QPdfDocument()
{
}

QPdfDocument::Error QPdfDocument::load(const QString &fileName, const QString &password)
{
    return d->load(new QFile(fileName), /*transfer ownership*/true, password);
}

QPdfDocument::Error QPdfDocument::load(QIODevice *device, const QString &password)
{
    return d->load(device, /*transfer ownership*/false, password);
}

int QPdfDocument::pageCount() const
{
    if (!d->doc)
        return 0;
    QMutexLocker lock(pdfMutex());
    return FPDF_GetPageCount(d->doc);
}

QSizeF QPdfDocument::pageSize(int page) const
{
    QSizeF result;
    if (!d->doc)
        return result;
    QMutexLocker lock(pdfMutex());
    FPDF_GetPageSizeByIndex(d->doc, page, &result.rwidth(), &result.rheight());
    return result;
}

QImage QPdfDocument::render(int page, const QSizeF &pageSize)
{
    if (!d->doc)
        return QImage();

    QMutexLocker lock(pdfMutex());

    FPDF_PAGE pdfPage = FPDF_LoadPage(d->doc, page);
    if (!pdfPage)
        return QImage();

    QImage result(pageSize.toSize(), QImage::Format_ARGB32);
    result.fill(Qt::transparent);
    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(result.width(), result.height(), FPDFBitmap_BGRA, result.bits(), result.bytesPerLine());

    FPDF_RenderPageBitmap(bitmap, pdfPage, 0, 0, result.width(), result.height(), 0, 0);

    FPDFBitmap_Destroy(bitmap);
    return result;
}
