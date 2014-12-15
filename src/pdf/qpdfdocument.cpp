#include "qpdfdocument.h"

#include "qpdfdocument_p.h"

#include <QFile>
#include <QIODevice>
#include <QMutex>

// The library is not thread-safe at all, it has a lot of global variables.
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, pdfMutex, (QMutex::Recursive));
static int libraryRefCount;

QPdfDocumentPrivate::QPdfDocumentPrivate()
    : avail(0)
    , doc(0)
    , lastError(QPdfDocument::NoError)
{
    QMutexLocker lock(pdfMutex());
    if (libraryRefCount == 0)
        FPDF_InitLibrary();
    ++libraryRefCount;

    // FPDF_FILEACCESS setup
    m_Param = this;
    m_GetBlock = fpdf_GetBlock;

    // FX_FILEAVAIL setup
    FX_FILEAVAIL::version = 1;
    IsDataAvail = fpdf_IsDataAvail;

    // FX_DOWNLOADHINTS setup
    FX_DOWNLOADHINTS::version = 1;
    AddSegment = fpdf_AddSegment;
}

QPdfDocumentPrivate::~QPdfDocumentPrivate()
{
    QMutexLocker lock(pdfMutex());

    clear();

    if (!--libraryRefCount)
        FPDF_DestroyLibrary();
}

void QPdfDocumentPrivate::clear()
{
    if (doc)
        FPDF_CloseDocument(doc);
    doc = 0;

    if (avail)
        FPDFAvail_Destroy(avail);
    avail = 0;

    asyncBuffer.close();
    asyncBuffer.setData(QByteArray());
    asyncBuffer.open(QIODevice::ReadWrite);
}

void QPdfDocumentPrivate::setErrorCode()
{
    switch (FPDF_GetLastError()) {
    case FPDF_ERR_SUCCESS: lastError = QPdfDocument::NoError; break;
    case FPDF_ERR_UNKNOWN: lastError = QPdfDocument::UnknownError; break;
    case FPDF_ERR_FILE: lastError = QPdfDocument::FileNotFoundError; break;
    case FPDF_ERR_FORMAT: lastError = QPdfDocument::InvalidFileFormatError; break;
    case FPDF_ERR_PASSWORD: lastError = QPdfDocument::IncorrectPasswordError; break;
    case FPDF_ERR_SECURITY: lastError = QPdfDocument::UnsupportedSecuritySchemeError; break;
    default:
        Q_UNREACHABLE();
    }
}

QPdfDocument::Error QPdfDocumentPrivate::load(QIODevice *newDevice, bool transferDeviceOwnership, const QString &documentPassword)
{
    clear();

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
    setErrorCode();
    return lastError;
}

void QPdfDocumentPrivate::_q_initiateAsyncLoad()
{
    QMutexLocker lock(pdfMutex());
    if (avail)
        return;

    QVariant contentLength = remoteDevice->header(QNetworkRequest::ContentLengthHeader);
    if (!contentLength.isValid())
        return;

    // FPDF_FILEACCESS setup
    m_FileLen = contentLength.toULongLong();

    QObject::connect(remoteDevice, SIGNAL(readyRead()), q, SLOT(_q_readFromDevice()));

    avail = FPDFAvail_Create(this, this);

    if (remoteDevice->bytesAvailable())
        _q_readFromDevice();
}

void QPdfDocumentPrivate::_q_readFromDevice()
{
    QMutexLocker lock(pdfMutex());
    QByteArray data = remoteDevice->read(remoteDevice->bytesAvailable());
    if (data.isEmpty())
        return;
    asyncBuffer.seek(asyncBuffer.size());
    asyncBuffer.write(data);

    if (!doc) {
        tryLoadDocument();
    }
}

void QPdfDocumentPrivate::tryLoadDocument()
{
    if (!FPDFAvail_IsDocAvail(avail, this))
        return;

    Q_ASSERT(!doc);

    doc = FPDFAvail_GetDocument(avail, password);
    if (!doc) {
        setErrorCode();
        if (lastError == QPdfDocument::IncorrectPasswordError)
            emit q->passwordRequired();
    }
    if (doc)
        emit q->documentReady();
}

bool QPdfDocumentPrivate::fpdf_IsDataAvail(_FX_FILEAVAIL *pThis, size_t offset, size_t size)
{
    QPdfDocumentPrivate *d = static_cast<QPdfDocumentPrivate*>(pThis);
    return offset + size <= static_cast<quint64>(d->asyncBuffer.size());
}

int QPdfDocumentPrivate::fpdf_GetBlock(void *param, unsigned long position, unsigned char *pBuf, unsigned long size)
{
    QPdfDocumentPrivate *d = static_cast<QPdfDocumentPrivate*>(reinterpret_cast<FPDF_FILEACCESS*>(param));
    d->device->seek(position);
    return qMax(qint64(0), d->device->read(reinterpret_cast<char *>(pBuf), size));

}

void QPdfDocumentPrivate::fpdf_AddSegment(_FX_DOWNLOADHINTS *pThis, size_t offset, size_t size)
{
    Q_UNUSED(pThis);
    Q_UNUSED(offset);
    Q_UNUSED(size);
}

QPdfDocument::QPdfDocument(QObject *parent)
    : QObject(parent)
    , d(new QPdfDocumentPrivate)
{
    d->q = this;
}

QPdfDocument::~QPdfDocument()
{
}

QPdfDocument::Error QPdfDocument::load(const QString &fileName, const QString &password)
{
    QMutexLocker lock(pdfMutex());
    return d->load(new QFile(fileName), /*transfer ownership*/true, password);
}

QPdfDocument::Error QPdfDocument::load(QIODevice *device, const QString &password)
{
    QMutexLocker lock(pdfMutex());
    return d->load(device, /*transfer ownership*/false, password);
}

void QPdfDocument::loadAsynchronously(QNetworkReply *device)
{
    QMutexLocker lock(pdfMutex());
    d->clear();

    d->ownDevice.reset();
    d->device = &d->asyncBuffer;

    if (d->remoteDevice)
        d->remoteDevice->disconnect(this);

    d->remoteDevice = device;

    if (d->remoteDevice->header(QNetworkRequest::ContentLengthHeader).isValid())
        d->_q_initiateAsyncLoad();
    else
        connect(d->remoteDevice, SIGNAL(metaDataChanged()), this, SLOT(_q_initiateAsyncLoad()));
}

void QPdfDocument::setPassword(const QString &password)
{
    QMutexLocker lock(pdfMutex());
    d->password = password.toUtf8();

    if (!d->doc && d->avail)
        d->tryLoadDocument();
}

QString QPdfDocument::password() const
{
    return QString::fromUtf8(d->password);
}

QPdfDocument::Error QPdfDocument::error() const
{
    return d->lastError;
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

#include "moc_qpdfdocument.cpp"
