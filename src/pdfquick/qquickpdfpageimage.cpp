/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickpdfpageimage_p.h"
#include "qquickpdfdocument_p.h"
#include <private/qpdffile_p.h>
#include <QtPdf/QPdfPageNavigation>
#include <QtQuick/private/qquickimage_p_p.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcImg, "qt.pdf.image")

/*!
    \qmltype PdfPageImage
    \instantiates QQuickPdfPageImage
    \inqmlmodule QtPdf
    \ingroup pdf
    \inherits Item
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

QQuickPdfPageImage::~QQuickPdfPageImage()
{
    Q_D(QQuickPdfPageImage);
    // cancel any async rendering job that is running on my behalf
    d->pix.clear();
}

void QQuickPdfPageImage::setDocument(QQuickPdfDocument *document)
{
    Q_D(QQuickPdfPageImage);
    if (d->doc == document)
        return;

    if (d->doc)
        disconnect(d->doc, &QQuickPdfDocument::statusChanged, this, &QQuickPdfPageImage::documentStatusChanged);
    d->doc = document;
    if (document) {
        connect(document, &QQuickPdfDocument::statusChanged, this, &QQuickPdfPageImage::documentStatusChanged);
        if (document->status() == QPdfDocument::Status::Ready)
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
    qCDebug(qLcImg) << "document status" << d->doc->status();
    if (d->doc->status() == QPdfDocument::Status::Ready)
        setSource(d->doc->resolvedSource()); // calls load()
}

QT_END_NAMESPACE
