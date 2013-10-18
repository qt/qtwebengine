/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "backing_store_qt.h"

#include "content/public/browser/render_process_host.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/rect_conversions.h"
#include "ui/gfx/vector2d_conversions.h"
#include "skia/ext/platform_canvas.h"

#include <QPainter>
#include <QScreen>
#include <QSizeF>
#include <QWindow>

BackingStoreQt::BackingStoreQt(content::RenderWidgetHost *host, const gfx::Size &size, QWindow* parent)
    : content::BackingStore(host, size)
    , m_deviceScaleFactor((parent && parent->screen()) ? parent->screen()->devicePixelRatio() : 1)
    , m_pixelBuffer(size.width() * m_deviceScaleFactor, size.height() * m_deviceScaleFactor)
{
}

BackingStoreQt::~BackingStoreQt()
{
}

void BackingStoreQt::paintToTarget(QPainter* painter, const QRectF& rect)
{
    if (m_pixelBuffer.isNull())
        return;

    qreal x = rect.x() * m_deviceScaleFactor;
    qreal y = rect.y() * m_deviceScaleFactor;
    qreal w = rect.width() * m_deviceScaleFactor;
    qreal h = rect.height() * m_deviceScaleFactor;

    QRectF source(x, y, w, h);
    painter->drawPixmap(rect, m_pixelBuffer, source);
}

void BackingStoreQt::PaintToBackingStore(content::RenderProcessHost *process,
                                 TransportDIB::Id bitmap,
                                 const gfx::Rect &bitmap_rect,
                                 const std::vector<gfx::Rect> &copy_rects,
                                 float scale_factor,
                                 const base::Closure &completion_callback,
                                 bool *scheduled_completion_callback)
{
    if (bitmap_rect.IsEmpty())
        return;

    *scheduled_completion_callback = false;
    TransportDIB* dib = process->GetTransportDIB(bitmap);
    if (!dib)
      return;

    gfx::Rect pixel_bitmap_rect = gfx::ToEnclosingRect(gfx::ScaleRect(bitmap_rect, scale_factor));

    uint8_t* bitmapData = static_cast<uint8_t*>(dib->memory());
    const QImage img(bitmapData, pixel_bitmap_rect.width(), pixel_bitmap_rect.height(), QImage::Format_ARGB32);

    QPainter painter(&m_pixelBuffer);
    for (size_t i = 0; i < copy_rects.size(); ++i) {
        gfx::Rect copy_rect = gfx::ToEnclosingRect(gfx::ScaleRect(copy_rects[i], scale_factor));

        QRect source = QRect( copy_rect.x() - pixel_bitmap_rect.x()
                            , copy_rect.y() - pixel_bitmap_rect.y()
                            , copy_rect.width()
                            , copy_rect.height());

        gfx::Rect copy_rect_dst = gfx::ToEnclosingRect(gfx::ScaleRect(copy_rects[i], m_deviceScaleFactor));

        QRect destination = QRect( copy_rect_dst.x()
                                 , copy_rect_dst.y()
                                 , copy_rect_dst.width()
                                 , copy_rect_dst.height());

        painter.drawImage(destination, img, source);
    }
}

void BackingStoreQt::ScrollBackingStore(const gfx::Vector2d &delta, const gfx::Rect &clip_rect, const gfx::Size &view_size)
{
    DCHECK(delta.x() == 0 || delta.y() == 0);

    gfx::Rect pixel_rect = gfx::ToEnclosingRect(gfx::ScaleRect(clip_rect, m_deviceScaleFactor));
    gfx::Vector2d pixel_delta = gfx::ToFlooredVector2d(gfx::ScaleVector2d(delta, m_deviceScaleFactor));

    m_pixelBuffer.scroll(pixel_delta.x()
                         , pixel_delta.y()
                         , pixel_rect.x()
                         , pixel_rect.y()
                         , pixel_rect.width()
                         , pixel_rect.height());

}

bool BackingStoreQt::CopyFromBackingStore(const gfx::Rect &rect, skia::PlatformBitmap *output)
{
    const int width = std::min(m_pixelBuffer.width(), rect.width());
    const int height = std::min(m_pixelBuffer.height(), rect.height());

    if (!output->Allocate(width, height, true))
        return false;

    // This code assumes a visual mode where a pixel is
    // represented using a 32-bit unsigned int, with a byte per component.
    const SkBitmap& bitmap = output->GetBitmap();
    if (bitmap.rowBytes() != 4)
        return false;

    SkAutoLockPixels alp(bitmap);

    QPixmap cpy = m_pixelBuffer.copy(rect.x(), rect.y(), rect.width(), rect.height());
    QImage img = cpy.toImage();

    // Convert the format and remove transparency.
    if (img.format() != QImage::Format_RGB32)
        img = img.convertToFormat(QImage::Format_RGB32);

    const uint8_t* src = img.bits();
    uint8_t* dst = reinterpret_cast<uint8_t*>(bitmap.getAddr32(0,0));

    int bytesPerLine = img.bytesPerLine();
    int bytesPerPixel = bytesPerLine / img.width();
    int copyLineLength = width * bytesPerPixel;
    int lineOffset = rect.y() * img.width();
    int rowOffset = rect.x() * bytesPerPixel;

    const uint8_t* copyLineBegin = src + rowOffset + lineOffset;

    for (int lineNumber = 0; lineNumber < height; ++lineNumber) {
        memcpy(dst, copyLineBegin, copyLineLength);
        dst += copyLineLength;
        copyLineBegin += img.width();
    }

    return true;
}

