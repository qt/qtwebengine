// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpdfdocument.h"
#include "qpdfdocument_p.h"

#include "third_party/pdfium/public/fpdf_doc.h"
#include "third_party/pdfium/public/fpdf_text.h"

#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QHash>
#include <QLoggingCategory>
#include <QMetaEnum>
#include <QMutex>
#include <QPixmap>
#include <QVector2D>

#include <QtCore/private/qtools_p.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QRecursiveMutex, pdfMutex)
static int libraryRefCount;
static const double CharacterHitTolerance = 16.0;
Q_LOGGING_CATEGORY(qLcDoc, "qt.pdf.document")

QPdfMutexLocker::QPdfMutexLocker()
    : std::unique_lock<QRecursiveMutex>(*pdfMutex())
{
}

class Q_PDF_EXPORT QPdfPageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    QPdfPageModel(QPdfDocument *doc) : QAbstractListModel(doc)
    {
        m_roleNames = QAbstractItemModel::roleNames();
        QMetaEnum rolesMetaEnum = doc->metaObject()->enumerator(doc->metaObject()->indexOfEnumerator("PageModelRole"));
        for (int r = Qt::UserRole; r < int(QPdfDocument::PageModelRole::NRoles); ++r) {
            auto name = QByteArray(rolesMetaEnum.valueToKey(r));
            name[0] = QtMiscUtils::toAsciiLower(name[0]);
            m_roleNames.insert(r, name);
        }
        connect(doc, &QPdfDocument::statusChanged, this, [this](QPdfDocument::Status s) {
            if (s == QPdfDocument::Status::Loading)
                beginResetModel();
            else if (s == QPdfDocument::Status::Ready)
                endResetModel();
        });
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid())
            return QVariant();

        switch (QPdfDocument::PageModelRole(role)) {
        case QPdfDocument::PageModelRole::Label:
            return document()->pageLabel(index.row());
        case QPdfDocument::PageModelRole::PointSize:
            return document()->pagePointSize(index.row());
        case QPdfDocument::PageModelRole::NRoles:
            break;
        }

        switch (role) {
        case Qt::DecorationRole:
            return pageThumbnail(index.row());
        case Qt::DisplayRole:
            return document()->pageLabel(index.row());
        }

        return QVariant();
    }

    int rowCount(const QModelIndex & = QModelIndex()) const override { return document()->pageCount(); }

    QHash<int, QByteArray> roleNames() const override { return m_roleNames; }

private:
    QPdfDocument *document() const { return static_cast<QPdfDocument *>(parent()); }
    QPixmap pageThumbnail(int page) const
    {
        auto it = m_thumbnails.constFind(page);
        if (it == m_thumbnails.constEnd()) {
            auto doc = document();
            auto size = doc->pagePointSize(page);
            size.scale(128, 128, Qt::KeepAspectRatio);
            // TODO use QPdfPageRenderer for threading?
            auto image = document()->render(page, size.toSize());
            QPixmap ret = QPixmap::fromImage(image);
            m_thumbnails.insert(page, ret);
            return ret;
        }
        return it.value();
    }

    QHash<int, QByteArray> m_roleNames;
    mutable QHash<int, QPixmap> m_thumbnails;
};

QPdfDocumentPrivate::QPdfDocumentPrivate()
    : avail(nullptr)
    , doc(nullptr)
    , loadComplete(false)
    , status(QPdfDocument::Status::Null)
    , lastError(QPdfDocument::Error::None)
    , pageCount(0)
{
    asyncBuffer.setData(QByteArray());
    asyncBuffer.open(QIODevice::ReadWrite);

    const QPdfMutexLocker lock;

    if (libraryRefCount == 0) {
        QElapsedTimer timer;
        timer.start();
        FPDF_InitLibrary();
        qCDebug(qLcDoc) << "FPDF_InitLibrary took" << timer.elapsed() << "ms";
    }
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
    q->close();

    const QPdfMutexLocker lock;

    if (!--libraryRefCount) {
        qCDebug(qLcDoc) << "FPDF_DestroyLibrary";
        FPDF_DestroyLibrary();
    }
}

void QPdfDocumentPrivate::clear()
{
    QPdfMutexLocker lock;

    if (doc)
        FPDF_CloseDocument(doc);
    doc = nullptr;

    if (avail)
        FPDFAvail_Destroy(avail);
    avail = nullptr;
    lock.unlock();

    if (pageCount != 0) {
        pageCount = 0;
        emit q->pageCountChanged(pageCount);
        emit q->pageModelChanged();
    }

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
        lastError = QPdfDocument::Error::None;
        return;
    }

    QPdfMutexLocker lock;
    const unsigned long error = FPDF_GetLastError();
    lock.unlock();

    switch (error) {
    case FPDF_ERR_SUCCESS: lastError = QPdfDocument::Error::None; break;
    case FPDF_ERR_UNKNOWN: lastError = QPdfDocument::Error::Unknown; break;
    case FPDF_ERR_FILE: lastError = QPdfDocument::Error::FileNotFound; break;
    case FPDF_ERR_FORMAT: lastError = QPdfDocument::Error::InvalidFileFormat; break;
    case FPDF_ERR_PASSWORD: lastError = QPdfDocument::Error::IncorrectPassword; break;
    case FPDF_ERR_SECURITY: lastError = QPdfDocument::Error::UnsupportedSecurityScheme; break;
    default:
        Q_UNREACHABLE();
    }
    if (lastError != QPdfDocument::Error::None)
        qCDebug(qLcDoc) << "FPDF error" << error << "->" << lastError;
}

void QPdfDocumentPrivate::load(QIODevice *newDevice, bool transferDeviceOwnership)
{
    if (transferDeviceOwnership)
        ownDevice.reset(newDevice);
    else
        ownDevice.reset();

    if (newDevice->isSequential()) {
        sequentialSourceDevice = newDevice;
        device = &asyncBuffer;
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(sequentialSourceDevice);

        if (!reply) {
            setStatus(QPdfDocument::Status::Error);
            qWarning() << "QPdfDocument: Loading from sequential devices only supported with QNetworkAccessManager.";
            return;
        }

        if (reply->isFinished() && reply->error() != QNetworkReply::NoError) {
            setStatus(QPdfDocument::Status::Error);
            return;
        }

        QObject::connect(reply, &QNetworkReply::finished, q, [this, reply](){
            if (reply->error() != QNetworkReply::NoError || reply->bytesAvailable() == 0) {
                this->setStatus(QPdfDocument::Status::Error);
            }
        });

        if (reply->header(QNetworkRequest::ContentLengthHeader).isValid())
            _q_tryLoadingWithSizeFromContentHeader();
        else
            QObject::connect(reply, SIGNAL(metaDataChanged()), q, SLOT(_q_tryLoadingWithSizeFromContentHeader()));
    } else {
        device = newDevice;
        initiateAsyncLoadWithTotalSizeKnown(device->size());
        if (!avail) {
            setStatus(QPdfDocument::Status::Error);
            return;
        }

        if (!doc)
            tryLoadDocument();

        if (!doc) {
            updateLastError();
            setStatus(QPdfDocument::Status::Error);
            return;
        }

        QPdfMutexLocker lock;
        const int newPageCount = FPDF_GetPageCount(doc);
        lock.unlock();
        if (newPageCount != pageCount) {
            pageCount = newPageCount;
            emit q->pageCountChanged(pageCount);
            emit q->pageModelChanged();
        }

        // If it's a local file, and the first couple of pages are available,
        // probably the whole document is available.
        if (checkPageComplete(0) && (pageCount < 2 || checkPageComplete(1))) {
            setStatus(QPdfDocument::Status::Ready);
        } else {
            updateLastError();
            setStatus(QPdfDocument::Status::Error);
        }
    }
}

void QPdfDocumentPrivate::_q_tryLoadingWithSizeFromContentHeader()
{
    if (avail)
        return;

    const QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(sequentialSourceDevice);
    if (!networkReply) {
        setStatus(QPdfDocument::Status::Error);
        return;
    }

    const QVariant contentLength = networkReply->header(QNetworkRequest::ContentLengthHeader);
    if (!contentLength.isValid()) {
        setStatus(QPdfDocument::Status::Error);
        return;
    }

    QObject::connect(sequentialSourceDevice, SIGNAL(readyRead()), q, SLOT(_q_copyFromSequentialSourceDevice()));

    initiateAsyncLoadWithTotalSizeKnown(contentLength.toULongLong());

    if (sequentialSourceDevice->bytesAvailable())
        _q_copyFromSequentialSourceDevice();
}

void QPdfDocumentPrivate::initiateAsyncLoadWithTotalSizeKnown(quint64 totalSize)
{
    // FPDF_FILEACCESS setup
    m_FileLen = totalSize;

    const QPdfMutexLocker lock;

    avail = FPDFAvail_Create(this, this);
}

void QPdfDocumentPrivate::_q_copyFromSequentialSourceDevice()
{
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
    QPdfMutexLocker lock;
    switch (FPDFAvail_IsDocAvail(avail, this)) {
        case PDF_DATA_ERROR:
            qCDebug(qLcDoc) << "error loading";
            break;
        case PDF_DATA_NOTAVAIL:
            qCDebug(qLcDoc) << "data not yet available";
            lastError = QPdfDocument::Error::DataNotYetAvailable;
            break;
        case PDF_DATA_AVAIL:
            lastError = QPdfDocument::Error::None;
            break;
    }

    Q_ASSERT(!doc);

    doc = FPDFAvail_GetDocument(avail, password);
    lock.unlock();

    updateLastError();
    if (lastError != QPdfDocument::Error::None)
        setStatus(QPdfDocument::Status::Error);

    if (lastError == QPdfDocument::Error::IncorrectPassword) {
        FPDF_CloseDocument(doc);
        doc = nullptr;

        setStatus(QPdfDocument::Status::Error);
        emit q->passwordRequired();
    }
}

void QPdfDocumentPrivate::checkComplete()
{
    if (!avail || loadComplete)
        return;

    if (!doc)
        tryLoadDocument();

    if (!doc)
        return;

    loadComplete = true;

    QPdfMutexLocker lock;

    const int newPageCount = FPDF_GetPageCount(doc);
    for (int i = 0; i < newPageCount; ++i) {
        int result = PDF_DATA_NOTAVAIL;
        while (result == PDF_DATA_NOTAVAIL) {
            result = FPDFAvail_IsPageAvail(avail, i, this);
        }

        if (result == PDF_DATA_ERROR)
            loadComplete = false;
    }

    lock.unlock();

    if (loadComplete) {
        if (newPageCount != pageCount) {
            pageCount = newPageCount;
            emit q->pageCountChanged(pageCount);
            emit q->pageModelChanged();
        }

        setStatus(QPdfDocument::Status::Ready);
    }
}

bool QPdfDocumentPrivate::checkPageComplete(int page)
{
    if (page < 0 || page >= pageCount)
        return false;

    if (loadComplete)
        return true;

    QPdfMutexLocker lock;
    int result = PDF_DATA_NOTAVAIL;
    while (result == PDF_DATA_NOTAVAIL)
        result = FPDFAvail_IsPageAvail(avail, page, this);
    lock.unlock();

    if (result == PDF_DATA_ERROR)
        updateLastError();

    return (result != PDF_DATA_ERROR);
}

void QPdfDocumentPrivate::setStatus(QPdfDocument::Status documentStatus)
{
    if (status == documentStatus)
        return;

    status = documentStatus;
    emit q->statusChanged(status);
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

QString QPdfDocumentPrivate::getText(FPDF_TEXTPAGE textPage, int startIndex, int count)
{
    QList<ushort> buf(count + 1);
    // TODO is that enough space in case one unicode character is more than one in utf-16?
    int len = FPDFText_GetText(textPage, startIndex, count, buf.data());
    Q_ASSERT(len - 1 <= count); // len is number of characters written, including the terminator
    return QString::fromUtf16(reinterpret_cast<const char16_t *>(buf.constData()), len - 1);
}

QPointF QPdfDocumentPrivate::getCharPosition(FPDF_TEXTPAGE textPage, double pageHeight, int charIndex)
{
    double x, y;
    int count = FPDFText_CountChars(textPage);
    bool ok = FPDFText_GetCharOrigin(textPage, qMin(count - 1, charIndex), &x, &y);
    if (!ok)
        return QPointF();
    return QPointF(x, pageHeight - y);
}

QRectF QPdfDocumentPrivate::getCharBox(FPDF_TEXTPAGE textPage, double pageHeight, int charIndex)
{
    double l, t, r, b;
    bool ok = FPDFText_GetCharBox(textPage, charIndex, &l, &r, &b, &t);
    if (!ok)
        return QRectF();
    return QRectF(l, pageHeight - t, r - l, t - b);
}

QPdfDocumentPrivate::TextPosition QPdfDocumentPrivate::hitTest(int page, QPointF position)
{
    const QPdfMutexLocker lock;

    TextPosition result;
    FPDF_PAGE pdfPage = FPDF_LoadPage(doc, page);
    double pageHeight = FPDF_GetPageHeight(pdfPage);
    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(pdfPage);
    int hitIndex = FPDFText_GetCharIndexAtPos(textPage, position.x(), pageHeight - position.y(),
                                              CharacterHitTolerance, CharacterHitTolerance);
    if (hitIndex >= 0) {
        QPointF charPos = getCharPosition(textPage, pageHeight, hitIndex);
        if (!charPos.isNull()) {
            QRectF charBox = getCharBox(textPage, pageHeight, hitIndex);
            // If the given position is past the end of the line, i.e. if the right edge of the found character's
            // bounding box is closer to it than the left edge is, we say that we "hit" the next character index after
            if (qAbs(charBox.right() - position.x()) < qAbs(charPos.x() - position.x())) {
                charPos.setX(charBox.right());
                ++hitIndex;
            }
            qCDebug(qLcDoc) << "on page" << page << "@" << position << "got char position" << charPos << "index" << hitIndex;
            result =  { charPos, charBox.height(), hitIndex };
        }
    }

    FPDFText_ClosePage(textPage);
    FPDF_ClosePage(pdfPage);

    return result;
}

/*!
    \class QPdfDocument
    \since 5.10
    \inmodule QtPdf

    \brief The QPdfDocument class loads a PDF document and renders pages from it.
*/

/*!
    Constructs a new document with parent object \a parent.
*/
QPdfDocument::QPdfDocument(QObject *parent)
    : QObject(parent)
    , d(new QPdfDocumentPrivate)
{
    d->q = this;
}

/*!
    Destroys the document.
*/
QPdfDocument::~QPdfDocument()
{
}

/*!
    Loads the document contents from \a fileName.
*/
QPdfDocument::Error QPdfDocument::load(const QString &fileName)
{
    qCDebug(qLcDoc) << "loading" << fileName;

    close();

    d->setStatus(QPdfDocument::Status::Loading);

    std::unique_ptr<QFile> f(new QFile(fileName));
    if (!f->open(QIODevice::ReadOnly)) {
        d->lastError = Error::FileNotFound;
        d->setStatus(QPdfDocument::Status::Error);
    } else {
        d->load(f.release(), /*transfer ownership*/true);
    }
    return d->lastError;
}

/*! \internal
    Returns the filename of the document that has been opened,
    or an empty string if no document is open.
*/
QString QPdfDocument::fileName() const
{
    const QFile *f = qobject_cast<QFile *>(d->device.data());
    if (f)
        return f->fileName();
    return QString();
}

/*!
    \enum QPdfDocument::Status

    This enum describes the current status of the document.

    \value Null The initial status after the document has been created or after it has been closed.
    \value Loading The status after load() has been called and before the document is fully loaded.
    \value Ready The status when the document is fully loaded and its data can be accessed.
    \value Unloading The status after close() has been called on an open document.
                     At this point the document is still valid and all its data can be accessed.
    \value Error The status after Loading, if loading has failed.

    \sa QPdfDocument::status()
*/

/*!
    \property QPdfDocument::status

    This property holds the current status of the document.
*/
QPdfDocument::Status QPdfDocument::status() const
{
    return d->status;
}

/*!
    Loads the document contents from \a device.
*/
void QPdfDocument::load(QIODevice *device)
{
    close();

    d->setStatus(QPdfDocument::Status::Loading);

    d->load(device, /*transfer ownership*/false);
}

/*!
    \property QPdfDocument::password

    This property holds the document password.

    If the document is protected by a password, the user must provide it, and
    the application must set this property. Otherwise, it's not needed.
*/
void QPdfDocument::setPassword(const QString &password)
{
    const QByteArray newPassword = password.toUtf8();

    if (d->password == newPassword)
        return;

    d->password = newPassword;
    emit passwordChanged();
}

QString QPdfDocument::password() const
{
    return QString::fromUtf8(d->password);
}

/*!
    \enum QPdfDocument::MetaDataField

    This enum describes the available fields of meta data.

    \value Title The document's title as QString.
    \value Author The name of the person who created the document as QString.
    \value Subject The subject of the document as QString.
    \value Keywords Keywords associated with the document as QString.
    \value Creator If the document was converted to PDF from another format,
                   the name of the conforming product that created the original document
                   from which it was converted as QString.
    \value Producer If the document was converted to PDF from another format,
                    the name of the conforming product that converted it to PDF as QString.
    \value CreationDate The date and time the document was created as QDateTime.
    \value ModificationDate The date and time the document was most recently modified as QDateTime.

    \sa QPdfDocument::metaData()
*/

/*!
    Returns the meta data of the document for the given \a field.
*/
QVariant QPdfDocument::metaData(MetaDataField field) const
{
    if (!d->doc)
        return QString();

    static QMetaEnum fieldsMetaEnum = metaObject()->enumerator(metaObject()->indexOfEnumerator("MetaDataField"));
    QByteArray fieldName;
    switch (field) {
    case MetaDataField::ModificationDate:
        fieldName = "ModDate";
        break;
    default:
        fieldName = QByteArray(fieldsMetaEnum.valueToKey(int(field)));
        break;
    }

    QPdfMutexLocker lock;
    const unsigned long len = FPDF_GetMetaText(d->doc, fieldName.constData(), nullptr, 0);

    QList<ushort> buf(len);
    FPDF_GetMetaText(d->doc, fieldName.constData(), buf.data(), buf.size());
    lock.unlock();

    QString text = QString::fromUtf16(reinterpret_cast<const char16_t *>(buf.data()));

    switch (field) {
    case MetaDataField::Title: // fall through
    case MetaDataField::Subject:
    case MetaDataField::Author:
    case MetaDataField::Keywords:
    case MetaDataField::Producer:
    case MetaDataField::Creator:
        return text;
    case MetaDataField::CreationDate: // fall through
    case MetaDataField::ModificationDate:
        // convert a "D:YYYYMMDDHHmmSSOHH'mm'" into "YYYY-MM-DDTHH:mm:ss+HH:mm"
        if (text.startsWith(QLatin1String("D:")))
            text = text.mid(2);
        text.insert(4, QLatin1Char('-'));
        text.insert(7, QLatin1Char('-'));
        text.insert(10, QLatin1Char('T'));
        text.insert(13, QLatin1Char(':'));
        text.insert(16, QLatin1Char(':'));
        text.replace(QLatin1Char('\''), QLatin1Char(':'));
        if (text.endsWith(QLatin1Char(':')))
            text.chop(1);

        return QDateTime::fromString(text, Qt::ISODate);
    }

    return QVariant();
}

/*!
    \enum QPdfDocument::Error

    This enum describes the error while attempting the last operation on the document.

    \value None No error occurred.
    \value Unknown Unknown type of error.
    \value DataNotYetAvailable The document is still loading, it's too early to attempt the operation.
    \value FileNotFound The file given to load() was not found.
    \value InvalidFileFormat The file given to load() is not a valid PDF file.
    \value IncorrectPassword The password given to setPassword() is not correct for this file.
    \value UnsupportedSecurityScheme QPdfDocument is not able to unlock this kind of PDF file.

    \sa QPdfDocument::error()
*/

/*!
    Returns the type of error if \l status is \c Error, or \c NoError if there
    is no error.
*/
QPdfDocument::Error QPdfDocument::error() const
{
    return d->lastError;
}

/*!
    Closes the document.
*/
void QPdfDocument::close()
{
    if (!d->doc)
        return;

    d->setStatus(Status::Unloading);

    d->clear();

    if (!d->password.isEmpty()) {
        d->password.clear();
        emit passwordChanged();
    }

    d->setStatus(Status::Null);
}

/*!
    \property QPdfDocument::pageCount

    This property holds the number of pages in the loaded document or \c 0 if
    no document is loaded.
*/
int QPdfDocument::pageCount() const
{
    return d->pageCount;
}

/*!
    Returns the size of page \a page in points (1/72 of an inch).
*/
QSizeF QPdfDocument::pagePointSize(int page) const
{
    QSizeF result;
    if (!d->doc || !d->checkPageComplete(page))
        return result;

    const QPdfMutexLocker lock;

    FPDF_GetPageSizeByIndex(d->doc, page, &result.rwidth(), &result.rheight());
    return result;
}

/*!
    \enum QPdfDocument::PageModelRole

    Roles in pageModel().

    \value Label The page number to be used for display purposes (QString).
    \value PointSize The page size in points (1/72 of an inch) (QSizeF).
    \omitvalue NRoles
*/

/*!
    \property QPdfDocument::pageModel

    This property holds an instance of QAbstractListModel to provide
    page-specific metadata, containing one row for each page in the document.

    \sa QPdfDocument::PageModelRole
*/
QAbstractListModel *QPdfDocument::pageModel()
{
    if (!d->pageModel)
        d->pageModel = new QPdfPageModel(this);
    return d->pageModel;
}

/*!
    Returns the \a page number to be used for display purposes.

    For example, a document may have multiple sections with different numbering.
    Perhaps the preface uses roman numerals, the body starts on page 1, and the
    appendix starts at A1. Whenever a PDF viewer shows a page number, to avoid
    confusing the user it should be the same "number" as is printed on the
    corner of the page, rather than the zero-based page index that we use in
    APIs (assuming the document author has made the page labels match the
    printed numbers).

    If the document does not have custom page numbering, this function returns
    \c {page + 1}.

    \sa pageIndexForLabel()
*/
QString QPdfDocument::pageLabel(int page)
{
    const unsigned long len = FPDF_GetPageLabel(d->doc, page, nullptr, 0);
    if (len == 0)
        return QString::number(page + 1);
    QList<char16_t> buf(len);
    QPdfMutexLocker lock;
    FPDF_GetPageLabel(d->doc, page, buf.data(), len);
    lock.unlock();
    return QString::fromUtf16(buf.constData());
}

/*!
    Returns the index of the page that has the \a label, or \c -1 if not found.

    \sa pageLabel()
    \since 6.6
*/
int QPdfDocument::pageIndexForLabel(const QString &label)
{
    for (int i = 0; i < d->pageCount; ++i) {
        if (pageLabel(i) == label)
            return i;
    }
    return -1;
}

/*!
    Renders the \a page into a QImage of size \a imageSize according to the
    provided \a renderOptions.

    Returns the rendered page or an empty image in case of an error.

    Note: If the \a imageSize does not match the aspect ratio of the page in the
    PDF document, the page is rendered scaled, so that it covers the
    complete \a imageSize.
*/
QImage QPdfDocument::render(int page, QSize imageSize, QPdfDocumentRenderOptions renderOptions)
{
    if (!d->doc || !d->checkPageComplete(page))
        return QImage();

    const QPdfMutexLocker lock;

    QElapsedTimer timer;
    if (Q_UNLIKELY(qLcDoc().isDebugEnabled()))
        timer.start();
    FPDF_PAGE pdfPage = FPDF_LoadPage(d->doc, page);
    if (!pdfPage)
        return QImage();

    QImage result(imageSize, QImage::Format_ARGB32);
    result.fill(Qt::transparent);
    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(result.width(), result.height(), FPDFBitmap_BGRA, result.bits(), result.bytesPerLine());

    const QPdfDocumentRenderOptions::RenderFlags renderFlags = renderOptions.renderFlags();
    int flags = 0;
    if (renderFlags & QPdfDocumentRenderOptions::RenderFlag::Annotations)
        flags |= FPDF_ANNOT;
    if (renderFlags & QPdfDocumentRenderOptions::RenderFlag::OptimizedForLcd)
        flags |= FPDF_LCD_TEXT;
    if (renderFlags & QPdfDocumentRenderOptions::RenderFlag::Grayscale)
        flags |= FPDF_GRAYSCALE;
    if (renderFlags & QPdfDocumentRenderOptions::RenderFlag::ForceHalftone)
        flags |= FPDF_RENDER_FORCEHALFTONE;
    if (renderFlags & QPdfDocumentRenderOptions::RenderFlag::TextAliased)
        flags |= FPDF_RENDER_NO_SMOOTHTEXT;
    if (renderFlags & QPdfDocumentRenderOptions::RenderFlag::ImageAliased)
        flags |= FPDF_RENDER_NO_SMOOTHIMAGE;
    if (renderFlags & QPdfDocumentRenderOptions::RenderFlag::PathAliased)
        flags |= FPDF_RENDER_NO_SMOOTHPATH;

    if (renderOptions.scaledClipRect().isValid()) {
        const QRect &clipRect = renderOptions.scaledClipRect();

        // TODO take rotation into account, like cpdf_page.cpp lines 145-178
        float x0 = clipRect.left();
        float y0 = clipRect.top();
        float x1 = clipRect.left();
        float y1 = clipRect.bottom();
        float x2 = clipRect.right();
        float y2 = clipRect.top();
        QSizeF origSize = pagePointSize(page);
        QVector2D pageScale(1, 1);
        if (!renderOptions.scaledSize().isNull()) {
            pageScale = QVector2D(renderOptions.scaledSize().width() / float(origSize.width()),
                                  renderOptions.scaledSize().height() / float(origSize.height()));
        }
        FS_MATRIX matrix {(x2 - x0) / result.width() * pageScale.x(),
                          (y2 - y0) / result.width() * pageScale.x(),
                          (x1 - x0) / result.height() * pageScale.y(),
                          (y1 - y0) / result.height() * pageScale.y(), -x0, -y0};

        FS_RECTF clipRectF { 0, 0, float(imageSize.width()), float(imageSize.height()) };

        FPDF_RenderPageBitmapWithMatrix(bitmap, pdfPage, &matrix, &clipRectF, flags);
        qCDebug(qLcDoc) << "matrix" << matrix.a << matrix.b << matrix.c << matrix.d << matrix.e << matrix.f;
        qCDebug(qLcDoc) << "page" << page << "region" << renderOptions.scaledClipRect()
                        << "size" << imageSize << "took" << timer.elapsed() << "ms";
    } else {
        const auto rotation = QPdfDocumentPrivate::toFPDFRotation(renderOptions.rotation());
        FPDF_RenderPageBitmap(bitmap, pdfPage, 0, 0, result.width(), result.height(), rotation, flags);
        qCDebug(qLcDoc) << "page" << page << "size" << imageSize << "took" << timer.elapsed() << "ms";
    }

    FPDFBitmap_Destroy(bitmap);

    FPDF_ClosePage(pdfPage);
    return result;
}

/*!
    Returns information about the text on the given \a page that can be found
    between the given \a start and \a end points, if any.
*/
QPdfSelection QPdfDocument::getSelection(int page, QPointF start, QPointF end)
{
    const QPdfMutexLocker lock;
    FPDF_PAGE pdfPage = FPDF_LoadPage(d->doc, page);
    double pageHeight = FPDF_GetPageHeight(pdfPage);
    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(pdfPage);
    int startIndex = FPDFText_GetCharIndexAtPos(textPage, start.x(), pageHeight - start.y(),
                                                CharacterHitTolerance, CharacterHitTolerance);
    int endIndex = FPDFText_GetCharIndexAtPos(textPage, end.x(), pageHeight - end.y(),
                                              CharacterHitTolerance, CharacterHitTolerance);

    QPdfSelection result;

    if (startIndex >= 0 && endIndex != startIndex) {
        if (startIndex > endIndex)
            qSwap(startIndex, endIndex);

        // If the given end position is past the end of the line, i.e. if the right edge of the last character's
        // bounding box is closer to it than the left edge is, then extend the char range by one
        QRectF endCharBox = d->getCharBox(textPage, pageHeight, endIndex);
        if (qAbs(endCharBox.right() - end.x()) < qAbs(endCharBox.x() - end.x()))
            ++endIndex;

        int count = endIndex - startIndex;
        QString text = d->getText(textPage, startIndex, count);
        QList<QPolygonF> bounds;
        QRectF hull;
        int rectCount = FPDFText_CountRects(textPage, startIndex, endIndex - startIndex);
        for (int i = 0; i < rectCount; ++i) {
            double l, r, b, t;
            FPDFText_GetRect(textPage, i, &l, &t, &r, &b);
            QRectF rect(l, pageHeight - t, r - l, t - b);
            if (hull.isNull())
                hull = rect;
            else
                hull = hull.united(rect);
            bounds << QPolygonF(rect);
        }
        qCDebug(qLcDoc) << page << start << "->" << end << "found" << startIndex << "->" << endIndex << text;
        result = QPdfSelection(text, bounds, hull, startIndex, endIndex);
    } else {
        qCDebug(qLcDoc) << page << start << "->" << end << "nothing found";
    }

    FPDFText_ClosePage(textPage);
    FPDF_ClosePage(pdfPage);

    return result;
}

/*!
    Returns information about the text on the given \a page that can be found
    beginning at the given \a startIndex with at most \a maxLength characters.
*/
QPdfSelection QPdfDocument::getSelectionAtIndex(int page, int startIndex, int maxLength)
{

    if (page < 0 || startIndex < 0 || maxLength < 0)
        return {};
    const QPdfMutexLocker lock;
    FPDF_PAGE pdfPage = FPDF_LoadPage(d->doc, page);
    double pageHeight = FPDF_GetPageHeight(pdfPage);
    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(pdfPage);
    int pageCount = FPDFText_CountChars(textPage);
    if (startIndex >= pageCount)
        return QPdfSelection();
    QList<QPolygonF> bounds;
    QRectF hull;
    int rectCount = 0;
    QString text;
    if (maxLength > 0) {
        text = d->getText(textPage, startIndex, maxLength);
        rectCount = FPDFText_CountRects(textPage, startIndex, text.size());
        for (int i = 0; i < rectCount; ++i) {
            double l, r, b, t;
            FPDFText_GetRect(textPage, i, &l, &t, &r, &b);
            QRectF rect(l, pageHeight - t, r - l, t - b);
            if (hull.isNull())
                hull = rect;
            else
                hull = hull.united(rect);
            bounds << QPolygonF(rect);
        }
    }
    if (bounds.isEmpty())
        hull = QRectF(d->getCharPosition(textPage, pageHeight, startIndex), QSizeF());
    qCDebug(qLcDoc) << "on page" << page << "at index" << startIndex << "maxLength" << maxLength
                    << "got" << text.size() << "chars," << rectCount << "rects within" << hull;

    FPDFText_ClosePage(textPage);
    FPDF_ClosePage(pdfPage);

    return QPdfSelection(text, bounds, hull, startIndex, startIndex + text.size());
}

/*!
    Returns all the text and its bounds on the given \a page.
*/
QPdfSelection QPdfDocument::getAllText(int page)
{
    const QPdfMutexLocker lock;
    FPDF_PAGE pdfPage = FPDF_LoadPage(d->doc, page);
    double pageHeight = FPDF_GetPageHeight(pdfPage);
    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(pdfPage);
    int count = FPDFText_CountChars(textPage);
    if (count < 1)
        return QPdfSelection();
    QString text = d->getText(textPage, 0, count);
    QList<QPolygonF> bounds;
    QRectF hull;
    int rectCount = FPDFText_CountRects(textPage, 0, count);
    for (int i = 0; i < rectCount; ++i) {
        double l, r, b, t;
        FPDFText_GetRect(textPage, i, &l, &t, &r, &b);
        QRectF rect(l, pageHeight - t, r - l, t - b);
        if (hull.isNull())
            hull = rect;
        else
            hull = hull.united(rect);
        bounds << QPolygonF(rect);
    }
    qCDebug(qLcDoc) << "on page" << page << "got" << count << "chars," << rectCount << "rects within" << hull;

    FPDFText_ClosePage(textPage);
    FPDF_ClosePage(pdfPage);

    return QPdfSelection(text, bounds, hull, 0, count);
}

QT_END_NAMESPACE

#include "qpdfdocument.moc"
#include "moc_qpdfdocument.cpp"
