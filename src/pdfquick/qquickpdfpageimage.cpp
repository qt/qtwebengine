// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickpdfpageimage_p.h"
#include "qquickpdfdocument_p.h"
#include <private/qpdffile_p.h>
#include <QtQuick/private/qquickimage_p_p.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcImg, "qt.pdf.image")

/*!
    \qmltype PdfPageImage
//!    \instantiates QQuickPdfPageImage
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \inherits Image
    \brief Displays one page from a PDF document.
    \since 6.4

    The PdfPageImage type is an Image specialized to render a page from a PDF document.
*/

class QQuickPdfPageImagePrivate: public QQuickImagePrivate
{
public:
    QQuickPdfPageImagePrivate() : QQuickImagePrivate() {}

    QQuickPdfDocument *doc = nullptr;
};

QQuickPdfPageImage::QQuickPdfPageImage(QQuickItem *parent)
    : QQuickImage(*(new QQuickPdfPageImagePrivate), parent)
{
}

/*!
    \internal
*/
QQuickPdfPageImage::~QQuickPdfPageImage()
{
    Q_D(QQuickPdfPageImage);
    // cancel any async rendering job that is running on my behalf
    d->pix.clear();
}

/*!
    \qmlproperty PdfDocument PdfPageImage::document

    This property holds the PDF document from which to render an image.
*/
void QQuickPdfPageImage::setDocument(QQuickPdfDocument *document)
{
    Q_D(QQuickPdfPageImage);
    if (d->doc == document)
        return;

    if (d->doc)
        disconnect(d->doc->document(), &QPdfDocument::statusChanged, this, &QQuickPdfPageImage::documentStatusChanged);
    d->doc = document;
    if (document) {
        connect(document->document(), &QPdfDocument::statusChanged, this, &QQuickPdfPageImage::documentStatusChanged);
        if (document->document()->status() == QPdfDocument::Status::Ready)
            setSource(document->resolvedSource()); // calls load()
    }
    emit documentChanged();
}

QQuickPdfDocument *QQuickPdfPageImage::document() const
{
    Q_D(const QQuickPdfPageImage);
    return d->doc;
}

void QQuickPdfPageImage::load()
{
    Q_D(QQuickPdfPageImage);
    auto carrierFile = d->doc->carrierFile();
    static int thisRequestProgress = -1;
    static int thisRequestFinished = -1;
    if (thisRequestProgress == -1) {
        thisRequestProgress =
            QQuickImageBase::staticMetaObject.indexOfSlot("requestProgress(qint64,qint64)");
        thisRequestFinished =
            QQuickImageBase::staticMetaObject.indexOfSlot("requestFinished()");
    }

    d->pix.loadImageFromDevice(qmlEngine(this), carrierFile, d->url,
                               d->sourceClipRect.toRect(), d->sourcesize * d->devicePixelRatio,
                               QQuickImageProviderOptions(), d->currentFrame, d->frameCount);

    qCDebug(qLcImg) << "loading page" << d->currentFrame << "of" << d->frameCount
                    << "from" << carrierFile->fileName() << "status" << d->pix.status();

    switch (d->pix.status()) {
    case QQuickPixmap::Ready:
        pixmapChange();
        break;
    case QQuickPixmap::Loading:
        d->pix.connectFinished(this, thisRequestFinished);
        d->pix.connectDownloadProgress(this, thisRequestProgress);
        if (d->progress != 0.0) {
            d->progress = 0.0;
            emit progressChanged(d->progress);
        }
        if (d->status != Loading) {
            d->status = Loading;
            emit statusChanged(d->status);
        }
        break;
    default:
        qCDebug(qLcImg) << "unexpected status" << d->pix.status();
        break;
    }
}

void QQuickPdfPageImage::documentStatusChanged()
{
    Q_D(QQuickPdfPageImage);
    const auto status = d->doc->document()->status();
    qCDebug(qLcImg) << "document status" << status;
    if (status == QPdfDocument::Status::Ready)
        setSource(d->doc->resolvedSource()); // calls load()
}

QT_END_NAMESPACE

#include "moc_qquickpdfpageimage_p.cpp"
