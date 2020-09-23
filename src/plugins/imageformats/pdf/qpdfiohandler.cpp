/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qpdfiohandler_p.h"
#include <QLoggingCategory>
#include <QPainter>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcPdf, "qt.imageformat.pdf")

QPdfIOHandler::QPdfIOHandler()
{
}

bool QPdfIOHandler::canRead() const
{
    if (!device())
        return false;
    if (m_loaded)
        return true;
    if (QPdfIOHandler::canRead(device())) {
        setFormat("pdf");
        return true;
    }
    return false;
}

bool QPdfIOHandler::canRead(QIODevice *device)
{
    char buf[6];
    device->peek(buf, 6);
    return (!qstrncmp(buf, "%PDF-", 5) || Q_UNLIKELY(!qstrncmp(buf, "\012%PDF-", 6)));
}

int QPdfIOHandler::currentImageNumber() const
{
    return m_page;
}

QRect QPdfIOHandler::currentImageRect() const
{
    return QRect(QPoint(0, 0), m_doc.pageSize(m_page).toSize());
}

int QPdfIOHandler::imageCount() const
{
    int ret = 0;
    if (const_cast<QPdfIOHandler *>(this)->load(device()))
        ret = m_doc.pageCount();
    qCDebug(qLcPdf) << "imageCount" << ret;
    return ret;
}

bool QPdfIOHandler::read(QImage *image)
{
    if (load(device())) {
        if (m_page >= m_doc.pageCount())
            return false;
        if (m_page < 0)
            m_page = 0;
        const bool xform = (m_clipRect.isValid() || m_scaledSize.isValid() || m_scaledClipRect.isValid());
        QSize pageSize = m_doc.pageSize(m_page).toSize();
        QSize finalSize = pageSize;
        QRectF bounds;
        if (xform && !finalSize.isEmpty()) {
            bounds = QRectF(QPointF(0,0), QSizeF(finalSize));
            QPoint tr1, tr2;
            QSizeF sc(1, 1);
            if (m_clipRect.isValid()) {
                tr1 = -m_clipRect.topLeft();
                finalSize = m_clipRect.size();
            }
            if (m_scaledSize.isValid()) {
                sc = QSizeF(qreal(m_scaledSize.width()) / finalSize.width(),
                            qreal(m_scaledSize.height()) / finalSize.height());
                finalSize = m_scaledSize;
                pageSize = m_scaledSize;
            }
            if (m_scaledClipRect.isValid()) {
                tr2 = -m_scaledClipRect.topLeft();
                finalSize = m_scaledClipRect.size();
            }
            QTransform t;
            t.translate(tr2.x(), tr2.y());
            t.scale(sc.width(), sc.height());
            t.translate(tr1.x(), tr1.y());
            bounds = t.mapRect(bounds);
        }
        qCDebug(qLcPdf) << Q_FUNC_INFO << m_page << finalSize;
        if (image->size() != finalSize || !image->reinterpretAsFormat(QImage::Format_ARGB32_Premultiplied)) {
            *image = QImage(finalSize, QImage::Format_ARGB32_Premultiplied);
            if (!finalSize.isEmpty() && image->isNull()) {
                // avoid QTBUG-68229
                qWarning("QPdfIOHandler: QImage allocation failed (size %i x %i)", finalSize.width(), finalSize.height());
                return false;
            }
        }
        if (!finalSize.isEmpty()) {
            QPdfDocumentRenderOptions options;
            if (m_scaledClipRect.isValid())
                options.setScaledClipRect(m_scaledClipRect);
            options.setScaledSize(pageSize);
            image->fill(m_backColor.rgba());
            QPainter p(image);
            QImage pageImage = m_doc.render(m_page, finalSize, options);
            p.drawImage(0, 0, pageImage);
            p.end();
        }
        return true;
    }

    return false;
}

QVariant QPdfIOHandler::option(ImageOption option) const
{
    switch (option) {
    case ImageFormat:
        return QImage::Format_ARGB32_Premultiplied;
    case Size:
        const_cast<QPdfIOHandler *>(this)->load(device());
        return m_doc.pageSize(qMax(0, m_page));
    case ClipRect:
        return m_clipRect;
    case ScaledSize:
        return m_scaledSize;
    case ScaledClipRect:
        return m_scaledClipRect;
    case BackgroundColor:
        return m_backColor;
    case Name:
        return m_doc.metaData(QPdfDocument::Title);
    default:
        break;
    }
    return QVariant();
}

void QPdfIOHandler::setOption(ImageOption option, const QVariant & value)
{
    switch (option) {
    case ClipRect:
        m_clipRect = value.toRect();
        break;
    case ScaledSize:
        m_scaledSize = value.toSize();
        break;
    case ScaledClipRect:
        m_scaledClipRect = value.toRect();
        break;
    case BackgroundColor:
        m_backColor = value.value<QColor>();
        break;
    default:
        break;
    }
}

bool QPdfIOHandler::supportsOption(ImageOption option) const
{
    switch (option)
    {
    case ImageFormat:
    case Size:
    case ClipRect:
    case ScaledSize:
    case ScaledClipRect:
    case BackgroundColor:
    case Name:
        return true;
    default:
        break;
    }
    return false;
}

bool QPdfIOHandler::jumpToImage(int frame)
{
    qCDebug(qLcPdf) << Q_FUNC_INFO << frame;
    if (frame < 0 || frame >= imageCount())
        return false;
    m_page = frame;
    return true;
}

bool QPdfIOHandler::jumpToNextImage()
{
    return jumpToImage(m_page + 1);
}

bool QPdfIOHandler::load(QIODevice *device)
{
    if (m_loaded)
        return true;
    if (format().isEmpty())
        if (!canRead())
            return false;

    m_doc.load(device);
    m_loaded = (m_doc.error() == QPdfDocument::DocumentError::NoError);

    return m_loaded;
}

QT_END_NAMESPACE
