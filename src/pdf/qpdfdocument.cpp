/******************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt PDF Module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#include "qpdfdocument.h"

#include "qpdfdocument_p.h"

#include <QFile>
#include <QMutex>

QT_BEGIN_NAMESPACE

// The library is not thread-safe at all, it has a lot of global variables.
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, pdfMutex, (QMutex::Recursive));
static int libraryRefCount;

QPdfDocumentPrivate::QPdfDocumentPrivate()
    : avail(Q_NULLPTR)
    , doc(Q_NULLPTR)
    , loadComplete(true)
    , lastError(QPdfDocument::NoError)
{
    const QMutexLocker lock(pdfMutex());

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
    const QMutexLocker lock(pdfMutex());

    clear();

    if (!--libraryRefCount)
        FPDF_DestroyLibrary();
}

void QPdfDocumentPrivate::clear()
{
    if (doc)
        FPDF_CloseDocument(doc);
    doc = Q_NULLPTR;

    if (avail)
        FPDFAvail_Destroy(avail);
    avail = Q_NULLPTR;

    loadComplete = false;

    asyncBuffer.close();
    asyncBuffer.setData(QByteArray());
    asyncBuffer.open(QIODevice::ReadWrite);

    if (sequentialSourceDevice)
        sequentialSourceDevice->disconnect(q);
}

void QPdfDocumentPrivate::updateLastError()
{
    if (doc) {
        lastError = QPdfDocument::NoError;
        return;
    }
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

void QPdfDocumentPrivate::load(QIODevice *newDevice, bool transferDeviceOwnership)
{
    clear();

    if (transferDeviceOwnership)
        ownDevice.reset(newDevice);
    else
        ownDevice.reset();

    if (newDevice->isSequential()) {
        sequentialSourceDevice = newDevice;
        device = &asyncBuffer;
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(sequentialSourceDevice);
        if (!reply) {
            qWarning() << "QPdfDocument: Loading from sequential devices only supported with QNetworkAccessManager.";
            return;
        }

        if (reply->header(QNetworkRequest::ContentLengthHeader).isValid())
            _q_tryLoadingWithSizeFromContentHeader();
        else
            QObject::connect(reply, SIGNAL(metaDataChanged()), q, SLOT(_q_tryLoadingWithSizeFromContentHeader()));
    } else {
        device = newDevice;
        initiateAsyncLoadWithTotalSizeKnown(device->size());
        checkComplete();
    }
}

void QPdfDocumentPrivate::_q_tryLoadingWithSizeFromContentHeader()
{
    const QMutexLocker lock(pdfMutex());

    if (avail)
        return;

    const QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(sequentialSourceDevice);
    if(!networkReply)
        return;

    const QVariant contentLength = networkReply->header(QNetworkRequest::ContentLengthHeader);
    if (!contentLength.isValid())
        return;

    QObject::connect(sequentialSourceDevice, SIGNAL(readyRead()), q, SLOT(_q_copyFromSequentialSourceDevice()));

    initiateAsyncLoadWithTotalSizeKnown(contentLength.toULongLong());

    if (sequentialSourceDevice->bytesAvailable())
        _q_copyFromSequentialSourceDevice();
}

void QPdfDocumentPrivate::initiateAsyncLoadWithTotalSizeKnown(quint64 totalSize)
{
    // FPDF_FILEACCESS setup
    m_FileLen = totalSize;

    avail = FPDFAvail_Create(this, this);
}

void QPdfDocumentPrivate::_q_copyFromSequentialSourceDevice()
{
    const QMutexLocker lock(pdfMutex());

    if (loadComplete)
        return;

    const QByteArray data = sequentialSourceDevice->read(sequentialSourceDevice->bytesAvailable());
    if (data.isEmpty())
        return;

    asyncBuffer.seek(asyncBuffer.size());
    asyncBuffer.write(data);

    checkComplete();
}

void QPdfDocumentPrivate::tryLoadDocument()
{
    const QMutexLocker lock(pdfMutex());

    if (!FPDFAvail_IsDocAvail(avail, this))
        return;

    Q_ASSERT(!doc);

    doc = FPDFAvail_GetDocument(avail, password);
    updateLastError();
    if (lastError == QPdfDocument::IncorrectPasswordError)
        emit q->passwordRequired();
    else if (doc)
        emit q->documentLoadStarted();
}

void QPdfDocumentPrivate::checkComplete()
{
    const QMutexLocker lock(pdfMutex());

    if (!avail || loadComplete)
        return;

    if (!doc)
        tryLoadDocument();

    if (!doc)
        return;

    loadComplete = true;
    for (int i = 0, count = FPDF_GetPageCount(doc); i < count; ++i)
        if (!FPDFAvail_IsPageAvail(avail, i, this))
            loadComplete = false;

    if (loadComplete)
        emit q->documentLoadFinished();
}

FPDF_BOOL QPdfDocumentPrivate::fpdf_IsDataAvail(_FX_FILEAVAIL *pThis, size_t offset, size_t size)
{
    QPdfDocumentPrivate *d = static_cast<QPdfDocumentPrivate*>(pThis);
    return offset + size <= static_cast<quint64>(d->device->size());
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

QPdfDocument::Error QPdfDocument::load(const QString &fileName)
{
    const QMutexLocker lock(pdfMutex());

    QScopedPointer<QFile> f(new QFile(fileName));
    if (!f->open(QIODevice::ReadOnly)) {
        d->lastError = FileNotFoundError;
    } else {
        d->load(f.take(), /*transfer ownership*/true);
    }
    return d->lastError;
}

bool QPdfDocument::isLoading() const
{
    return !d->loadComplete;
}

void QPdfDocument::load(QIODevice *device)
{
    const QMutexLocker lock(pdfMutex());

    d->load(device, /*transfer ownership*/false);
}

void QPdfDocument::setPassword(const QString &password)
{
    const QMutexLocker lock(pdfMutex());

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

    const QMutexLocker lock(pdfMutex());

    return FPDF_GetPageCount(d->doc);
}

QSizeF QPdfDocument::pageSize(int page) const
{
    QSizeF result;
    if (!d->doc)
        return result;

    const QMutexLocker lock(pdfMutex());

    FPDF_GetPageSizeByIndex(d->doc, page, &result.rwidth(), &result.rheight());
    return result;
}

QImage QPdfDocument::render(int page, const QSizeF &pageSize)
{
    if (!d->doc)
        return QImage();

    const QMutexLocker lock(pdfMutex());

    FPDF_PAGE pdfPage = FPDF_LoadPage(d->doc, page);
    if (!pdfPage)
        return QImage();

    QImage result(pageSize.toSize(), QImage::Format_ARGB32);
    result.fill(Qt::transparent);
    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(result.width(), result.height(), FPDFBitmap_BGRA, result.bits(), result.bytesPerLine());

    FPDF_RenderPageBitmap(bitmap, pdfPage, 0, 0, result.width(), result.height(), 0, 0);

    FPDFBitmap_Destroy(bitmap);

    FPDF_ClosePage(pdfPage);

    return result;
}

QT_END_NAMESPACE

#include "moc_qpdfdocument.cpp"
